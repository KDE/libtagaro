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

#include "instantiable.h"

#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <KDE/KIcon>
#include <KDE/KMessageBox>
#include <KDE/KServiceTypeTrader>
#include <KDE/KToolInvocation>

//BEGIN TApp::Instantiable

TApp::Instantiable::Instantiable()
	: m_running(false)
	, m_activated(false)
	, m_widget(0)
{
}

TApp::Instantiable::~Instantiable()
{
	//NOTE: It's too late for close() because the vtable has been destroyed.
}

QWidget* TApp::Instantiable::widget() const
{
	return m_widget;
}

/*static*/ TApp::Instantiable* TApp::Instantiable::forWidget(QWidget* widget)
{
	return widget->property("_t_instantiable").value<TApp::Instantiable*>();
}

bool TApp::Instantiable::open()
{
	if (m_running)
		return true;
	m_running = createInstance(m_widget);
	if (!m_running)
		m_widget = 0;
	if (m_widget)
		m_widget->setProperty("_t_instantiable", QVariant::fromValue(this));
	m_activated = false;
	return m_running;
}

bool TApp::Instantiable::activate()
{
	if (!m_running)
		if (!open())
			return false;
	if (!m_activated)
		m_activated = activateInstance(m_widget);
	return m_activated;
}

bool TApp::Instantiable::deactivate()
{
	if (!m_running || !m_activated)
		return true;
	const bool success = deactivateInstance(m_widget);
	m_activated = !success;
	return success;
}

bool TApp::Instantiable::close()
{
	if (!m_running)
		return true;
	const bool success = deleteInstance(m_widget);
	m_running = !success;
	if (success)
		m_widget = 0;
	return success;
}

//END TApp::Instantiable
//BEGIN TApp::TagaroGamePlugin

/*static*/ void TApp::TagaroGamePlugin::loadInto(QStandardItemModel* model)
{
	KService::List services = KServiceTypeTrader::self()->query(QLatin1String("Tagaro/Game"));
	foreach (KService::Ptr service, services)
		model->appendRow(new TApp::TagaroGamePlugin(service));
}

TApp::TagaroGamePlugin::TagaroGamePlugin(KService::Ptr service)
	: m_service(service)
{
	setData(service->name(), Qt::DisplayRole);
	setData(service->genericName(), Qt::ToolTipRole);
	setData(KIcon(service->icon()), Qt::DecorationRole);
}

TApp::InstantiatorFlags TApp::TagaroGamePlugin::flags() const
{
	return 0;
}

#include <KDebug>
bool TApp::TagaroGamePlugin::createInstance(QWidget*& widget)
{
	kDebug();
	QString error;
	widget = m_service->createInstance<QWidget>(0, QVariantList(), &error);
	if (!widget)
		KMessageBox::detailedError(0, i18n("The game \"%1\" could not be launched.", m_service->name()), error);
	//TODO: when there is a Plugin class, forward call to plugin (for late init)
	return (bool) widget;
}

bool TApp::TagaroGamePlugin::activateInstance(QWidget* widget)
{
	kDebug();
	//TODO: when there is a manager for these widget, tell it to select the widget
	//TODO: when there is a Plugin class, forward call to plugin
//	widget->show();
//	widget->activateWindow();
	return true;
}

bool TApp::TagaroGamePlugin::deactivateInstance(QWidget* widget)
{
	kDebug();
	//TODO: when there is a manager for these widget, tell it to select the widget
	//TODO: when there is a Plugin class, forward call to plugin
//	widget->hide();
	return true;
}

bool TApp::TagaroGamePlugin::deleteInstance(QWidget*& widget)
{
	kDebug();
	//TODO: when there is a Plugin class, forward call to plugin
	delete widget;
	return true;
}

//END TApp::TagaroGamePlugin
//BEGIN TApp::XdgAppPlugin

/*static*/ void TApp::XdgAppPlugin::loadInto(QStandardItemModel* model)
{
	KService::List services = KServiceTypeTrader::self()->query(
		QLatin1String("Application"),
		QLatin1String("'Game' in Categories and exist Exec and not ('tagaroshell' ~ Exec)")
	);
	foreach (KService::Ptr service, services)
		model->appendRow(new TApp::XdgAppPlugin(service));
}

TApp::XdgAppPlugin::XdgAppPlugin(KService::Ptr service)
	: m_service(service)
{
	setData(service->name(), Qt::DisplayRole);
	setData(service->genericName(), Qt::ToolTipRole);
	setData(KIcon(service->icon()), Qt::DecorationRole);
}

TApp::InstantiatorFlags TApp::XdgAppPlugin::flags() const
{
	return TApp::OutOfProcessInstance;
}

bool TApp::XdgAppPlugin::createInstance(QWidget*& widget)
{
	//TODO: Is it possible to get a QProcess instance out of KToolInvocation
	//      (or similar) to control the started application?
	widget = 0;
	KToolInvocation::startServiceByDesktopPath(m_service->entryPath());
	return true;
}

bool TApp::XdgAppPlugin::activateInstance(QWidget* widget)
{
	//dummy implementation (see todo item in createInstance())
	Q_UNUSED(widget)
	return true;
}

bool TApp::XdgAppPlugin::deactivateInstance(QWidget* widget)
{
	//does nothing by design (external processes cannot be paused)
	Q_UNUSED(widget)
	return true;
}

bool TApp::XdgAppPlugin::deleteInstance(QWidget*& widget)
{
	//dummy implementation (see todo item in createInstance())
	Q_UNUSED(widget)
	return true;
}

//END TApp::XdgAppPlugin
#ifdef TAGAROAPP_USE_GLUON
//BEGIN TApp::GluonGameFile

#include <gluon/core/gluon_global.h>
#include <gluon/engine/game.h>
#include <gluon/engine/gameproject.h>
#include <gluon/graphics/renderwidget.h>

/*static*/ void TApp::GluonGameFile::loadInto(QStandardItemModel* model)
{
	//find games
	QDir dataDir(GluonCore::Global::dataDirectory() + "/gluon/games");
	dataDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	dataDir.setSorting(QDir::Name);
	const QStringList gameDirs = dataDir.entryList(QStringList() << (QChar('*') + GluonEngine::projectSuffix));
	//read project file for each game
	foreach (const QString& gameDirName, gameDirs)
	{
		QDir gameDir(dataDir.absoluteFilePath(gameDirName));
		if (!gameDir.exists(GluonEngine::projectFilename))
			continue;
		model->appendRow(new TApp::GluonGameFile(gameDir.absoluteFilePath(GluonEngine::projectFilename)));
	}
}

TApp::GluonGameFile::GluonGameFile(const QString& projectFile)
	: m_project(new GluonEngine::GameProject)
{
	m_project->loadFromFile(projectFile);
	setData(m_project->name(), Qt::DisplayRole);
	setData(m_project->description(), Qt::ToolTipRole);
	setData(KIcon("gluon"), Qt::DecorationRole);
}

TApp::GluonGameFile::~GluonGameFile()
{
	m_project->deleteLater();
}

TApp::InstantiatorFlags TApp::GluonGameFile::flags() const
{
	return 0;
}

bool TApp::GluonGameFile::createInstance(QWidget*& widget)
{
	widget = new GluonGraphics::RenderWidget;
	widget->setWindowTitle(m_project->name());
	return true;
}

bool TApp::GluonGameFile::activateInstance(QWidget* widget)
{
	GluonEngine::Game* g = GluonEngine::Game::instance();
	if (g->gameProject() == m_project)
	{
		g->setPause(false);
		widget->show();
	}
	else
	{
		//changing from another game
		g->stopGame();
		g->setGameProject(m_project);
		g->setCurrentScene(m_project->entryPoint());
		widget->show();
		//cannot call that directly because it blocks until g->stopGame()
		QMetaObject::invokeMethod(g, "runGame", Qt::QueuedConnection);
	}
	return true;
}

bool TApp::GluonGameFile::deactivateInstance(QWidget* widget)
{
	GluonEngine::Game::instance()->setPause(true);
	widget->hide();
	return true;
}

bool TApp::GluonGameFile::deleteInstance(QWidget*& widget)
{
	GluonEngine::Game* g = GluonEngine::Game::instance();
	g->stopGame();
	//problem: widget is rendered upon from a different thread
	//detach it by hand from the parent and save it from deletion
	//by the Instantiable superclass until the next few frames
	//have been rendered
	widget->setParent(0);
	widget->hide();
	QTimer::singleShot(100, widget, SLOT(deleteLater()));
	widget = 0;
	return true;
}

//END TApp::GluonGameFile
#endif // TAGAROAPP_USE_GLUON

#include "instantiable.moc"
