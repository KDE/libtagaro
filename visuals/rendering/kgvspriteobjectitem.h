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

#ifndef KGVSPRITEOBJECTITEM_H
#define KGVSPRITEOBJECTITEM_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsItem>

#include <kgvrendererclient.h>
#include <libkgame_export.h>

/**
 * @class KgvSpriteObjectItem kgvspriteobjectitem.h <KgvSpriteObjectItem>
 * @since 4.6
 * @short A QGraphicsObject which displays pixmaps from a KgvRenderer.
 *
 * This item displays a pixmap which is retrieved from a KgvRenderer, and is
 * updated automatically when the KgvRenderer changes the theme.
 *
 * The item has built-in handling for animated sprites (i.e. those with multiple
 * frames). It is a QGraphicsObject and exposes a "frame" property, so you can
 * easily run the animation by plugging in a QPropertyAnimation.
 *
 * The most important difference to KgvSpriteItem, which is a classic
 * QGraphicsPixmapItem, is that the bounding rectangle of this item is fixed
 * to QRectF(offset(), size()). The rendered pixmap is scaled to fit into this
 * bounding rectangle, regardless of its render size.
 *
 * To automatically adjust the render size to the physical coordinate system of
 * a QGraphicsView, insert the KgvSpriteObjectItem into a KgvBoard.
 */
class KGAMEVISUALS_EXPORT KgvSpriteObjectItem : public QGraphicsObject, public KgvRendererClient
{
	Q_OBJECT
	Q_PROPERTY(int frame READ frame WRITE setFrame)
	Q_PROPERTY(QPointF offset READ offset WRITE setOffset)
	Q_PROPERTY(QSize renderSize READ renderSize WRITE setRenderSize)
	Q_PROPERTY(QSizeF size READ size WRITE setSize)
	public:
		///Creates a new KgvSpriteObjectItem which renders the sprite with
		///the given @a spriteKey as provided by the given @a renderer.
		KgvSpriteObjectItem(KgvRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent = 0);
		virtual ~KgvSpriteObjectItem();

		///@return the item's offset, which defines the point of the top-left
		///corner of the bounding rect, in local coordinates.
		QPointF offset() const;
		///Sets the item's offset, which defines the point of the top-left
		///corner of the bounding rect, in local coordinates.
		void setOffset(const QPointF& offset);
		///@overload
		inline void setOffset(qreal x, qreal y);
		///@return the size of the item's bounding rect
		QSizeF size() const;
		///Sets the size of the item's bounding rect. The rendered pixmap is
		///scaled to fit in this size, regardless of the render size.
		void setSize(const QSizeF& size);
		///@overload
		inline void setSize(qreal width, qreal height);

		//QGraphicsItem reimplementations (see comment in source file for why we need all of this)
		virtual QRectF boundingRect() const;
		virtual bool contains(const QPointF& point) const;
		virtual bool isObscuredBy(const QGraphicsItem* item) const;
		virtual QPainterPath opaqueArea() const;
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
		virtual QPainterPath shape() const;
	protected:
		virtual void receivePixmap(const QPixmap& pixmap);
	private:
		class Private;
		Private* const d;
};

void KgvSpriteObjectItem::setOffset(qreal x, qreal y)
{
	setOffset(QPointF(x, y));
}

void KgvSpriteObjectItem::setSize(qreal w, qreal h)
{
	setSize(QSizeF(w, h));
}

#endif // KGVSPRITEOBJECTITEM_H
