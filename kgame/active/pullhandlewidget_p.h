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

#ifndef KGAME_PULLHANDLE_P_H
#define KGAME_PULLHANDLE_P_H

#include <QtGui/QFrame>

namespace KGame {

class PullHandleWidget : public QFrame
{
	Q_OBJECT
	public:
		enum Edge { LeftEdge, RightEdge, TopEdge, BottomEdge };
		PullHandleWidget(const QString& title, Edge edge, QWidget* parent = 0);

		virtual QSize sizeHint() const;
	Q_SIGNALS:
		///Emitted by mousePressEvent().
		void pulled();
	protected:
		virtual bool event(QEvent* event);
		virtual void paintEvent(QPaintEvent* event);
		virtual void resizeEvent(QResizeEvent* event);
	private:
		QString m_text;
		QSize m_sizeHint;
		int m_pointSize, m_rotation;
};

} //namespace KGame

#endif // KGAME_PULLHANDLE_P_H
