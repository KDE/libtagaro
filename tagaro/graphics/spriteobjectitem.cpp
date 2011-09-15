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

#include "spriteobjectitem.h"
#include "spriteobjectitem_p.h"
#include "../interface/board_p.h"

#include <QtCore/qmath.h>
#include <QtGui/QGraphicsScene>

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

Tagaro::SpriteObjectItem::Private::Private(QGraphicsItem* parent)
	: QGraphicsPixmapItem(dummyPixmap(), parent)
	, m_size(1, 1)
	, m_pixmapSize(1, 1)
	, m_board(0)
{
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

Tagaro::SpriteObjectItem::SpriteObjectItem(Tagaro::Sprite* sprite, QGraphicsItem* parent)
	: QGraphicsObject(parent)
	, Tagaro::SpriteClient(sprite)
	, d(new Private(this))
{
	d->findBoardFromParent(this, parent);
}

Tagaro::SpriteObjectItem::~SpriteObjectItem()
{
	//deregister from board, if any
	d->findBoardFromParent(this, 0);
	//usual cleanup
	delete d;
}

void Tagaro::SpriteObjectItem::Private::findBoardFromParent(Tagaro::SpriteObjectItem* q, QGraphicsItem* parent)
{
	//find board among parents
	Tagaro::Board* board = 0;
	while (parent)
	{
		//is parent a board?
		QGraphicsObject* obj = parent->toGraphicsObject();
		if (obj && (board = qobject_cast<Tagaro::Board*>(obj)))
		{
			break;
		}
		//if not, continue with parent of parent
		parent = parent->parentItem();
	}
	//disconnect from old board, connect to new board
	//NOTE: also if m_board == board; findBoardFromParent() was usually called for a reason,
	//and even parent changes inside the same board are a reason for an update delivered by
	//registerItem()
	if (m_board)
	{
		m_board->d->unregisterItem(q);
	}
	m_board = board;
	if (board)
	{
		board->d->registerItem(q);
	}
}

QVariant Tagaro::SpriteObjectItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
	if (change == QGraphicsItem::ItemParentChange)
	{
		d->findBoardFromParent(this, value.value<QGraphicsItem*>());
	}
	return QGraphicsObject::itemChange(change, value);
}

QPointF Tagaro::SpriteObjectItem::offset() const
{
	return d->pos();
}

void Tagaro::SpriteObjectItem::setOffset(const QPointF& offset)
{
	if (d->pos() != offset)
	{
		prepareGeometryChange();
		d->setPos(offset);
		update();
	}
}

QSizeF Tagaro::SpriteObjectItem::size() const
{
	return d->m_size;
}

void Tagaro::SpriteObjectItem::setSize(const QSizeF& size)
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

void Tagaro::SpriteObjectItem::receivePixmap(const QPixmap& pixmap)
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

void Tagaro::SpriteObjectItem::Private::updateTransform()
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
//Tagaro::SpriteObjectItem::Private for Tagaro::SpriteObjectItem.
//Then the relevant methods in Tagaro::SpriteObjectItem::Private are reimplemented
//empty to clear the item and hide it from any collision detection. This
//strategy allows us to use the nifty QGraphicsPixmapItem logic without exposing
//a QGraphicsPixmapItem subclass (which would conflict with QGraphicsObject).

//BEGIN QGraphicsItem reimplementation of Tagaro::SpriteObjectItem

QRectF Tagaro::SpriteObjectItem::boundingRect() const
{
	return d->mapRectToParent(d->QGraphicsPixmapItem::boundingRect());
}

bool Tagaro::SpriteObjectItem::contains(const QPointF& point) const
{
	//return d->QGraphicsPixmapItem::contains(d->mapFromParent(point));
	//This does not work because QGraphicsPixmapItem::contains is actually not
	//implemented. (It is, but it just calls QGraphicsItem::contains as of 4.7.)
	const QPixmap& pixmap = d->pixmap();
	if (pixmap.isNull())
		return false;
	const QPoint pixmapPoint = d->mapFromParent(point).toPoint();
	return pixmap.copy(QRect(pixmapPoint, QSize(1, 1))).toImage().pixel(0, 0);
}

bool Tagaro::SpriteObjectItem::isObscuredBy(const QGraphicsItem* item) const
{
	return d->QGraphicsPixmapItem::isObscuredBy(item);
}

QPainterPath Tagaro::SpriteObjectItem::opaqueArea() const
{
	return d->mapToParent(d->QGraphicsPixmapItem::opaqueArea());
}

void Tagaro::SpriteObjectItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(painter) Q_UNUSED(option) Q_UNUSED(widget)
}

QPainterPath Tagaro::SpriteObjectItem::shape() const
{
	return d->mapToParent(d->QGraphicsPixmapItem::shape());
}

//END QGraphicsItem reimplementation of Tagaro::SpriteObjectItem
//BEGIN QGraphicsItem reimplementation of Tagaro::SpriteObjectItem::Private

bool Tagaro::SpriteObjectItem::Private::contains(const QPointF& point) const
{
	Q_UNUSED(point)
	return false;
}

bool Tagaro::SpriteObjectItem::Private::isObscuredBy(const QGraphicsItem* item) const
{
	Q_UNUSED(item)
	return false;
}

QPainterPath Tagaro::SpriteObjectItem::Private::opaqueArea() const
{
	return QPainterPath();
}

QPainterPath Tagaro::SpriteObjectItem::Private::shape() const
{
	return QPainterPath();
}

//END QGraphicsItem reimplementation of Tagaro::SpriteObjectItem::Private

#include "spriteobjectitem.moc"
