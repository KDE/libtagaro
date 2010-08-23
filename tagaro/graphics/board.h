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

#ifndef TAGARO_BOARD_H
#define TAGARO_BOARD_H

#include <QtGui/QGraphicsObject>

#include <libtagaro_export.h>

TAGARO_BEGIN_NAMESPACE

/**
 * @class Tagaro::Board board.h <Tagaro/Board>
 *
 * The Tagaro::Board is basically a usual QGraphicsItem which can be used to
 * group items. However, it has two special features:
 * @li It can adjust its size automatically to fit into the bounding rect of the
 *     parent item (or the scene rect, if there is no parent item). This
 *     behavior is controlled by the alignment() property.
 * @li When it is resized, it will automatically adjust the renderSize of any
 *     contained Tagaro::SpriteObjectItems.
 */
class TAGARO_EXPORT Board : public QGraphicsObject
{
	Q_OBJECT
	public:
		///Creates a new Tagaro::Board instance below the given @a parent item.
		///The logicalSize() is initialized to (1,1). The physicalSize() is
		///determined from the parent item's bounding rect by using the default
		///alignment Qt::AlignCenter.
		Board(QGraphicsItem* parent = 0);
		///Destroys the Tagaro::Board and all its children.
		virtual ~Board();

		///@return the logical size of this board, i.e. the size of its
		///bounding rect in the item's local coordinates
		QSizeF logicalSize() const;
		///Sets the logical size of this board, i.e. the size of its bounding
		///rect in the item's local coordinates.
		void setLogicalSize(const QSizeF& size);
		///@return the physical size of this board, i.e. the size of its
		///bounding rect in the parent item's local coordinates
		QSizeF physicalSize() const;
		///Sets the physical size of this board, i.e. the size of its
		///bounding rect in the parent item's local coordinates.
		///@warning Calls to this method will disable automatic alignment by
		///setting the Tagaro::Board::alignment() to 0.
		void setPhysicalSize(const QSizeF& size);

		///@return the alignment of this board in the parent item's bounding
		///rect (or the scene rect, if there is no parent item)
		Qt::Alignment alignment() const;
		///Sets the @a alignment of this board in the parent item's bounding
		///rect (or the scene rect, if there is no parent item). If an alignment
		///is set, changes to the scene rect will cause the board to change its
		///size and location to fit into the parent item. The board keeps its
		///aspect ratio and determines its position from the @a alignment.
		///
		///The default alignment is Qt::AlignCenter. Call this function with
		///argument 0 to disable the alignment behavior.
		///@note The flag Qt::AlignJustify is not interpreted.
		void setAlignment(Qt::Alignment alignment);

		virtual QRectF boundingRect() const;
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
	protected:
		virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);
		virtual void timerEvent(QTimerEvent* event);
	private:
		class Private;
		Private* const d;
		Q_PRIVATE_SLOT(d, void _k_update());
		Q_PRIVATE_SLOT(d, void _k_updateItem());
};

TAGARO_END_NAMESPACE

#endif // TAGARO_BOARD_H
