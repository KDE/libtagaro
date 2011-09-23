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

#ifndef KGAME_SLIDELAYOUT_P_H
#define KGAME_SLIDELAYOUT_P_H

#include <QtCore/QVariantAnimation>
#include <QtCore/QVector>
#include <QtGui/QLayout>
class QMouseEvent;

namespace KGame {

class SlideLayout : public QLayout
{
	public:
		SlideLayout(QWidget* parent);
		~SlideLayout();

		///Fixations are used to calculate the sizes of the contained widgets.
		///Every set of widgets between two fixations will be scaled to the full
		///parent's width. For example, @code
		/// SlideLayout layout(Qt::Horizontal);
		/// layout.addFixation();
		/// layout.addWidget(view);
		/// layout.addWidget(button);
		/// layout.addFixation();
		/// layout.addWidget(menu);
		///@endcode will create a layout of three widgets. The sum of the widths
		///will be equal to the parent's width, while the menu will get its
		///hinted width.
		void addFixation();
		void addLayout(QLayout* layout);

		///Move the @a index-th fixation to the left edge of the parent widget,
		///immediately (without animation).
		void setCurrentScrollTarget(int index);

		//maintaining the list of layout items
		virtual int count() const;
		virtual QLayoutItem* itemAt(int index) const;
		virtual void addItem(QLayoutItem* item);
		virtual QLayoutItem* takeAt(int index);

		//layouting
		virtual QSize sizeHint() const;
		virtual QSize minimumSize() const;
		virtual void setGeometry(const QRect& rect);
	protected:
		virtual bool eventFilter(QObject* watched, QEvent* event);
		void mousePressEvent(QMouseEvent* event);
		void mouseMoveEvent(QMouseEvent* event);
		void mouseReleaseEvent(QMouseEvent* event);
	private:
		void prepareMetrics();
		void alignMetrics(int width);

		QList<QLayoutItem*> m_items;
		QSize m_sizeHint;
		QVector<int> m_itemWidths;

		class ScrollOffset : public QVariantAnimation
		{
			public:
				ScrollOffset(QLayout* parent) : m_value(0), m_parent(parent) {}
				operator int() const { return m_value; }
				operator int&() { return m_value; }
				void operator=(int value) { m_value = value; }
			protected:
				virtual void updateCurrentValue(const QVariant& value)
				{
					if (m_value != value.toInt())
					{
						m_value = value.toInt();
						m_parent->invalidate();
					}
				}
			private:
				int m_value;
				QLayout* m_parent;
		};
		ScrollOffset m_scrollOffset;
		QVector<int> m_scrollTargets;
		QPoint m_lastMousePos;
		int m_currentScrollTarget;
};

} //namespace KGame

#endif // KGAME_SLIDELAYOUT_P_H
