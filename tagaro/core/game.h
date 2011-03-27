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

#ifndef TAGARO_GAME_H
#define TAGARO_GAME_H

#include <QtCore/QVariant>
#include <QtGui/QWidget>
class KAboutData;
class KComponentData;

#include <libtagaro_export.h>

namespace Tagaro
{
	/**
	 * @class Tagaro::Game game.h <Tagaro/Game>
	 *
	 * The base class for Tagaro game plugins. This provides the KComponentData
	 * and integration with the surrounding window.
	 */
	class TAGARO_EXPORT Game : public QWidget
	{
		Q_OBJECT
		public:
			///Initialize KAboutData as usual. In the second and third
			///parameter, pass the @a parent and @a args which you get from the
			///plugin factory call.
			Game(const KAboutData& aboutData, QObject* parent, const QVariantList& args);
			virtual ~Game();

			///@return whether this Game instance is active
			///@see setActive()
			bool isActive() const;
			///@return whether this Game instance is paused
			///@see setPaused()
			bool isPaused() const;
			///@return the component data for this game instance
			///
			///Like for KParts, the componentData is guaranteed to be set as
			///KGlobal::activeComponent() while the Game instance isActive().
			const KComponentData& componentData() const;
		public Q_SLOTS:
			///Sets whether this Game instance is active. In each running shell,
			///only one Game instance may at most be active at the same time.
			///
			///A Game instance is guaranteed to be active while its interface is
			///visible to the user. While a Game is active, its componentData()
			///is set as KGlobal::activeComponent().
			///
			///Implement activeEvent() or listen to activeChanged() to actually
			///do something.
			void setActive(bool active);
			///Sets whether this Game instance is paused. This actually does
			///nothing. Implement pauseEvent() or listen to pausedChanged() to
			///actually do something.
			void setPaused(bool paused);
			///Sets the window title. (The game's name, according to aboutData,
			///is appended automatically.)
			void setCaption(const QString& caption);
			///Sets the window title.
			void setWindowTitle(const QString& windowTitle);
		Q_SIGNALS:
			///@see setActive(), activeEvent()
			void activeChanged(bool active);
			///@see setPaused(), pauseEvent()
			void pausedChanged(bool paused);
		protected:
			///Reimplement this to handle changes to isActive(). Alternatively,
			///use the activeChanged() signal. The default implementation does
			///nothing.
			virtual void activeEvent(bool active);
			///Reimplement this to handle changes to isPaused(). Alternatively,
			///use the pausedChanged() signal. The default implementation does
			///nothing.
			virtual void pauseEvent(bool paused);
		private:
			class Private;
			Private* const d;
	};
}

#endif // TAGARO_GAME_H
