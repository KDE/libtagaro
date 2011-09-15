/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>                     *
 *   Copyright 2011 Jeffrey Kelling <>                                     *
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

#ifndef TAGARO_SPRITEOBJECTITEM_P_H
#define TAGARO_SPRITEOBJECTITEM_P_H

#include "spriteobjectitem.h"

namespace Tagaro {
	class Board;
}

class Tagaro::SpriteObjectItem::Private : public QGraphicsPixmapItem
{
	public:
		QSizeF m_size, m_pixmapSize;

		Private(QGraphicsItem* parent);
		inline void updateTransform();

		//relation to Tagaro::Board
		Tagaro::Board* m_board;
		void findBoardFromParent(Tagaro::SpriteObjectItem* q, QGraphicsItem* parent);
		inline void unsetBoard() {m_board = 0;}

		//QGraphicsItem reimplementations (see comment below for why we need all of this)
		virtual bool contains(const QPointF& point) const;
		virtual bool isObscuredBy(const QGraphicsItem* item) const;
		virtual QPainterPath opaqueArea() const;
		virtual QPainterPath shape() const;
};

#endif
