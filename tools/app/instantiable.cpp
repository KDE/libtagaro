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
#include <KDE/KAboutData>
#include <KDE/KIcon>
#include <KDE/KMainWindow>
#include <KDE/KMessageBox>
#include <KDE/KServiceTypeTrader>
#include <KDE/KToolInvocation>
#include <Tagaro/Game>

KMainWindow* TApp::mainWindow;

//BEGIN TApp::Instantiable

/*static*/ TApp::Instantiable* TApp::Instantiable::forWidget(QWidget* widget)
{
	return widget->property("_t_instantiable").value<TApp::Instantiable*>();
}

Tagaro::Game* TApp::Instantiable::createInstance()
{
	//prepare plugin args for shell handshake
	QVariantList gameArgs;
	gameArgs << QVariant::fromValue<int>(1); //handshake protocol version
	gameArgs << QVariant::fromValue<QObject*>(TApp::mainWindow);
	gameArgs << QVariant::fromValue<QIcon>(data(Qt::DecorationRole).value<QIcon>());
	//create instance
	Tagaro::Game* game = createInstance(TApp::mainWindow, gameArgs);
	if (game)
	{
		game->setProperty("_t_instantiable", QVariant::fromValue(this));
		connect(this, SIGNAL(destroyed(QObject*)), game, SLOT(deleteLater()));
	}
	return game;
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

Tagaro::Game* TApp::TagaroGamePlugin::createInstance(QObject* parent, const QVariantList& args)
{
	QString error;
	Tagaro::Game* game = m_service->createInstance<Tagaro::Game>(parent, args, &error);
	if (!game)
		KMessageBox::detailedError(0, i18n("The game \"%1\" could not be launched.", m_service->name()), error);
	return game;
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

Tagaro::Game* TApp::XdgAppPlugin::createInstance(QObject* parent, const QVariantList& args)
{
	Q_UNUSED(parent) Q_UNUSED(args)
	//TODO: Is it possible to get a QProcess instance out of KToolInvocation
	//TODO: (or similar) to control the started application? (The QProcess
	//TODO: would then probably need to be wrapped in a custom Tagaro::Game.)
	KToolInvocation::startServiceByDesktopPath(m_service->entryPath());
	return 0;
}

//END TApp::XdgAppPlugin
#ifdef TAGAROAPP_USE_GLUON
//BEGIN TApp::GluonGameFile

#include <QtGui/QVBoxLayout>
#include <gluon/core/gluon_global.h>
#include <gluon/engine/game.h>
#include <gluon/engine/gameproject.h>
#include <gluon/input/inputmanager.h>
#include <gluon/graphics/renderwidget.h>

class GluonGame : public Tagaro::Game
{
	public:
		GluonGame(GluonEngine::GameProject* project, QObject* parent, const QVariantList& args);
		virtual ~GluonGame();
	protected:
		virtual void activeEvent(bool active);
	private:
		GluonEngine::GameProject* m_project;
		GluonGraphics::RenderWidget* m_renderer;
};

static KAboutData gluonAbout(GluonEngine::GameProject* project)
{
	return KAboutData(
		"gluon", 0, ki18n(qPrintable(project->name())),
		"0.0", ki18n(qPrintable(project->description()))
	);
}

GluonGame::GluonGame(GluonEngine::GameProject* project, QObject* parent, const QVariantList& args)
	: Tagaro::Game(gluonAbout(project), parent, args)
	, m_project(project)
	, m_renderer(new GluonGraphics::RenderWidget)
{
	connect(this, SIGNAL(pausedChanged(bool)), GluonEngine::Game::instance(), SLOT(setPause(bool)));
	//setup game interface
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->addWidget(m_renderer);
}

GluonGame::~GluonGame()
{
	GluonEngine::Game::instance()->stopGame();
	//problem: m_renderer is rendered upon from a different thread
	//detach it by hand from the parent and save it from deletion
	//by the Instantiable superclass until the next few frames
	//have been rendered
	m_renderer->setParent(0);
	m_renderer->hide();
	QTimer::singleShot(100, m_renderer, SLOT(deleteLater()));
	m_project->deleteLater();
}

void GluonGame::activeEvent(bool active)
{
	GluonEngine::Game* g = GluonEngine::Game::instance();
	if (active)
	{
		if (g->gameProject() != m_project)
		{
			//changing from another game
			g->stopGame();
			g->setGameProject(m_project);
			g->setCurrentScene(m_project->entryPoint());
			//cannot call that directly because it blocks until g->stopGame()
			QMetaObject::invokeMethod(g, "runGame", Qt::QueuedConnection);
		}
		connect(g, SIGNAL(painted(int)), m_renderer, SLOT(updateGL()));
		GluonInput::InputManager::instance()->setFilteredObject(m_renderer);
		m_renderer->setFocus();
	}
	else
	{
		disconnect(g, 0, m_renderer, 0);
	}
}

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
	: m_projectFile(projectFile)
{
	GluonEngine::GameProject project;
	project.loadFromFile(projectFile);
	setData(project.name(), Qt::DisplayRole);
	setData(project.description(), Qt::ToolTipRole);
	setData(KIcon("gluon"), Qt::DecorationRole);
}

TApp::InstantiatorFlags TApp::GluonGameFile::flags() const
{
	return 0;
}

Tagaro::Game* TApp::GluonGameFile::createInstance(QObject* parent, const QVariantList& args)
{
	GluonCore::GluonObjectFactory::instance()->loadPlugins();
	GluonEngine::GameProject* project = new GluonEngine::GameProject;
	project->loadFromFile(m_projectFile);
	return new GluonGame(project, parent, args);
}

//END TApp::GluonGameFile
#endif // TAGAROAPP_USE_GLUON
