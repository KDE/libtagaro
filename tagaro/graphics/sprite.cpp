/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   version 2 as published by the Free Software Foundation                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "sprite.h"
#include "sprite_p.h"
#include "graphicssource.h"
#include "settings.h"

#include <QtCore/QRunnable>
#include <QtCore/QSet>
#include <QtCore/QThreadPool>

Tagaro::Sprite::Sprite()
	: d(new Private)
{
}

Tagaro::Sprite::Private::Private()
	: m_source(0)
{
}

Tagaro::Sprite::~Sprite()
{
	//disconnect clients (take a copy of d->m_clients because this list is modified by Tagaro::SpriteClient::setSprite)
	const QList<Tagaro::SpriteClient*> clients = d->m_clients;
	const int clientCount = clients.count();
	for (int i = 0; i < clientCount; ++i)
	{
		clients[i]->setSprite(0);
	}
	//delete own stuff
	qDeleteAll(d->m_fetchers);
	delete d;
}

void Tagaro::Sprite::Private::addClient(Tagaro::SpriteClient* client)
{
	m_clients << client;
}

void Tagaro::Sprite::Private::removeClient(Tagaro::SpriteClient* client)
{
	m_clients.removeAll(client);
}

void Tagaro::Sprite::Private::setSource(const Tagaro::GraphicsSource* source, const QString& element)
{
	if (m_source != source || m_element != element)
	{
		m_source = source;
		m_element = element;
		foreach (Tagaro::SpriteFetcher* fetcher, m_fetchers)
		{
			fetcher->updateAllClients();
		}
	}
}

QRectF Tagaro::Sprite::bounds(int frame) const
{
	if (!d->m_source)
	{
		return QRectF();
	}
	const QString frameElement = d->m_source->frameElementKey(d->m_element, frame);
	return d->m_source->elementBounds(frameElement);
}

bool Tagaro::Sprite::exists() const
{
	return frameCount() >= 0;
}

int Tagaro::Sprite::frameCount() const
{
	return d->m_source ? d->m_source->frameCount(d->m_element) : -1;
}

QString Tagaro::Sprite::key() const
{
	return d->m_element;
}

Tagaro::SpriteFetcher* Tagaro::Sprite::Private::fetcher(const QSize& size)
{
	if (size.isEmpty())
	{
		return 0;
	}
	Tagaro::SpriteFetcher*& fetcher = m_fetchers[size];
	if (!fetcher)
	{
		fetcher = new Tagaro::SpriteFetcher(size, this);
	}
	return fetcher;
}

QPixmap Tagaro::Sprite::pixmap(const QSize& size, int frame) const
{
	if (!d->m_source || size.isEmpty())
	{
		return QPixmap();
	}
	return d->fetcher(size)->cachePixmap(frame, QImage());
}

//BEGIN asynchronous pixmap serving

void Tagaro::SpriteFetcher::addClient(Tagaro::SpriteClient* client)
{
	m_clients << client;
	updateClient(client);
}

void Tagaro::SpriteFetcher::removeClient(Tagaro::SpriteClient* client)
{
	m_clients.removeAll(client);
}

void Tagaro::SpriteFetcher::updateClient(Tagaro::SpriteClient* client)
{
	const int frame = client->frame();
	//check if request can be served immediately
	QHash<int, QPixmap>::const_iterator it = m_pixmapCache.find(frame);
	if (it != m_pixmapCache.constEnd())
	{
		client->d->receivePixmap(it.value());
		return;
	}
	//check if request can be served without much hassle
	if (d->m_source)
	{
		const QString frameElement = d->m_source->frameElementKey(d->m_element, frame);
		const QImage image = d->m_source->elementImage(frameElement, m_size, true);
		if (!image.isNull())
		{
			//This also sends the pixmap to the client in question.
			cachePixmap(frame, image);
		}
	}
	//create rendering request
	startJob(frame);
}

void Tagaro::SpriteFetcher::updateAllClients()
{
	m_pixmapCache.clear();
	//determine which frames are used
	QSet<int> frames;
	const int clientCount = m_clients.count();
	for (int i = 0; i < clientCount; ++i)
	{
		frames << m_clients[i]->frame();
	}
	//create rendering requests for all frames in the set
	QSet<int>::const_iterator it1 = frames.constBegin(), it2 = frames.constEnd();
	for (; it1 != it2; ++it1)
	{
		startJob(*it1);
	}
}

namespace Tagaro {
	class SpriteFetcherWorker : public QRunnable
	{
		private:
			const Tagaro::GraphicsSource* m_source;
			QString m_element;
			int m_frame;
			QSize m_size;
			Tagaro::SpriteFetcher* m_receiver;
		public:
			SpriteFetcherWorker(const Tagaro::GraphicsSource* source, const QString& element, int frame, const QSize& size, Tagaro::SpriteFetcher* receiver)
				: m_source(source)
				  //DO NOT do the following in the worker thread. frameElementKey() is not guaranteed to be thread-safe!
				, m_element(m_source->frameElementKey(element, frame))
				, m_frame(frame)
				, m_size(size)
				, m_receiver(receiver)
			{
			}
			virtual void run()
			{
				const QImage result = m_source->elementImage(m_element, m_size, false);
				QMetaObject::invokeMethod(m_receiver, "cachePixmap",
					Q_ARG(int, m_frame), Q_ARG(QImage, result)
				);
			}
	};
}

void Tagaro::SpriteFetcher::startJob(int frame)
{
	if (Tagaro::Settings::useRenderingThreads())
	{
		QThreadPool::globalInstance()->start(new Tagaro::SpriteFetcherWorker(d->m_source, d->m_element, frame, m_size, this));
	}
	else
	{
		const QString element = d->m_source->frameElementKey(d->m_element, frame);
		const QImage result = d->m_source->elementImage(element, m_size, false);
		cachePixmap(frame, result);
	}
}

QPixmap Tagaro::SpriteFetcher::cachePixmap(int frame, const QImage& image)
{
	//look in cache
	QHash<int, QPixmap>::const_iterator it = m_pixmapCache.find(frame);
	if (it != m_pixmapCache.constEnd())
	{
		return it.value();
	}
	//convert or render image
	QImage useImage = image;
	if (image.isNull())
	{
		if (d->m_source)
		{
			const QString frameElement = d->m_source->frameElementKey(d->m_element, frame);
			useImage = d->m_source->elementImage(frameElement, m_size, false);
		}
		else
		{
			useImage = QImage(m_size, QImage::Format_ARGB32_Premultiplied);
			useImage.fill(QColor(Qt::transparent).rgba());
		}
	}
	const QPixmap result = QPixmap::fromImage(image);
	m_pixmapCache.insert(frame, result);
	//if this frame has been requested by some clients, send it out
	const int clientCount = m_clients.count();
	for (int i = 0; i < clientCount; ++i)
	{
		m_clients[i]->d->receivePixmap(result);
	}
	//done
	return result;
}

//END asynchronous pixmap serving

#include "sprite_p.moc"
