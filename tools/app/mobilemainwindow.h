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

#ifndef TAPP_MOBILEMAINWINDOW_H
#define TAPP_MOBILEMAINWINDOW_H

class QStackedWidget;
#include <KDE/KMainWindow>
namespace Tagaro
{
	class Game;
}

namespace TApp
{
	class AppListView;
	class ButtonList;
	class Instantiable;

	class MobileMainWindow : public KMainWindow
	{
		Q_OBJECT
		public:
			MobileMainWindow();
		private Q_SLOTS:
			void activate(TApp::Instantiable* inst);
			void showGame(Tagaro::Game* game);
			void showNewDialog();
		private:
			QWidget* m_centralWidget;
			QStackedWidget* m_stackedWidget;
			TApp::ButtonList* m_buttonList;
			TApp::AppListView* m_appListView;
			Tagaro::Game* m_activeGame;
			QString m_defaultWindowTitle;
			QIcon m_defaultWindowIcon;
	};
}

#endif // TAPP_MOBILEMAINWINDOW_H
