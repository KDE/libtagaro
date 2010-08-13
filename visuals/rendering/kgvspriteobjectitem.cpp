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

#include "kgvspriteobjectitem.h"
#include "kgvrenderer.h"

#include <QtCore/qmath.h>

class KgvSpriteObjectItemPrivate : public QGraphicsPixmapItem
{
	public:
		KgvSpriteObjectItemPrivate(QGraphicsItem* parent);

		//QGraphicsItem reimplementations (see comment below for why we need all of this)
		virtual bool contains(const QPointF& point) const;
		virtual bool isObscuredBy(const QGraphicsItem* item) const;
		virtual QPainterPath opaqueArea() const;
		virtual QPainterPath shape() const;
};

KgvSpriteObjectItemPrivate::KgvSpriteObjectItemPrivate(QGraphicsItem* parent)
	: QGraphicsPixmapItem(parent)
{
}

KgvSpriteObjectItem::KgvSpriteObjectItem(KgvRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent)
	: QGraphicsObject(parent)
	, KgvRendererClient(renderer, spriteKey)
	, d(new KgvSpriteObjectItemPrivate(this))
{
}

KgvSpriteObjectItem::~KgvSpriteObjectItem()
{
	delete d;
}

QPointF KgvSpriteObjectItem::offset() const
{
	return d->pos();
}

void KgvSpriteObjectItem::setOffset(const QPointF& offset)
{
	if (d->pos() != offset)
	{
		prepareGeometryChange();
		d->setPos(offset);
		update();
	}
}

void KgvSpriteObjectItem::setOffset(qreal x, qreal y)
{
	setOffset(QPointF(x, y));
}

void KgvSpriteObjectItem::receivePixmap(const QPixmap& pixmap)
{
	prepareGeometryChange();
	d->setPixmap(pixmap);
	update();
}

//We want to make sure that all interactional events are sent ot this item, and
//not to the contained QGraphicsPixmapItem which provides the visual
//representation (and the metrics calculations).
//At the same time, we do not want the contained QGraphicsPixmapItem to slow
//down operations like QGraphicsScene::collidingItems().
//So the strategy is to use the QGraphicsPixmapItem implementation from
//KgvSpriteObjectItemPrivate for KgvSpriteObjectItem.
//Then the relevant methods in KgvSpriteObjectItemPrivate are reimplemented empty
//to effectively clear the item and hide it from any collision detection. This
//strategy allows us to use the nifty QGraphicsPixmapItem logic without exposing
//a QGraphicsPixmapItem subclass (which would conflict with QGraphicsObject).

//BEGIN QGraphicsItem reimplementation of KgvSpriteObjectItem

QRectF KgvSpriteObjectItem::boundingRect() const
{
	return d->mapRectToParent(d->QGraphicsPixmapItem::boundingRect());
}

bool KgvSpriteObjectItem::contains(const QPointF& point) const
{
	return d->QGraphicsPixmapItem::contains(d->mapFromParent(point));
}

bool KgvSpriteObjectItem::isObscuredBy(const QGraphicsItem* item) const
{
	return d->QGraphicsPixmapItem::isObscuredBy(item);
}

QPainterPath KgvSpriteObjectItem::opaqueArea() const
{
	return d->mapToParent(d->QGraphicsPixmapItem::opaqueArea());
}

void KgvSpriteObjectItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(painter) Q_UNUSED(option) Q_UNUSED(widget)
}

QPainterPath KgvSpriteObjectItem::shape() const
{
	return d->mapToParent(d->QGraphicsPixmapItem::shape());
}

//END QGraphicsItem reimplementation of KgvSpriteObjectItem
//BEGIN QGraphicsItem reimplementation of KgvSpriteObjectItemPrivate

bool KgvSpriteObjectItemPrivate::contains(const QPointF& point) const
{
	Q_UNUSED(point)
	return false;
}

bool KgvSpriteObjectItemPrivate::isObscuredBy(const QGraphicsItem* item) const
{
	Q_UNUSED(item)
	return false;
}

QPainterPath KgvSpriteObjectItemPrivate::opaqueArea() const
{
	return QPainterPath();
}

QPainterPath KgvSpriteObjectItemPrivate::shape() const
{
	return QPainterPath();
}

//END QGraphicsItem reimplementation of KgvSpriteObjectItemPrivate

#include "kgvspriteobjectitem.moc"
