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
#include "renderer.h"

//TODO: ACTION PLAN for Tagaro theming 2.0
//TODO: 1. (DONE) replace Renderer dependency of RendererClient by Sprite dependency
//TODO: 2. move rendering logic from Renderer to Sprite
//TODO: 3. move renderer module and cache instantiation from Renderer to Theme
//TODO: 4. (SOMEHOW DONE) move worker pool management from Renderer to an internal global-static class
//TODO: 5. merge Renderer into ThemeProvider; convert legacy properties (frameSuffix, frameBaseIndex) and strategies into a configuration structure that can only be passed in once at ThemeProvider creation

struct Tagaro::Sprite::Private
{
	Tagaro::Renderer* m_renderer;
	QString m_key;

	Private(Tagaro::Renderer* renderer, const QString& key) : m_renderer(renderer), m_key(key) {}
};

Tagaro::Sprite::Sprite(Tagaro::Renderer* renderer, const QString& key)
	: d(new Private(renderer, key))
{
}

Tagaro::Sprite::~Sprite()
{
	delete d;
}

QRectF Tagaro::Sprite::bounds(int frame) const
{
	return d->m_renderer->boundsOnSprite(d->m_key, frame);
}

bool Tagaro::Sprite::exists() const
{
	return d->m_renderer->spriteExists(d->m_key);
}

int Tagaro::Sprite::frameCount() const
{
	return d->m_renderer->frameCount(d->m_key);
}

QString Tagaro::Sprite::key() const
{
	return d->m_key;
}

Tagaro::Renderer* Tagaro::Sprite::renderer() const
{
	return d->m_renderer;
}

QPixmap Tagaro::Sprite::pixmap(const QSize& size, int frame) const
{
	return d->m_renderer->spritePixmap(d->m_key, size, frame);
}
