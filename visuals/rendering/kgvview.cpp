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

#include "kgvview.h"

#include <QtGui/QResizeEvent>

struct KgvView::Private
{
	KgvView* m_parent;
	QBrush m_brush;
	bool m_adjustingSceneRect; //This flag is set during forced sceneRect changes, to avoid unneeded adjust() calls.

	Private(KgvView* parent);
	void _k_sceneRectChanged();
	void adjust();
};

KgvView::Private::Private(KgvView* parent)
	: m_parent(parent)
	, m_brush(Qt::white) //only a place-holder, until the first pixmap brush is delivered
	, m_adjustingSceneRect(false)
{
	//The scene rect is adjusted to fit in the view, so no need for scrollbars.
	m_parent->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_parent->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

KgvView::KgvView(QWidget* parent)
	: QGraphicsView(parent)
	, KgvRendererClient(0, QString())
	, d(new Private(this))
{
}

KgvView::KgvView(KgvRenderer* renderer, const QString& spriteKey, QWidget* parent)
	: QGraphicsView(parent)
	, KgvRendererClient(renderer, spriteKey)
	, d(new Private(this))
{
}

KgvView::~KgvView()
{
	delete d;
}

void KgvView::setScene(QGraphicsScene* scene)
{
	QGraphicsScene* oldScene = this->scene();
	if (oldScene)
	{
		disconnect(oldScene, 0, this, 0);
	}
	QGraphicsView::setScene(scene);
	if (scene)
	{
		connect(scene, SIGNAL(sceneRectChanged(QRectF)), SLOT(_k_sceneRectChanged(QRectF)));
		d->adjust();
		//do not apply background brush unless we have a renderer!
		if (renderer())
		{
			scene->setBackgroundBrush(d->m_brush);
		}
	}
}

void KgvView::resizeEvent(QResizeEvent* event)
{
	event->accept();
	d->adjust();
	emit resized(event->size());
}

void KgvView::receivePixmap(const QPixmap& pixmap)
{
	d->m_brush = pixmap;
	QGraphicsScene* scene = this->scene();
	if (scene)
	{
		scene->setBackgroundBrush(d->m_brush);
	}
}

void KgvView::Private::_k_sceneRectChanged()
{
	if (!m_adjustingSceneRect)
	{
		adjust();
	}
}

void KgvView::Private::adjust()
{
	const QRectF sceneRect = m_parent->rect();
	m_adjustingSceneRect = true;
	m_parent->scene()->setSceneRect(sceneRect);
	m_parent->setTransform(QTransform());
	m_parent->setRenderSize(sceneRect.size().toSize());
	m_adjustingSceneRect = false;
}

#include "kgvview.moc"
