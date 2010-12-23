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
#include "sprite.h"
#include "sprite_p.h"

#include <QtCore/QTimer>

//WARNING: d->m_sprite == 0 is allowed, and used actively by Tagaro::Scene.

Tagaro::RendererClient::Private::Private(Tagaro::Sprite* sprite, Tagaro::RendererClient* q)
	: q(q)
	, m_sprite(sprite)
	, m_fetcher(0)
	, m_frame(-1)
{
}

Tagaro::RendererClient::RendererClient(Tagaro::Sprite* sprite)
	: d(new Private(sprite, this))
{
	if (sprite)
	{
		sprite->d->addClient(this);
	}
}

Tagaro::RendererClient::~RendererClient()
{
	//This is setSprite(0), but that can't be called directly because this might
	//call receivePixmap() which is pure virtual at this point.
	if (d->m_sprite)
	{
		if (d->m_fetcher)
		{
			d->m_fetcher->removeClient(this);
		}
		d->m_sprite->d->removeClient(this);
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
			if (d->m_fetcher)
			{
				d->m_fetcher->removeClient(this);
			}
			d->m_sprite->d->removeClient(this);
		}
		d->m_sprite = sprite;
		d->m_fetcher = 0;
		if (sprite)
		{
			sprite->d->addClient(this);
			d->m_fetcher = sprite->d->fetcher(d->m_size);
		}
		if (d->m_fetcher)
		{
			d->m_fetcher->addClient(this);
		}
		else
		{
			d->receivePixmap(QPixmap());
		}
	}
}

int Tagaro::RendererClient::frame() const
{
	return d->m_frame;
}

void Tagaro::RendererClient::setFrame(int frame)
{
	if (d->m_frame != frame)
	{
		d->m_frame = frame;
		if (d->m_fetcher)
		{
			d->m_fetcher->updateClient(this);
		}
	}
}

QSize Tagaro::RendererClient::renderSize() const
{
	return d->m_size;
}

void Tagaro::RendererClient::setRenderSize(const QSize& size)
{
	if (d->m_size != size)
	{
		d->m_size = size;
		Tagaro::SpriteFetcher* fetcher = d->m_sprite ? d->m_sprite->d->fetcher(size) : 0;
		if (d->m_fetcher != fetcher)
		{
			if (d->m_fetcher)
			{
				d->m_fetcher->removeClient(this);
			}
			d->m_fetcher = fetcher;
			if (fetcher)
			{
				fetcher->addClient(this);
			}
			else
			{
				d->receivePixmap(QPixmap());
			}
		}
	}
}

QPixmap Tagaro::RendererClient::pixmap() const
{
	return d->m_pixmap;
}

void Tagaro::RendererClient::Private::receivePixmap(const QPixmap& pixmap)
{
	m_pixmap = pixmap;
	q->receivePixmap(pixmap);
}
