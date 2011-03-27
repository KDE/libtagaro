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
{
	setupActions();
	setupGUI(KXmlGuiWindow::StandardWindowOptions(KXmlGuiWindow::Default & ~KXmlGuiWindow::StatusBar));
	setCentralWidget(m_tabWidget);
	//setup tab widget
	m_tabWidget->setDocumentMode(true);
	m_tabWidget->setTabsClosable(true);
	m_tabWidget->setMovable(true);
	connect(m_tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
	//open first "New game" dialog
	actionNew();
}

TApp::MainWindow::~MainWindow()
{
	//close() all loaded Instantiables properly
	const int count = m_tabWidget->count();
	QList<QWidget*> widgets;
	for (int i = 0; i < count; ++i)
		widgets << m_tabWidget->widget(i);
	foreach (QWidget* widget, widgets)
	{
		TApp::Instantiable* inst = TApp::Instantiable::forWidget(widget);
		if (inst)
			inst->close();
	}
}

void TApp::MainWindow::setupActions()
{
	KStandardAction::openNew(this, SLOT(actionNew()), actionCollection());
}

void TApp::MainWindow::activate(TApp::Instantiable* game)
{
	if (!game->activate())
		return;
	//check if this game is already open
	QWidget* widget = game->game();
	if (widget)
	{
		//if "New game" tab is open, close it
		if (!TApp::Instantiable::forWidget(m_tabWidget->currentWidget()))
			delete m_tabWidget->currentWidget();
		//display interface of game
		if (widget->parent() != m_tabWidget)
		{
			const QIcon icon = game->data(Qt::DecorationRole).value<QIcon>();
			const QString title = game->data(Qt::DisplayRole).toString();
			if (m_tabWidget->count() == 0)
				m_tabWidget->addTab(widget, icon, title);
			else
				m_tabWidget->insertTab(m_tabWidget->currentIndex() - 1, widget, icon, title);
		}
		m_tabWidget->setCurrentWidget(widget);
	}
}

void TApp::MainWindow::closeTab(int index)
{
	QWidget* widget = m_tabWidget->widget(index);
	TApp::Instantiable* inst = TApp::Instantiable::forWidget(widget);
	if (inst)
	{
		inst->close();
		//if this was the last open tab, open a "New game" tab
		if (m_tabWidget->count() == 0)
			actionNew();
	}
	//!inst -> must be a "New game" tab -> allow close unless it's the only open tab
	else if (m_tabWidget->count() > 1)
		delete widget;
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
