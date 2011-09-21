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

#include "spriteclient.h"
#include "sprite.h"
#include "sprite_p.h"

#include <QtCore/QTimer>

//WARNING: d->m_sprite == 0 is allowed, and used actively by KGame::Scene.

KGame::SpriteClient::Private::Private(KGame::Sprite* sprite, KGame::SpriteClient* q)
	: q(q)
	, m_sprite(sprite)
	, m_fetcher(0)
	, m_frame(-1)
{
}

KGame::SpriteClient::SpriteClient(KGame::Sprite* sprite)
	: d(new Private(sprite, this))
{
	if (sprite)
	{
		sprite->d->addClient(this);
	}
}

KGame::SpriteClient::~SpriteClient()
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

KGame::Sprite* KGame::SpriteClient::sprite() const
{
	return d->m_sprite;
}

void KGame::SpriteClient::setSprite(KGame::Sprite* sprite)
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
			d->m_fetcher = sprite->d->fetcher(d->m_size, d->m_processingInstruction);
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

int KGame::SpriteClient::frame() const
{
	return d->m_frame;
}

void KGame::SpriteClient::setFrame(int frame)
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

QString KGame::SpriteClient::processingInstruction() const
{
	return d->m_processingInstruction;
}

void KGame::SpriteClient::setProcessingInstruction(const QString& processingInstruction)
{
	if (d->m_processingInstruction != processingInstruction)
	{
		d->m_processingInstruction = processingInstruction;
		KGame::SpriteFetcher* f = 0;
		if (d->m_sprite)
		{
			f = d->m_sprite->d->fetcher(d->m_size, processingInstruction);
		}
		d->setFetcher(f);
	}
}

QSize KGame::SpriteClient::renderSize() const
{
	return d->m_size;
}

void KGame::SpriteClient::setRenderSize(const QSize& size)
{
	if (d->m_size != size)
	{
		d->m_size = size;
		KGame::SpriteFetcher* f = 0;
		if (d->m_sprite)
		{
			f = d->m_sprite->d->fetcher(size, d->m_processingInstruction);
		}
		d->setFetcher(f);
	}
}

void KGame::SpriteClient::Private::setFetcher(KGame::SpriteFetcher* fetcher)
{
	if (m_fetcher == fetcher)
	{
		return;
	}
	if (m_fetcher)
	{
		m_fetcher->removeClient(q);
	}
	m_fetcher = fetcher;
	if (fetcher)
	{
		fetcher->addClient(q);
	}
	else
	{
		receivePixmap(QPixmap());
	}
}

QPixmap KGame::SpriteClient::pixmap() const
{
	return d->m_pixmap;
}

void KGame::SpriteClient::Private::receivePixmap(const QPixmap& pixmap)
{
	m_pixmap = pixmap;
	q->receivePixmap(pixmap);
}
