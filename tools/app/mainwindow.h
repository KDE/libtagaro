/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#ifndef TAPP_MAINWINDOW_H
#define TAPP_MAINWINDOW_H

class KTabWidget;
#include <KDE/KXmlGuiWindow>
namespace Tagaro
{
	class Game;
}

namespace TApp
{
	class Instantiable;

	class MainWindow : public KXmlGuiWindow
	{
		Q_OBJECT
		public:
			MainWindow();
		private Q_SLOTS:
			void actionNew();
			void activate(TApp::Instantiable* inst);
			void closeTab(int index);
			void selectTab(int index);
		private:
			void setupActions();

			KTabWidget* m_tabWidget;
			Tagaro::Game* m_activeGame;
			QString m_defaultWindowTitle;
			QIcon m_defaultWindowIcon;
	};
}

#endif // TAPP_MAINWINDOW_H
