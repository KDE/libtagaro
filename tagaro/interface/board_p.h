/***************************************************************************
 *   Copyright 2010-2011 Stefan Majewsky <majewsky@gmx.net>                *
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

#ifndef TAGARO_BOARD_P_H
#define TAGARO_BOARD_P_H

#include "board.h"

class Tagaro::Board::Private
{
	friend class Tagaro::Board;
	Tagaro::Board* m_board;

	Qt::Alignment m_alignment;
	QSizeF m_logicalSize, m_size;
	qreal m_physicalSizeFactor;
	QPointF m_renderSizeFactor;

	QList<Tagaro::SpriteObjectItem*> m_items;

	void _k_update();
	void update(Tagaro::SpriteObjectItem* item);
	void _k_updateItem();

	Private(Tagaro::Board* board) : m_board(board), m_alignment(Qt::AlignCenter), m_logicalSize(1, 1), m_size(1, 1), m_physicalSizeFactor(1), m_renderSizeFactor(1, 1) {}

	public: //interface to Tagaro::SpriteObjectItem
		void registerItem(Tagaro::SpriteObjectItem* item);
		void unregisterItem(Tagaro::SpriteObjectItem* item);
};

#endif // TAGARO_BOARD_P_H
