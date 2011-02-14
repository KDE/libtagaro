/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>                     *
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

#ifndef TAGARO_MESSAGEOVERLAY_H
#define TAGARO_MESSAGEOVERLAY_H

#include <QtCore/QObject>

#include <libtagaro_export.h>

namespace Tagaro {

class Scene;

class TAGARO_EXPORT MessageOverlay : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
	Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
	public:
		///Creates a new Tagaro::MessageOverlay on the given @a scene. The
		///MessageOverlay is hidden until you explicitly show it.
		///
		///The @a scene takes ownership of the Tagaro::MessageOverlay instance.
		MessageOverlay(Tagaro::Scene* scene);
		///Destroys this Tagaro::MessageOverlay instance.
		virtual ~MessageOverlay();

		///@return the text which is shown on the overlay when it is visible.
		QString text() const;
		///Sets the text which is shown on the overlay when it is visible.
		///
		///The text is always shown centered on the sceneRect().
		void setText(const QString& text);
		///@return the timeout (in milliseconds) after which the message will
		///disappear automatically.
		int timeout() const;
		///Sets the timeout (in milliseconds) after which the message will
		///disappear automatically. The internal timer is reset by every call
		///to setVisible(). The default is -1, i.e. the message will not
		///disappear unless you call setVisible(false) or hide().
		void setTimeout(int timeout);
		///@return whether the overlay is visible on the scene. This need not
		///mean that the overlay is visible on screen, if the scene is not
		///exposed on any visible view.
		bool isVisible() const;
	public Q_SLOTS:
		///Sets whether the overlay is visible on the scene. Any call to this
		///method will reset the internal timer which hides the item after the
		///timeout().
		///@see isVisible()
		///
		///If multiple overlays are set to be visible on the same scene at the
		///same time, the scene will only render that overlay which was
		///rendered
		void setVisible(bool visible);
		///A synonym for setVisible(true).
		void show();
		///A synonym for setVisible(false).
		void hide();
	Q_SIGNALS:
		void textChanged(const QString& text);
		void visibleChanged(bool visible);
	protected:
		virtual void timerEvent(QTimerEvent* event);
	private:
		class Private;
		Private* const d;
};

} //namespace Tagaro

#endif // TAGARO_MESSAGEOVERLAY_H
