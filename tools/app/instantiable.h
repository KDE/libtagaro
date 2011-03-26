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
#include <KDE/KService>

namespace TApp
{
	enum InstantiatorFlag
	{
		OutOfProcessInstance = 0x1
	};
	Q_DECLARE_FLAGS(InstantiatorFlags, InstantiatorFlag)

	///Represents a game which can be launched.
	class Instantiable : public QObject, public QStandardItem
	{
		Q_OBJECT
		public:
			Instantiable();
			~Instantiable();

			///Returns information how this Instantiable behaves.
			virtual TApp::InstantiatorFlags flags() const = 0;
		public Q_SLOTS:
			bool activate();
			bool close();
		protected:
			///Launches the game. Shall return true on success. For games that
			///run in the same process, a pointer to the created widget must be
			///given.
			virtual bool createInstance(QWidget*& widget) = 0;
			///Activates the game. Shall return true on success. For games that
			///run in the same process, a pointer to the created widget must be
			///given.
			virtual bool activateInstance(QWidget* widget) = 0;
			///Shuts down the game. Shall return true on success. For games
			///that run in the same process, the top-level widget is given
			///(which was returned from createInstance()).
			virtual bool deleteInstance(QWidget* widget) = 0;
		private:
			bool m_running;
			QWidget* m_widget;
	};

	class TagaroGamePlugin : public TApp::Instantiable
	{
		public:
			TagaroGamePlugin(KService::Ptr service);
			///Loads available Tagaro/Game plugins into the given @a model.
			static void loadInto(QStandardItemModel* model);

			virtual TApp::InstantiatorFlags flags() const;
		protected:
			virtual bool createInstance(QWidget*& widget);
			virtual bool activateInstance(QWidget* widget);
			virtual bool deleteInstance(QWidget* widget);
		private:
			KService::Ptr m_service;
	};
}

Q_DECLARE_OPERATORS_FOR_FLAGS(TApp::InstantiatorFlags)

#endif // TAPP_INSTANTIATOR_H
