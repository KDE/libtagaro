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

#include "pullhandlewidget_p.h"

#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>

namespace KGame {

PullHandleWidget::PullHandleWidget(const QString& text, PullHandleWidget::Edge edge,QWidget* parent)
	: QFrame(parent)
	, m_text(text)
	, m_pointSize(font().pointSizeF())
	, m_rotation(0)
{
	setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
	//NOTE: No layout direction dependency here! Vertical text is rotated such
	//      that the baseline is on the opposite side of the screen edge.
	switch (edge)
	{
	case LeftEdge:
		m_rotation = -90;
		break;
	case RightEdge:
		m_rotation = 90;
		break;
	case TopEdge: case BottomEdge:
		break;
	}
	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	resizeEvent(0); //calculate m_sizeHint
}

QSize PullHandleWidget::sizeHint() const
{
	return m_sizeHint;
}

static const int g_textFlags = Qt::TextSingleLine | Qt::TextExpandTabs;

bool PullHandleWidget::event(QEvent* event)
{
	//emit pulled() without really handling the event in a way that
	//is visible to Qt, in order to propagate the event to the
	//TouchWindow/SlideLayout
	if (event->type() == QEvent::MouseButtonPress)
	{
		emit pulled();
	}
	return QFrame::event(event);
}

void PullHandleWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)
	QPainter painter(this);
	drawFrame(&painter);

	QFont font = this->font();
	font.setPointSize(m_pointSize);
	painter.setFont(font);

	painter.rotate(m_rotation);
	const QRect drawRect = painter.transform().inverted().mapRect(rect());
	painter.drawText(drawRect, Qt::AlignCenter | g_textFlags, m_text);
}

void PullHandleWidget::resizeEvent(QResizeEvent* event)
{
	//find size that is available for text rendering
	QSizeF targetSize;
	if (event)
	{
		targetSize = 0.9 * QSizeF(event->size());
	}
	else //just calculating the m_sizeHint
	{
		targetSize = QSizeF(1000, 1000);
	}
	if (m_rotation != 0)
	{
		targetSize = QSizeF(targetSize.height(), targetSize.width());
	}
	//find optimal font size
	QFont font = this->font();
	QSizeF sizeHint;
	for (int iteration = 0; iteration < 3; ++iteration)
	{
		QFontMetrics fm(font);
		const QSizeF size = QFontMetrics(font).size(g_textFlags, m_text);
		if (iteration == 0)
		{
			sizeHint = size;
			if (!event) //just calculating the m_sizeHint
			{
				break;
			}
		}
		const qreal scaleX = targetSize.width() / size.width();
		const qreal scaleY = targetSize.height() / size.height();
		font.setPointSizeF(font.pointSizeF() * qMin<qreal>(scaleX, scaleY));
	}
	m_pointSize = font.pointSizeF();
	//polish size hint
	if (m_rotation != 0)
		sizeHint = QSizeF(sizeHint.height(), sizeHint.width());
	m_sizeHint = (sizeHint / 0.9).toSize();
}

} //namespace KGame
#include "pullhandlewidget_p.moc"
