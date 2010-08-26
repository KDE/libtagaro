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

#include "rendererclient.h"
#include "renderer.h"
#include "renderer_p.h"

#include <QtCore/QTimer>

//WARNING: d->m_renderer == 0 is allowed, and used actively by Tagaro::View.

Tagaro::RendererClientPrivate::RendererClientPrivate(Tagaro::Renderer* renderer, const QString& spriteKey, Tagaro::RendererClient* parent)
	: m_parent(parent)
	, m_renderer(renderer)
	, m_spec(spriteKey, -1, QSize(3, 3))
	, m_fetching(renderer && !spriteKey.isEmpty())
{
}

Tagaro::RendererClient::RendererClient(Tagaro::Renderer* renderer, const QString& spriteKey)
	: d(new Tagaro::RendererClientPrivate(renderer, spriteKey, this))
{
	if (renderer)
	{
		renderer->d->m_clients.insert(this, QString());
	}
	//The following may not be triggered directly because it may call receivePixmap() which is a pure virtual method at this point.
	QTimer::singleShot(0, d, SLOT(fetchPixmap()));
}

Tagaro::RendererClient::~RendererClient()
{
	if (d->m_renderer)
	{
		d->m_renderer->d->m_clients.remove(this);
	}
	delete d;
}

Tagaro::Renderer* Tagaro::RendererClient::renderer() const
{
	return d->m_renderer;
}

void Tagaro::RendererClient::setRenderer(Tagaro::Renderer* renderer)
{
	if (d->m_renderer != renderer)
	{
		d->m_renderer = renderer;
		d->m_fetching = d->m_renderer && !d->m_spec.spriteKey.isEmpty();
		d->fetchPixmap();
	}
}

QString Tagaro::RendererClient::spriteKey() const
{
	return d->m_spec.spriteKey;
}

void Tagaro::RendererClient::setSpriteKey(const QString& spriteKey)
{
	if (d->m_spec.spriteKey != spriteKey)
	{
		d->m_spec.spriteKey = spriteKey;
		d->m_fetching = d->m_renderer && !d->m_spec.spriteKey.isEmpty();
		d->fetchPixmap();
	}
}

int Tagaro::RendererClient::frameCount() const
{
	return d->m_renderer ? d->m_renderer->frameCount(d->m_spec.spriteKey) : -1;
}

int Tagaro::RendererClient::frame() const
{
	return d->m_spec.frame;
}

void Tagaro::RendererClient::setFrame(int frame)
{
	if (d->m_spec.frame != frame)
	{
		//do some normalization ourselves
		const int frameCount = this->frameCount();
		if (frameCount <= 0 || frame <= 0)
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

QSize Tagaro::RendererClient::renderSize() const
{
	return d->m_spec.size;
}

void Tagaro::RendererClient::setRenderSize(const QSize& renderSize)
{
	if (d->m_spec.size != renderSize)
	{
		d->m_spec.size = renderSize;
		d->fetchPixmap();
	}
}

QPixmap Tagaro::RendererClient::pixmap() const
{
	return d->m_pixmap;
}

void Tagaro::RendererClientPrivate::fetchPixmap()
{
	if (m_fetching)
	{
		m_renderer->d->requestPixmap(m_spec, m_parent);
	}
	else
	{
		receivePixmapInternal(QPixmap());
	}
}

void Tagaro::RendererClientPrivate::receivePixmapInternal(const QPixmap& pixmap)
{
	m_pixmap = pixmap;
	m_parent->receivePixmap(pixmap);
}
