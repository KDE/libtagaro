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

static QPixmap dummyPixmap()
{
	static QPixmap pix(1, 1);
	static bool first = true;
	if (first)
	{
		pix.fill(Qt::transparent);
		first = false;
	}
	return pix;
}

class KgvSpriteObjectItem::Private : public QGraphicsPixmapItem
{
	public:
		QSizeF m_size, m_pixmapSize;

		Private(QGraphicsItem* parent);
		inline void updateTransform();

		//QGraphicsItem reimplementations (see comment below for why we need all of this)
		virtual bool contains(const QPointF& point) const;
		virtual bool isObscuredBy(const QGraphicsItem* item) const;
		virtual QPainterPath opaqueArea() const;
		virtual QPainterPath shape() const;
};

KgvSpriteObjectItem::Private::Private(QGraphicsItem* parent)
	: QGraphicsPixmapItem(dummyPixmap(), parent)
	, m_size(1, 1)
	, m_pixmapSize(1, 1)
{
}

KgvSpriteObjectItem::KgvSpriteObjectItem(KgvRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent)
	: QGraphicsObject(parent)
	, KgvRendererClient(renderer, spriteKey)
	, d(new Private(this))
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

QSizeF KgvSpriteObjectItem::size() const
{
	return d->m_size;
}

void KgvSpriteObjectItem::setSize(const QSizeF& size)
{
	if (d->m_size != size && size.isValid())
	{
		prepareGeometryChange();
		d->m_size = size;
		d->updateTransform();
		emit sizeChanged(size);
		update();
	}
}

void KgvSpriteObjectItem::receivePixmap(const QPixmap& pixmap)
{
	QPixmap pixmapUse = pixmap.size().isEmpty() ? dummyPixmap() : pixmap;
	const QSizeF pixmapSize = pixmapUse.size();
	if (d->m_pixmapSize != pixmapSize)
	{
		prepareGeometryChange();
		d->m_pixmapSize = pixmapUse.size();
		d->updateTransform();
	}
	d->setPixmap(pixmapUse);
	update();
}

void KgvSpriteObjectItem::Private::updateTransform()
{
	setTransform(QTransform::fromScale(
		m_size.width() / m_pixmapSize.width(),
		m_size.height() / m_pixmapSize.height()
	));
}

//We want to make sure that all interactional events are sent ot this item, and
//not to the contained QGraphicsPixmapItem which provides the visual
//representation (and the metrics calculations).
//At the same time, we do not want the contained QGraphicsPixmapItem to slow
//down operations like QGraphicsScene::collidingItems().
//So the strategy is to use the QGraphicsPixmapItem implementation from
//KgvSpriteObjectItem::Private for KgvSpriteObjectItem.
//Then the relevant methods in KgvSpriteObjectItem::Private are reimplemented
//empty to clear the item and hide it from any collision detection. This
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
//BEGIN QGraphicsItem reimplementation of KgvSpriteObjectItem::Private

bool KgvSpriteObjectItem::Private::contains(const QPointF& point) const
{
	Q_UNUSED(point)
	return false;
}

bool KgvSpriteObjectItem::Private::isObscuredBy(const QGraphicsItem* item) const
{
	Q_UNUSED(item)
	return false;
}

QPainterPath KgvSpriteObjectItem::Private::opaqueArea() const
{
	return QPainterPath();
}

QPainterPath KgvSpriteObjectItem::Private::shape() const
{
	return QPainterPath();
}

//END QGraphicsItem reimplementation of KgvSpriteObjectItem::Private

#include "kgvspriteobjectitem.moc"
