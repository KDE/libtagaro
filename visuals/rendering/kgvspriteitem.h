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

#ifndef KGVSPRITEITEM_H
#define KGVSPRITEITEM_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsItem>

#include <kgvrendererclient.h>
#include <libkgame_export.h>

class KgvSpriteItemPrivate;

/**
 * @class KgvSpriteItem kgvspriteitem.h <KgvSpriteItem>
 * @short A QGraphicsPixmapItem which reacts to theme changes automatically.
 *
 * This class is a QGraphicsPixmapItem which retrieves its pixmap from a
 * KgvRenderer, and updates it automatically when the KgvRenderer changes
 * the theme.
 */
class KGAMEVISUALS_EXPORT KgvSpriteItem : public QGraphicsPixmapItem, public KgvRendererClient
{
	public:
		///Creates a new KgvSpriteItem which renders the sprite with the
		///given @a spriteKey as provided by the given @a renderer.
		KgvSpriteItem(KgvRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent = 0);
		virtual ~KgvSpriteItem();
	protected:
		virtual void receivePixmap(const QPixmap& pixmap);
	private:
		friend class KgvSpriteItemPrivate;
		KgvSpriteItemPrivate* const d;
};

#endif // KGVSPRITEITEM_H
