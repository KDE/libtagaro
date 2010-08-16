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

#include "kgvboard.h"
#include "kgvspriteobjectitem.h"

#include <QtCore/QBasicTimer>
#include <QtCore/QTimerEvent>
#include <QtGui/QApplication>
#include <QtGui/QGraphicsScene>

struct KgvBoard::Private
{
	KgvBoard* m_board;

	Qt::Alignment m_alignment;
	QSizeF m_logicalSize, m_physicalSize;
	QPointF m_renderSizeFactor;

	QList<KgvSpriteObjectItem*> m_items;
	QList<QGraphicsItem*> m_pendingItems;
	QBasicTimer m_pendingItemsTimer;

	void _k_update();
	inline void update(KgvSpriteObjectItem* item);
	void _k_updateItem();

	Private(KgvBoard* board) : m_board(board), m_alignment(Qt::AlignCenter), m_logicalSize(1, 1), m_physicalSize(1, 1), m_renderSizeFactor(1, 1) {}
};

KgvBoard::KgvBoard(QGraphicsItem* parent)
	: QGraphicsObject(parent)
	, d(new Private(this))
{
	setFlag(QGraphicsItem::ItemHasNoContents); //so do not call paint()
}

KgvBoard::~KgvBoard()
{
	delete d;
}

QSizeF KgvBoard::logicalSize() const
{
	return d->m_logicalSize;
}

void KgvBoard::setLogicalSize(const QSizeF& size)
{
	if (size.isValid() && d->m_logicalSize != size)
	{
		d->m_logicalSize = size;
		d->_k_update();
	}
}

QSizeF KgvBoard::physicalSize() const
{
	return d->m_physicalSize;
}

void KgvBoard::setPhysicalSize(const QSizeF& size)
{
	if (size.isValid() && d->m_physicalSize != size)
	{
		d->m_physicalSize = size;
		d->m_alignment = (Qt::Alignment) 0;
		d->_k_update();
	}
}

Qt::Alignment KgvBoard::alignment() const
{
	return d->m_alignment;
}

void KgvBoard::setAlignment(Qt::Alignment alignment)
{
	//filter Qt::AlignJustify which is not interpreted by this class
	static const Qt::Alignment respectedFlags = (Qt::Alignment) ((Qt::AlignHorizontal_Mask & ~Qt::AlignJustify) | Qt::AlignVertical_Mask);
	alignment &= respectedFlags;
	if (d->m_alignment != alignment)
	{
		d->m_alignment = alignment;
		d->_k_update();
	}
}

void KgvBoard::Private::_k_update()
{
	//determine physical size
	if (m_alignment)
	{
		QGraphicsScene* scene = m_board->scene();
		const QRectF baseRect = scene ? scene->sceneRect() : QRectF(QPointF(), m_physicalSize);
		//keep aspect ratio
		const qreal scaleX = baseRect.width() / m_logicalSize.width();
		const qreal scaleY = baseRect.height() / m_logicalSize.height();
		m_physicalSize = qMin(scaleX, scaleY) * m_logicalSize;
		QRectF physicalRect(baseRect.topLeft(), m_physicalSize);
		//horizontal alignment (constructor of physicalRect aligns on left)
		const bool hReverse = !(m_alignment & Qt::AlignAbsolute) && QApplication::isRightToLeft();
		if (m_alignment & Qt::AlignHCenter)
		{
			const qreal dx = (baseRect.width() - m_physicalSize.width()) / 2;
			physicalRect.translate(dx, 0);
		}
		else if (m_alignment & Qt::AlignRight || ((m_alignment & Qt::AlignLeft) && hReverse))
		{
			physicalRect.moveRight(baseRect.right());
		}
		//vertical alignment (constructor of physicalRect aligns on top)
		if (m_alignment & Qt::AlignVCenter)
		{
			const qreal dy = (baseRect.height() - m_physicalSize.height()) / 2;
			physicalRect.translate(0, dy);
		}
		else if (m_alignment & Qt::AlignBottom)
		{
			physicalRect.moveBottom(baseRect.bottom());
		}
		m_board->setPos(physicalRect.topLeft());
	}
	//update own transform
	m_renderSizeFactor.setX(m_physicalSize.width() / m_logicalSize.width());
	m_renderSizeFactor.setY(m_physicalSize.height() / m_logicalSize.height());
	m_board->setTransform(QTransform::fromScale(m_renderSizeFactor.x(), m_renderSizeFactor.y()));
	//update items
	QList<KgvSpriteObjectItem*>::const_iterator it1 = m_items.constBegin(), it2 = m_items.constEnd();
	for (; it1 != it2; ++it1)
		update(*it1);
}

void KgvBoard::Private::_k_updateItem()
{
	KgvSpriteObjectItem* item = qobject_cast<KgvSpriteObjectItem*>(m_board->sender());
	if (item)
	{
		update(item);
	}
}

void KgvBoard::Private::update(KgvSpriteObjectItem* item)
{
	QSizeF size = item->size();
	size.rwidth() *= m_renderSizeFactor.x();
	size.rheight() *= m_renderSizeFactor.y();
	item->setRenderSize(size.toSize());
}

QRectF KgvBoard::boundingRect() const
{
	return QRectF();
}

void KgvBoard::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(painter) Q_UNUSED(option) Q_UNUSED(widget)
}

QVariant KgvBoard::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
	if (change == ItemChildRemovedChange)
	{
		QGraphicsItem* item = value.value<QGraphicsItem*>();
		d->m_pendingItems.removeAll(item);
		QGraphicsObject* object = qgraphicsitem_cast<QGraphicsObject*>(item);
		KgvSpriteObjectItem* objectItem = qobject_cast<KgvSpriteObjectItem*>(object);
		disconnect(objectItem, 0, this, 0);
		d->m_items.removeAll(objectItem);
	}
	else if (change == ItemChildAddedChange)
	{
		//check if this is a KgvSpriteObjectItem
		QGraphicsItem* item = value.value<QGraphicsItem*>();
		QGraphicsObject* object = qgraphicsitem_cast<QGraphicsObject*>(item);
		KgvSpriteObjectItem* objectItem = qobject_cast<KgvSpriteObjectItem*>(object);
		if (objectItem)
		{
			d->m_items << objectItem;
			connect(objectItem, SIGNAL(sizeChanged(QSizeF)), SLOT(_k_updateItem()));
			d->update(objectItem);
		}
		//if we cannot cast to KgvSpriteObjectItem, it might be that the item is
		//not fully constructed -> check again later
		if (item && (!object || (object && object->metaObject()->className() != QByteArray("QGraphicsObject"))))
		{
			d->m_pendingItems << item;
			if (!d->m_pendingItemsTimer.isActive())
			{
				d->m_pendingItemsTimer.start(0, this);
			}
		}
	}
	else if (change == ItemSceneChange)
	{
		QGraphicsScene* scene = value.value<QGraphicsScene*>();
		if (scene)
		{
			disconnect(scene, 0, this, 0);
		}
	}
	else if (change == ItemSceneHasChanged)
	{
		QGraphicsScene* scene = value.value<QGraphicsScene*>();
		if (scene)
		{
			d->_k_update();
			connect(scene, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(_k_update()));
		}
	}
	return QGraphicsObject::itemChange(change, value);
}

void KgvBoard::timerEvent(QTimerEvent* event)
{
	if (event->timerId() == d->m_pendingItemsTimer.timerId())
	{
		d->m_pendingItemsTimer.stop();
		//process pending ItemChildAddedChanges
		QList<QGraphicsItem*>::const_iterator it1 = d->m_pendingItems.constBegin(), it2 = d->m_pendingItems.constEnd();
		for (; it1 != it2; ++it1)
		{
			QGraphicsItem* item = *it1;
			QGraphicsObject* object = qgraphicsitem_cast<QGraphicsObject*>(item);
			KgvSpriteObjectItem* objectItem = qobject_cast<KgvSpriteObjectItem*>(object);
			if (objectItem)
			{
				d->m_items << objectItem;
				connect(objectItem, SIGNAL(sizeChanged(QSizeF)), SLOT(_k_updateItem()));
				d->update(objectItem);
			}
		}
		d->m_pendingItems.clear();
	}
	else
	{
		QGraphicsObject::timerEvent(event);
	}
}

#include "kgvboard.moc"
