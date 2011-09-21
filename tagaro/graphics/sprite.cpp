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

KGame::Sprite::Sprite()
	: d(new Private)
{
}

KGame::Sprite::Private::Private()
	: m_source(0)
{
}

KGame::Sprite::~Sprite()
{
	//disconnect clients (take a copy of d->m_clients because this list is modified by KGame::SpriteClient::setSprite)
	const QList<KGame::SpriteClient*> clients = d->m_clients;
	const int clientCount = clients.count();
	for (int i = 0; i < clientCount; ++i)
	{
		clients[i]->setSprite(0);
	}
	//delete own stuff
	qDeleteAll(d->m_fetchers);
	delete d;
}

void KGame::Sprite::Private::addClient(KGame::SpriteClient* client)
{
	m_clients << client;
}

void KGame::Sprite::Private::removeClient(KGame::SpriteClient* client)
{
	m_clients.removeAll(client);
}

void KGame::Sprite::Private::setSource(const KGame::GraphicsSource* source, const QString& element)
{
	if (m_source != source || m_element != element)
	{
		m_source = source;
		m_element = element;
		foreach (KGame::SpriteFetcher* fetcher, m_fetchers)
		{
			fetcher->updateAllClients();
		}
	}
}

QRectF KGame::Sprite::bounds(int frame) const
{
	if (!d->m_source)
	{
		return QRectF();
	}
	const QString frameElement = d->m_source->frameElementKey(d->m_element, frame);
	return d->m_source->elementBounds(frameElement);
}

bool KGame::Sprite::exists() const
{
	return frameCount() >= 0;
}

int KGame::Sprite::frameCount() const
{
	return d->m_source ? d->m_source->frameCount(d->m_element) : -1;
}

QString KGame::Sprite::key() const
{
	return d->m_element;
}

KGame::SpriteFetcher* KGame::Sprite::Private::fetcher(const QSize& size, const QString& processingInstruction)
{
	if (size.isEmpty())
	{
		return 0;
	}
	KGame::SpriteFetcher*& fetcher = m_fetchers[qMakePair(size, processingInstruction)];
	if (!fetcher)
	{
		fetcher = new KGame::SpriteFetcher(size, processingInstruction, this);
	}
	return fetcher;
}

QPixmap KGame::Sprite::pixmap(const QSize& size, int frame, const QString& processingInstruction) const
{
	if (!d->m_source || size.isEmpty())
	{
		return QPixmap();
	}
	return d->fetcher(size, processingInstruction)->cachePixmap(frame, QImage());
}

//BEGIN asynchronous pixmap serving

void KGame::SpriteFetcher::addClient(KGame::SpriteClient* client)
{
	m_clients << client;
	updateClient(client);
}

void KGame::SpriteFetcher::removeClient(KGame::SpriteClient* client)
{
	m_clients.removeAll(client);
}

void KGame::SpriteFetcher::updateClient(KGame::SpriteClient* client)
{
	const int frame = client->frame();
	//check if request can be served immediately
	QHash<int, QPixmap>::const_iterator it = m_pixmapCache.find(frame);
	if (it != m_pixmapCache.constEnd())
	{
		client->d->receivePixmap(it.value());
		return;
	}
	//no source available?
	if (!d->m_source)
	{
		client->d->receivePixmap(QPixmap());
		return;
	}
	//check if request can be served without much hassle
	const QString frameElement = d->m_source->frameElementKey(d->m_element, frame);
	const QImage image = d->m_source->elementImage(frameElement, m_size, m_processingInstruction, true);
	if (!image.isNull())
	{
		//This also sends the pixmap to the client in question.
		cachePixmap(frame, image);
	}
	else
	{
		//create rendering request
		startJob(frame);
	}
}

void KGame::SpriteFetcher::updateAllClients()
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

namespace KGame {
	class SpriteFetcherWorker : public QRunnable
	{
		private:
			const KGame::GraphicsSource* m_source;
			QString m_element;
			int m_frame;
			QSize m_size;
			QString m_processingInstruction;
			KGame::SpriteFetcher* m_receiver;
		public:
			SpriteFetcherWorker(const KGame::GraphicsSource* source, const QString& element, int frame, const QSize& size, const QString& processingInstruction, KGame::SpriteFetcher* receiver)
				: m_source(source)
				  //DO NOT do the following in the worker thread. frameElementKey() is not guaranteed to be thread-safe!
				, m_element(m_source->frameElementKey(element, frame))
				, m_frame(frame)
				, m_size(size)
				, m_processingInstruction(processingInstruction)
				, m_receiver(receiver)
			{
			}
			virtual void run()
			{
				QImage result;
				if (m_source)
				{
					result = m_source->elementImage(m_element, m_size, m_processingInstruction, false);
				}
				else
				{
					result = QImage(m_size, QImage::Format_ARGB32_Premultiplied);
					result.fill(QColor(Qt::transparent).rgba());
				}
				QMetaObject::invokeMethod(m_receiver, "cachePixmap",
					Q_ARG(int, m_frame), Q_ARG(QImage, result)
				);
			}
	};
}

void KGame::SpriteFetcher::startJob(int frame)
{
	if (KGame::Settings::useRenderingThreads())
	{
		QThreadPool::globalInstance()->start(new KGame::SpriteFetcherWorker(d->m_source, d->m_element, frame, m_size, m_processingInstruction, this));
	}
	else
	{
		const QString element = d->m_source->frameElementKey(d->m_element, frame);
		const QImage result = d->m_source->elementImage(element, m_size, m_processingInstruction, false);
		cachePixmap(frame, result);
	}
}

QPixmap KGame::SpriteFetcher::cachePixmap(int frame, const QImage& image)
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
			useImage = d->m_source->elementImage(frameElement, m_size, m_processingInstruction, false);
		}
		else
		{
			useImage = QImage(m_size, QImage::Format_ARGB32_Premultiplied);
			useImage.fill(QColor(Qt::transparent).rgba());
		}
	}
	const QPixmap result = QPixmap::fromImage(useImage);
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
