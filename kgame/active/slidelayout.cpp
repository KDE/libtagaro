/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>                     *
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

#include "slidelayout_p.h"

#include <numeric>
#include <QtGui/QMouseEvent>
#include <QtGui/QWidget>

namespace KGame {

class SlideFixation : public QLayoutItem
{
	public:
		SlideFixation() {}

		//empty implementation
		virtual Qt::Orientations expandingDirections() const { return 0; }
		virtual QRect geometry() const { return QRect(); }
		virtual bool isEmpty() const { return true; }
		virtual QSize maximumSize() const { return QSize(0, 0); }
		virtual QSize minimumSize() const { return QSize(0, 0); }
		virtual void setGeometry(const QRect&) {}
		virtual QSize sizeHint() const { return QSize(0, 0); }
};

SlideLayout::SlideLayout(QWidget* parent)
	: QLayout(parent)
	, m_scrollOffset(this)
	, m_lastMousePos(-1, -1) // special value: no mouse interaction ATM
	, m_currentScrollTarget(0)
{
	parent->installEventFilter(this);
	m_scrollOffset.setEasingCurve(QEasingCurve::OutCubic);
}

SlideLayout::~SlideLayout()
{
	while (QLayoutItem* item = takeAt(0))
	{
		delete item;
	}
}

int SlideLayout::count() const
{
	return m_items.count();
}

QLayoutItem* SlideLayout::itemAt(int index) const
{
	return m_items.value(index);
}

void SlideLayout::addItem(QLayoutItem* item)
{
	m_items << item;
	prepareMetrics(); //TODO: necessary?
}

QLayoutItem* SlideLayout::takeAt(int index)
{
	if (index < 0 || index >= count())
	{
		return 0;
	}
	return m_items.takeAt(index);
}

void SlideLayout::addFixation()
{
	addItem(new SlideFixation);
}

void SlideLayout::addLayout(QLayout* layout)
{
	addChildLayout(layout);
	addItem(layout);
}

void SlideLayout::setCurrentScrollTarget(int index)
{
	m_currentScrollTarget = index;
	m_scrollOffset = m_scrollTargets.value(index);
}

QSize SlideLayout::sizeHint() const
{
	return m_sizeHint;
}

QSize SlideLayout::minimumSize() const
{
	//just some random hardcoded minimum
	return QSize(100, 100);
}

void SlideLayout::setGeometry(const QRect& rect)
{
	prepareMetrics();
	alignMetrics(rect.width());
	//reset scrollOffset to current scroll target
	const int scrollOffset = m_scrollTargets.value(m_currentScrollTarget);
	if (m_scrollOffset.state() == QAbstractAnimation::Running)
	{
		if (m_scrollOffset.endValue() != scrollOffset)
		{
			m_scrollOffset.stop();
			m_scrollOffset.setStartValue((int) m_scrollOffset);
			m_scrollOffset.setEndValue(scrollOffset);
			m_scrollOffset.start();
		}
	}
	else if (m_lastMousePos == QPoint(-1, -1)) //do not adjust during interaction
	{
		m_scrollOffset = scrollOffset;
	}
	//do layouting
	QRect itemRect(rect);
	itemRect.setLeft(-m_scrollOffset);
	for (int i = 0; i < m_items.size(); ++i)
	{
		itemRect.setWidth(m_itemWidths[i]);
		m_items[i]->setGeometry(itemRect);
		itemRect.setLeft(itemRect.left() + itemRect.width());
	}
}

///This fetches metrics information from the contained widgets. All jobs in
///here are independent from the actual geometry of the layout. The tasks
///relating to this geometry are in alignMetrics(), which assumes that this
///method has already been called.
void SlideLayout::prepareMetrics()
{
	m_itemWidths.resize(m_items.count());
	m_sizeHint = QSize();
	for (int i = 0; i < m_items.size(); ++i)
	{
		const QSize sh = m_items[i]->sizeHint();
		m_itemWidths[i] = sh.width();
		m_sizeHint.setWidth(m_sizeHint.width() + sh.width());
		m_sizeHint.setHeight(qMax<int>(m_sizeHint.height(), sh.height()));
	}
}

///This is the second half of the layout calculation process (after
///prepareMetrics()), which depends on the geometry to be filled.
void SlideLayout::alignMetrics(int width)
{
	//which are fixations?
	QVector<int> fixIndexes;
	for (int i = 0; i < m_items.size(); ++i)
	{
		if (dynamic_cast<SlideFixation*>(m_items[i]))
		{
			fixIndexes.append(i);
		}
	}
	//between fixations...
	for (int j = 1; j < fixIndexes.size(); ++j)
	{
		const int if1 = fixIndexes[j - 1]; //index in m_items of left fixation
		const int if2 = fixIndexes[j];     //index in m_items of right fixation
		//...sum hinted total width...
		const int hintedWidth = std::accumulate(
			m_itemWidths.constBegin() + if1 + 1,
			m_itemWidths.constBegin() + if2, 0
		);
		const int missingWidth = width - hintedWidth;
		//...and increase total width to parent width, using simplest strategy
		//of expanding the first layout item that expands in the given direction
		bool found = false;
		for (int i = if1 + 1; i < if2; ++i)
		{
			if (m_items[i]->expandingDirections() & Qt::Horizontal)
			{
				m_itemWidths[i] += missingWidth;
				found = true;
				break;
			}
		}
		//if there are no expanding items, look for items that can grow
		//if necessary
		if (!found)
		{
			for (int i = if1 + 1; i < if2; ++i)
			{
				if (QWidget* widget = m_items[i]->widget())
				{
					const QSizePolicy p = widget->sizePolicy();
					if (p.horizontalPolicy() & QSizePolicy::GrowFlag)
					{
						m_itemWidths[i] += missingWidth;
						break;
					}
				}
			}
		}
	}
	//find scroll targets (fixation-aligned stop values of m_scrollOffset)
	if (fixIndexes.size() < 2)
		m_scrollTargets = QVector<int>(1, 0);
	else
	{
		m_scrollTargets.clear();
		m_scrollTargets.resize(fixIndexes.size() + 1);
		m_scrollTargets[0] = 0;
		for (int j = 0; j < fixIndexes.size() - 1; ++j)
		{
			m_scrollTargets[j + 1] = std::accumulate(
				m_itemWidths.constBegin(),
				m_itemWidths.constBegin() + fixIndexes[j],
			0);
		}
		m_scrollTargets[fixIndexes.size()] = std::accumulate(
			m_itemWidths.constBegin(),
			m_itemWidths.constEnd(),
		0) - width;
	}
}

bool SlideLayout::eventFilter(QObject* watched, QEvent* event_)
{
	if (watched != parentWidget())
	{
		return QObject::eventFilter(watched, event_);
	}
	//skip unwanted events
	QMouseEvent* const event = dynamic_cast<QMouseEvent*>(event_);
	if (!event)
		return false;
	//handle all mouse events
	const QEvent::Type type = event->type();
	event->accept();
	if (type == QEvent::MouseButtonPress)
	{
		mousePressEvent(event);
	}
	else if (type == QEvent::MouseMove)
	{
		mouseMoveEvent(event);
	}
	else if (type == QEvent::MouseButtonRelease)
	{
		mouseReleaseEvent(event);
	}
	return true;
}

void SlideLayout::mousePressEvent(QMouseEvent* event)
{
	m_lastMousePos = event->globalPos();
	m_scrollOffset.stop();
}

void SlideLayout::mouseMoveEvent(QMouseEvent* event)
{
	//find how much mouse moved since last time
	const QPoint diff = event->globalPos() - m_lastMousePos;
	m_lastMousePos = event->globalPos();
	//move content (inside bounds of m_scrollTargetsc, trigger relayouting
	const int newScrollOffset = qBound<int>(
		m_scrollTargets.front(),
		m_scrollOffset - diff.x(),
		m_scrollTargets.back()
	);
	if (m_scrollOffset != newScrollOffset)
	{
		m_scrollOffset = newScrollOffset;
		invalidate();
	}
}

void SlideLayout::mouseReleaseEvent(QMouseEvent* event)
{
	Q_UNUSED(event)
	//find nearest scroll target
	int bestScrollTarget = 0, bestDistance = 100000;
	for (int j = 0; j < m_scrollTargets.size(); ++j)
	{
		const int distance = qAbs<int>(m_scrollTargets[j] - m_scrollOffset);
		if (distance < bestDistance)
		{
			bestScrollTarget = j;
			bestDistance = distance;
		}
	}
	//for small deviations from current scroll target, go to next scroll target
	//in that direction to support flicking
	if (bestDistance != 0 && bestScrollTarget == m_currentScrollTarget)
	{
		if (m_scrollOffset > m_scrollTargets[bestScrollTarget])
		{
			bestScrollTarget += 1;
		}
		else
		{
			bestScrollTarget -= 1;
		}
	}
	//move to scroll target to conclude mouse interaction
	m_currentScrollTarget = bestScrollTarget;
	const int newScrollOffset = m_scrollTargets[bestScrollTarget];
	const int optimalDuration = qAbs(5 * (newScrollOffset - m_scrollOffset));
	m_scrollOffset.setDuration(qMin<int>(250, optimalDuration));
	m_scrollOffset.setStartValue(QVariant::fromValue<int>(m_scrollOffset));
	m_scrollOffset.setEndValue(newScrollOffset);
	m_scrollOffset.start();
	//mark that no mouse interaction is running
	m_lastMousePos = QPoint(-1, -1);
}

} //namespace KGame
