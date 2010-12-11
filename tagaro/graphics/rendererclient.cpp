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

//WARNING: d->m_sprite == 0 is allowed, and used actively by Tagaro::Scene.

Tagaro::RendererClientPrivate::RendererClientPrivate(Tagaro::Sprite* sprite, Tagaro::RendererClient* parent)
	: m_parent(parent)
	, m_sprite(sprite)
	, m_spec(sprite ? sprite->key() : QString(), -1, QSize())
	, m_fetching(sprite && !sprite->key().isEmpty())
{
}

Tagaro::RendererClient::RendererClient(Tagaro::Sprite* sprite)
	: d(new Tagaro::RendererClientPrivate(sprite, this))
{
	if (sprite)
	{
		sprite->renderer()->d->m_clients.insert(this, QString());
	}
	//The following may not be triggered directly because it may call receivePixmap() which is a pure virtual method at this point.
	QTimer::singleShot(0, d, SLOT(fetchPixmap()));
}

Tagaro::RendererClient::~RendererClient()
{
	if (d->m_sprite)
	{
		d->m_sprite->renderer()->d->m_clients.remove(this);
	}
	delete d;
}

Tagaro::Sprite* Tagaro::RendererClient::sprite() const
{
	return d->m_sprite;
}

void Tagaro::RendererClient::setSprite(Tagaro::Sprite* sprite)
{
	if (d->m_sprite != sprite)
	{
		if (d->m_sprite)
		{
			d->m_sprite->renderer()->d->m_clients.remove(this);
		}
		d->m_sprite = sprite;
		if (sprite)
		{
			d->m_spec.spriteKey = sprite->key();
			d->m_fetching = !d->m_spec.spriteKey.isEmpty();
		}
		else
		{
			d->m_spec.spriteKey.clear();
			d->m_fetching = false;
		}
		d->fetchPixmap();
	}
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
		const int frameCount = d->m_sprite ? d->m_sprite->frameCount() : -1;
		if (frameCount <= 0 || frame <= 0)
		{
			frame = -1;
		}
		else
		{
			//NOTE: check for d->m_sprite == 0 not required because frameCount
			//is -1 in this case, i.e. this branch is not chosen
			const int frameBaseIndex = d->m_sprite->renderer()->frameBaseIndex();
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
		m_sprite->renderer()->d->requestPixmap(m_spec, m_parent);
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
