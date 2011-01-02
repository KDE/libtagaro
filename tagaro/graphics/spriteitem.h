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

#ifndef TAGARO_SPRITEITEM_H
#define TAGARO_SPRITEITEM_H

#include <QtGui/QGraphicsPixmapItem>

#include "spriteclient.h"
#include <libtagaro_export.h>

namespace Tagaro {

class SpriteItemPrivate;

/**
 * @class Tagaro::SpriteItem spriteitem.h <Tagaro/SpriteItem>
 * @short A QGraphicsPixmapItem which reacts to theme changes automatically.
 *
 * This class is a QGraphicsPixmapItem which retrieves its pixmap from a
 * Tagaro::Sprite, and updates it automatically when the corresponding
 * Tagaro::ThemeProvider changes the theme.
 */
class TAGARO_EXPORT SpriteItem : public QGraphicsPixmapItem, public Tagaro::SpriteClient
{
	public:
		///Creates a new Tagaro::SpriteItem which renders the given @a sprite.
		explicit SpriteItem(Tagaro::Sprite* sprite, QGraphicsItem* parent = 0);
		virtual ~SpriteItem();
	protected:
		virtual void receivePixmap(const QPixmap& pixmap);
	private:
		friend class Tagaro::SpriteItemPrivate;
		Tagaro::SpriteItemPrivate* const d;
};

} //namespace Tagaro

#endif // TAGARO_SPRITEITEM_H
