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

#include "mainwindow.h"
#include "applist.h"
#include "instantiable.h"

#include <KDE/KActionCollection>
#include <KDE/KIcon>
#include <KDE/KStandardAction>
#include <KDE/KTabWidget>
#include <Tagaro/Game>

TApp::MainWindow::MainWindow()
	: m_tabWidget(new KTabWidget)
	, m_activeGame(0)
	, m_defaultWindowTitle(windowTitle())
	, m_defaultWindowIcon(windowIcon())
{
	setupActions();
	setupGUI(KXmlGuiWindow::StandardWindowOptions(KXmlGuiWindow::Default & ~KXmlGuiWindow::StatusBar));
	setCentralWidget(m_tabWidget);
	//setup tab widget
	m_tabWidget->setDocumentMode(true);
	m_tabWidget->setTabsClosable(true);
	m_tabWidget->setMovable(true);
	connect(m_tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
	connect(m_tabWidget, SIGNAL(currentChanged(int)), SLOT(selectTab(int)));
	//open first "New game" dialog
	actionNew();
}

void TApp::MainWindow::setupActions()
{
	KStandardAction::openNew(this, SLOT(actionNew()), actionCollection());
}

void TApp::MainWindow::activate(TApp::Instantiable* inst)
{
	Tagaro::Game* game = inst->createInstance();
	if (game)
	{
		//if "New game" tab is open, close it
		QWidget* currentWidget = m_tabWidget->currentWidget();
		if (!qobject_cast<Tagaro::Game*>(currentWidget))
		{
			m_tabWidget->removeTab(m_tabWidget->currentIndex());
			currentWidget->deleteLater();
		}
		//display game interface
		if (game->parent() != m_tabWidget)
		{
			const QIcon icon = game->windowIcon();
			const QString title = game->windowTitle();
			if (m_tabWidget->count() == 0)
				m_tabWidget->addTab(game, icon, title);
			else
				m_tabWidget->insertTab(m_tabWidget->currentIndex() - 1, game, icon, title);
			//TODO: Changes to icon/title won't propagate to the tab icon/title.
		}
		m_tabWidget->setCurrentWidget(game);
	}
}

void TApp::MainWindow::closeTab(int index)
{
	QWidget* widget = m_tabWidget->widget(index);
	if (qobject_cast<Tagaro::Game*>(widget))
	{
		m_tabWidget->removeTab(index);
		widget->deleteLater();
		//if this was the last open tab, open a "New game" tab
		if (m_tabWidget->count() == 0)
			actionNew();
	}
	//not a game -> must be a "New game" tab -> allow close unless it's the only open tab
	else if (m_tabWidget->count() > 1)
	{
		m_tabWidget->removeTab(index);
		widget->deleteLater();
	}
}

void TApp::MainWindow::selectTab(int index)
{
	//determine active game
	QWidget* widget = m_tabWidget->widget(index);
	Tagaro::Game* game = qobject_cast<Tagaro::Game*>(widget);
	if (m_activeGame != game)
	{
		if (m_activeGame)
			m_activeGame->setActive(false);
		m_activeGame = game;
		if (m_activeGame)
			m_activeGame->setActive(true);
		else
		{
			//restore game-less state
			setWindowTitle(m_defaultWindowTitle);
			setWindowIcon(m_defaultWindowIcon);
		}
	}
}


void TApp::MainWindow::actionNew()
{
	//create new "New game" tab
	TApp::AppListView* view = new TApp::AppListView;
	connect(view, SIGNAL(selected(TApp::Instantiable*)), SLOT(activate(TApp::Instantiable*)));
	m_tabWidget->addTab(view, KIcon("document-new"), i18n("Start new game"));
	m_tabWidget->setCurrentWidget(view);
}

#include "mainwindow.moc"
