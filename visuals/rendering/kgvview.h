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

#ifndef KGVVIEW_H
#define KGVVIEW_H

#include <QtGui/QGraphicsView>
#include <kgvrendererclient.h>

#include <libkgame_export.h>

/**
 * @class KgvView kgvview.h <KgvView>
 * @brief QGraphicsView with automatic viewport transform adjustments
 *
 * This QGraphicsView class assists you in managing the scene and viewport
 * coordinates. More exactly, it automatically keeps the QGraphicsScene's
 * sceneRect() in sync with the QGraphicsView's rect(), and suppresses (as far 
 * as possible) any manual changes to the scene rect.
 *
 * Additionally, this class can act as a KgvRendererClient to fetch a scene
 * background pixmap from a KgvRenderer.
 *
 * @note Scroll bars are turned off by default, because the scene rect is
 * automatically adjusted to fit into the view.
 *
 * @warning Because of technical limitations, you have to use the setScene()
 * method provided by KgvView instead of QGraphicsView::setScene(). You can do
 * the following to make sure that the right method gets called:
 * @code
 * view->KgvView::setScene(scene);
 * @endcode
 */
class KGAMEVISUALS_EXPORT KgvView : public QGraphicsView, public KgvRendererClient
{
	Q_OBJECT
	public:
		///Creates a new KgvView. When this constructor is used, the 
		///KgvRendererClient capabilities are disabled.
		KgvView(QWidget* parent = 0);
		///Creates a new KgvView, which fetches the scene background as a pixmap 
		///from the given @a renderer.
		KgvView(KgvRenderer* renderer, const QString& spriteKey, QWidget* parent = 0);
		///Destroys this KgvView instance.
		virtual ~KgvView();

		///Due to technical limitations, this method has to be used instead of
		///QGraphicsView::setScene() to set the view's scene.
		void setScene(QGraphicsScene* scene);
	Q_SIGNALS:
		///This signal is emitted when the view has been resized. The given
		///@a size is also the size of the scene rect (who's top-left corner is
		///at (0,0) in scene coordinates).
		void resized(const QSize& size);
	protected:
		virtual void resizeEvent(QResizeEvent* event);
		virtual void receivePixmap(const QPixmap& pixmap);
	private:
		class Private;
		Private* const d;

		Q_PRIVATE_SLOT(d, void _k_sceneRectChanged());
};

#endif // KGVVIEW_H
