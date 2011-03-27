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

#include "mobilemainwindow.h"
#include "mobilemainwindow_p.h"
#include "applist.h"
#include "instantiable.h"

#include <QtGui/QGridLayout>
#include <QtGui/QStackedWidget>

TApp::MobileMainWindow::MobileMainWindow()
	: m_centralWidget(new QWidget)
	, m_stackedWidget(new QStackedWidget)
	, m_buttonList(new TApp::ButtonList(this))
	, m_appListView(new TApp::AppListView)
	, m_activeGame(0)
	, m_defaultWindowTitle(windowTitle())
	, m_defaultWindowIcon(windowIcon())
{
	//setup app list view
	connect(m_appListView, SIGNAL(selected(TApp::Instantiable*)), SLOT(activate(TApp::Instantiable*)));
	connect(m_buttonList, SIGNAL(selected(Tagaro::Game*)), SLOT(showGame(Tagaro::Game*)));
	connect(m_buttonList, SIGNAL(showNewDialog()), SLOT(showNewDialog()));
	//the grid layout contains the interface bits
	QGridLayout* layout = new QGridLayout(m_centralWidget);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(m_buttonList, 0, 0);
	layout->addWidget(m_appListView, 0, 1);
	layout->addWidget(m_stackedWidget, 0, 1);
	m_stackedWidget->lower();
	setCentralWidget(m_centralWidget);
}

void TApp::MobileMainWindow::activate(TApp::Instantiable* inst)
{
	Tagaro::Game* game = inst->createInstance();
	if (game)
	{
		m_stackedWidget->addWidget(game);
		m_buttonList->createButton(game);
		showGame(game);
	}
}

void TApp::MobileMainWindow::showGame(Tagaro::Game* game)
{
	if (m_activeGame)
		m_activeGame->setActive(false);
	m_activeGame = game;
	m_activeGame->setActive(true);
	m_stackedWidget->setCurrentWidget(m_activeGame);
	m_appListView->setVisible(false);
	m_stackedWidget->setVisible(true);
}

void TApp::MobileMainWindow::showNewDialog()
{
	if (m_activeGame)
		m_activeGame->setActive(false);
	m_activeGame = 0;
	m_appListView->setVisible(true);
	m_stackedWidget->setVisible(false);
	setWindowTitle(m_defaultWindowTitle);
	setWindowIcon(m_defaultWindowIcon);
}

#include "mobilemainwindow.moc"
#include "mobilemainwindow_p.moc"
