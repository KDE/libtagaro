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

#include "board.h"
#include "board_p.h"
#include "../graphics/spriteobjectitem.h"
#include "../graphics/spriteobjectitem_p.h"

#include <QtGui/QApplication>
#include <QtGui/QGraphicsScene>

Tagaro::Board::Board(QGraphicsItem* parent)
	: QGraphicsObject(parent)
	, d(new Private(this))
{
	setFlag(QGraphicsItem::ItemHasNoContents); //so do not call paint()
	if (parent)
	{
		d->_k_update();
	}
}

Tagaro::Board::~Board()
{
	delete d;
}

Tagaro::Board::Private::~Private()
{
	for(QList<Tagaro::SpriteObjectItem*>::const_iterator a = m_items.constBegin(); a != m_items.constEnd(); ++a)
		(*a)->d->unsetBoard();
}

QSizeF Tagaro::Board::logicalSize() const
{
	return d->m_logicalSize;
}

void Tagaro::Board::setLogicalSize(const QSizeF& size)
{
	if (size.isValid() && d->m_logicalSize != size)
	{
		d->m_logicalSize = size;
		d->_k_update();
	}
}

QSizeF Tagaro::Board::size() const
{
	return d->m_size;
}

void Tagaro::Board::setSize(const QSizeF& size)
{
	if (size.isValid() && d->m_size != size)
	{
		d->m_size = size;
		d->m_alignment = (Qt::Alignment) 0;
		d->_k_update();
	}
}

qreal Tagaro::Board::physicalSizeFactor() const
{
	return d->m_physicalSizeFactor;
}

void Tagaro::Board::setPhysicalSizeFactor(qreal physicalSizeFactor)
{
	if (physicalSizeFactor > 0.0 && d->m_physicalSizeFactor != physicalSizeFactor)
	{
		d->m_physicalSizeFactor = physicalSizeFactor;
		d->_k_update();
	}
}

Qt::Alignment Tagaro::Board::alignment() const
{
	return d->m_alignment;
}

void Tagaro::Board::setAlignment(Qt::Alignment alignment)
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

void Tagaro::Board::Private::_k_update()
{
	//determine physical size
	if (m_alignment)
	{
		//determine base rect (the rect into which we will be layouting)
		QRectF baseRect;
		QGraphicsItem* parentItem = m_board->parentItem();
		if (parentItem)
		{
			baseRect = parentItem->boundingRect();
		}
		else
		{
			QGraphicsScene* scene = m_board->scene();
			baseRect = scene ? scene->sceneRect() : QRectF(QPointF(), m_size);
		}
		//keep aspect ratio
		const qreal scaleX = baseRect.width() / m_logicalSize.width();
		const qreal scaleY = baseRect.height() / m_logicalSize.height();
		m_size = qMin(scaleX, scaleY) * m_logicalSize;
		QRectF physicalRect(baseRect.topLeft(), m_size);
		//horizontal alignment (constructor of physicalRect aligns on left)
		const bool hReverse = !(m_alignment & Qt::AlignAbsolute) && QApplication::isRightToLeft();
		if (m_alignment & Qt::AlignHCenter)
		{
			const qreal dx = (baseRect.width() - m_size.width()) / 2;
			physicalRect.translate(dx, 0);
		}
		else if (m_alignment & Qt::AlignRight || ((m_alignment & Qt::AlignLeft) && hReverse))
		{
			physicalRect.moveRight(baseRect.right());
		}
		//vertical alignment (constructor of physicalRect aligns on top)
		if (m_alignment & Qt::AlignVCenter)
		{
			const qreal dy = (baseRect.height() - m_size.height()) / 2;
			physicalRect.translate(0, dy);
		}
		else if (m_alignment & Qt::AlignBottom)
		{
			physicalRect.moveBottom(baseRect.bottom());
		}
		m_board->setPos(physicalRect.topLeft());
	}
	//update own transform and calculate renderSizeFactor
	m_renderSizeFactor.setX(m_size.width() / m_logicalSize.width());
	m_renderSizeFactor.setY(m_size.height() / m_logicalSize.height());
	m_board->setTransform(QTransform::fromScale(m_renderSizeFactor.x(), m_renderSizeFactor.y()));
	m_renderSizeFactor *= m_physicalSizeFactor;
	//update items
	QList<Tagaro::SpriteObjectItem*>::const_iterator it1 = m_items.constBegin(), it2 = m_items.constEnd();
	for (; it1 != it2; ++it1)
		update(*it1);
}

void Tagaro::Board::Private::_k_updateItem()
{
	Tagaro::SpriteObjectItem* item = qobject_cast<Tagaro::SpriteObjectItem*>(m_board->sender());
	if (item)
	{
		update(item);
	}
}

void Tagaro::Board::Private::update(Tagaro::SpriteObjectItem* item)
{
	QSizeF size;
	if (item->parentItem() == m_board)
	{
		size = item->size();
	}
	else
	{
		size = m_board->mapRectFromItem(item, item->boundingRect()).size();
	}
	size.rwidth() *= m_renderSizeFactor.x();
	size.rheight() *= m_renderSizeFactor.y();
	item->setRenderSize(size.toSize());
}

QRectF Tagaro::Board::boundingRect() const
{
	return QRectF(QPointF(), d->m_size);
}

void Tagaro::Board::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(painter) Q_UNUSED(option) Q_UNUSED(widget)
}

void Tagaro::Board::Private::registerItem(Tagaro::SpriteObjectItem* item)
{
	m_items << item;
	connect(item, SIGNAL(sizeChanged(QSizeF)), m_board, SLOT(_k_updateItem()));
	update(item);
}

void Tagaro::Board::Private::unregisterItem(Tagaro::SpriteObjectItem* item)
{
	disconnect(item, 0, m_board, 0);
	m_items.removeAll(item);
}

QVariant Tagaro::Board::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
	if (change == ItemSceneChange)
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

#include "board.moc"
