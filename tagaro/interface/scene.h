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

#ifndef TAGARO_SCENE_H
#define TAGARO_SCENE_H

#include <QtGui/QGraphicsScene>

#include <libtagaro_export.h>

namespace Tagaro {

class MessageOverlay;
class SpriteClient;
class Sprite;

/**
 * @class Tagaro::Scene scene.h <Tagaro/Scene>
 * @brief QGraphicsScene with automatic viewport transform adjustments
 *
 * This QGraphicsScene subclass provides integration with Tagaro and
 * miscellaneous convenience features:
 * @li It acts as a Tagaro::SpriteClient to fetch a scene background pixmap.
 * @li It can be used to keep the QGraphicsScene's sceneRect() in sync
 *     with the rect() of a QGraphicsView instance (the "main view").
 * @li It can display Tagaro::MessageOverlay instances.
 */
class TAGARO_EXPORT Scene : public QGraphicsScene
{
	Q_OBJECT
	public:
		///Creates a new Tagaro::Scene instance.
		explicit Scene(QObject* parent = 0);
		///@overload
		///Initializes the renderer client for the scene background brush with
		///the given @a sprite.
		explicit Scene(Tagaro::Sprite* backgroundSprite, QObject* parent = 0);
		///Destroys this Tagaro::Scene instance.
		virtual ~Scene();

		///@return the main view of this scene
		QGraphicsView* mainView() const;
		///Sets the main view of this scene (null by default). If set, the
		///scene's sceneRect() will always be set equal to the view's rect().
		///The scene will then suppress manual changes to the sceneRect() as
		///much as possible.
		///
		///This will also install this scene on the @a mainView. The behavior is
		///undefined if you set another scene on this view while it is this
		///scene's main view.
		void setMainView(QGraphicsView* mainView);

		///@return the renderer client for the scene's background brush
		///Use this to modify the background brush.
		///@warning Do not call setRenderSize() on this instance! The render
		///size is managed by the scene. Use setBackgroundBrushRenderSize()
		///instead.
		Tagaro::SpriteClient* backgroundBrushClient() const;
		///@return the background brush's render size
		///
		///If the render size is determined from the size of the sceneRect()
		///(the default), returns an invalid size. To determine the actual
		///render size, use backgroundBrushClient()->renderSize().
		QSize backgroundBrushRenderSize() const;
		///Sets the background brush's render size. If you set this to a valid
		///size, the background will be painted as a tiled pixmap of that size.
		///If an invalid size is set (the default), determine the actual render
		///size from the sceneRect().
		void setBackgroundBrushRenderSize(const QSize& size);
	protected:
		virtual bool eventFilter(QObject* watched, QEvent* event);
		virtual void drawForeground(QPainter* painter, const QRectF& rect);
	private:
		friend class MessageOverlay;
		class Private;
		Private* const d;
		Q_PRIVATE_SLOT(d, void _k_updateSceneRect(const QRectF&));
		Q_PRIVATE_SLOT(d, void _k_moDestroyed(QObject*));
		Q_PRIVATE_SLOT(d, void _k_moTextChanged(const QString&));
		Q_PRIVATE_SLOT(d, void _k_moVisibleChanged(bool));
};

} //namespace Tagaro

#endif // TAGARO_SCENE_H
