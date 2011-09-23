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

#ifndef KGAME_TOUCHWINDOW_H
#define KGAME_TOUCHWINDOW_H

#include <QtGui/QWidget>

#include <libkgame_export.h>

namespace KGame {

//TODO: document this class
class KGAME_EXPORT TouchWindow : public QWidget
{
	Q_OBJECT
	public:
		TouchWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);
		virtual ~TouchWindow();

		void addAction(QAction* action);
		void addPullHandle(const QString& title, QWidget* widget);

		QWidget* centralWidget() const;
		void setCentralWidget(QWidget* widget);
	Q_SIGNALS:
		void handlePulled(QWidget* contentWidget);
	private:
		class Private;
		Private* const d;

		Q_PRIVATE_SLOT(d, void _t_pullHandlePulled());
};

} // namespace KGame

#endif // KGAME_TOUCHWINDOW_H
