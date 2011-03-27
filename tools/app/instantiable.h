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

#ifndef TAPP_INSTANTIATOR_H
#define TAPP_INSTANTIATOR_H

#include <QtGui/QStandardItemModel>
class KMainWindow;
#include <KDE/KService>
namespace GluonEngine
{
	class GameProject;
}
namespace Tagaro
{
	class Game;
}

namespace TApp
{
	enum InstantiatorFlag
	{
		OutOfProcessInstance = 0x1
	};
	Q_DECLARE_FLAGS(InstantiatorFlags, InstantiatorFlag)

	extern KMainWindow* mainWindow;

	///Represents a game which can be launched.
	class Instantiable : public QObject, public QStandardItem
	{
		public:
			///This maps a widget returned from the widget() function
			///back to the TApp::Instantiable instance. Returns 0 if
			///the widget was not generated by TApp::Instantiable.
			static TApp::Instantiable* forWidget(QWidget* widget);

			///Returns information how this Instantiable behaves.
			virtual TApp::InstantiatorFlags flags() const = 0;
			///Returns a new instance of this game.
			Tagaro::Game* createInstance();
		protected:
			///Launches the game. Shall return true on success. For games that
			///run in the same process, a pointer to the created widget must be
			///given.
			virtual Tagaro::Game* createInstance(QObject* parent, const QVariantList& args) = 0;
	};

	class TagaroGamePlugin : public TApp::Instantiable
	{
		public:
			TagaroGamePlugin(KService::Ptr service);
			///Loads available Tagaro/Game plugins into the given @a model.
			static void loadInto(QStandardItemModel* model);

			virtual TApp::InstantiatorFlags flags() const;
		protected:
			virtual Tagaro::Game* createInstance(QObject* parent, const QVariantList& args);
		private:
			KService::Ptr m_service;
	};

	class XdgAppPlugin : public TApp::Instantiable
	{
		public:
			XdgAppPlugin(KService::Ptr service);
			///Loads available XDG-compliant applications into the given @a model.
			static void loadInto(QStandardItemModel* model);

			virtual TApp::InstantiatorFlags flags() const;
		protected:
			virtual Tagaro::Game* createInstance(QObject* parent, const QVariantList& args);
		private:
			KService::Ptr m_service;
	};

#ifdef TAGAROAPP_USE_GLUON
	class GluonGameFile : public TApp::Instantiable
	{
		public:
			GluonGameFile(const QString& projectFile);
			///Loads available Gluon games into the given @a model.
			static void loadInto(QStandardItemModel* model);

			virtual TApp::InstantiatorFlags flags() const;
		protected:
			virtual Tagaro::Game* createInstance(QObject* parent, const QVariantList& args);
		private:
			QString m_projectFile;
	};
#endif // TAGAROAPP_USE_GLUON
}

Q_DECLARE_OPERATORS_FOR_FLAGS(TApp::InstantiatorFlags)
Q_DECLARE_METATYPE(TApp::Instantiable*)

#endif // TAPP_INSTANTIATOR_H
