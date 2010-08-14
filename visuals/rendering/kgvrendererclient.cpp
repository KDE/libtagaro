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

#include "kgvrendererclient.h"
#include "kgvrenderer.h"
#include "kgvrenderer_p.h"

#include <QtCore/QTimer>

//WARNING: d->m_renderer == 0 is allowed, and used actively by KgvView.

KgvRendererClientPrivate::KgvRendererClientPrivate(KgvRenderer* renderer, const QString& spriteKey, KgvRendererClient* parent)
	: m_parent(parent)
	, m_renderer(renderer)
	, m_spec(spriteKey, -1, QSize(3, 3))
{
}

KgvRendererClient::KgvRendererClient(KgvRenderer* renderer, const QString& spriteKey)
	: d(new KgvRendererClientPrivate(renderer, spriteKey, this))
{
	if (renderer)
	{
		renderer->d->m_clients.insert(this, QString());
	}
	//The following may not be triggered directly because it may call receivePixmap() which is a pure virtual method at this point.
	QTimer::singleShot(0, d, SLOT(fetchPixmap()));
}

KgvRendererClient::~KgvRendererClient()
{
	if (d->m_renderer)
	{
		d->m_renderer->d->m_clients.remove(this);
	}
	delete d;
}

KgvRenderer* KgvRendererClient::renderer() const
{
	return d->m_renderer;
}

QString KgvRendererClient::spriteKey() const
{
	return d->m_spec.spriteKey;
}

void KgvRendererClient::setSpriteKey(const QString& spriteKey)
{
	if (d->m_spec.spriteKey != spriteKey)
	{
		d->m_spec.spriteKey = spriteKey;
		d->fetchPixmap();
	}
}

int KgvRendererClient::frameCount() const
{
	return d->m_renderer ? d->m_renderer->frameCount(d->m_spec.spriteKey) : -1;
}

int KgvRendererClient::frame() const
{
	return d->m_spec.frame;
}

void KgvRendererClient::setFrame(int frame)
{
	if (d->m_spec.frame != frame)
	{
		//do some normalization ourselves
		const int frameCount = this->frameCount();
		if (frameCount <= 0)
		{
			frame = -1;
		}
		else
		{
			//NOTE: check for d->m_renderer == 0 not required because frameCount
			//is -1 in this case, i.e. this branch is not chosen
			const int frameBaseIndex = d->m_renderer->frameBaseIndex();
			frame = (frame - frameBaseIndex) % frameCount + frameBaseIndex;
		}
		if (d->m_spec.frame != frame)
		{
			d->m_spec.frame = frame;
			d->fetchPixmap();
		}
	}
}

QPixmap KgvRendererClient::pixmap() const
{
	return d->m_pixmap;
}

QSize KgvRendererClient::renderSize() const
{
	return d->m_spec.size;
}

void KgvRendererClient::setRenderSize(const QSize& renderSize)
{
	if (d->m_spec.size != renderSize)
	{
		d->m_spec.size = renderSize;
		d->fetchPixmap();
	}
}

void KgvRendererClientPrivate::fetchPixmap()
{
	if (m_renderer)
	{
		m_renderer->d->requestPixmap(m_spec, m_parent);
	}
	else
	{
		m_parent->receivePixmap(QPixmap());
	}
}
