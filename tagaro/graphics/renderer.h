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

#ifndef TAGARO_RENDERER_H
#define TAGARO_RENDERER_H

#include "renderbackend.h"

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtGui/QPixmap>

#include <libtagaro_export.h>

namespace Tagaro {

class RendererPrivate;
class Sprite;
class Theme;
class ThemeProvider;

/**
 * @class Tagaro::Renderer renderer.h <Tagaro/Renderer>
 * @brief Cache-enabled rendering of SVG themes.
 *
 * Tagaro::Renderer is a light-weight rendering framework for the rendering of
 * SVG themes into pixmap caches.
 *
 * @section theming Automatic theme loading
 *
 * Tagaro::Renderer receives its themes from a Tagaro::ThemeProvider. The
 * functionality made available by Tagaro::ThemeProvider can be used to select a
 * theme. This selection will then automatically be picked up by all
 * Tagaro::Renderer instances using this theme provider.
 *
 * @section terminology Terminology
 *
 * A sprite is either a single pixmap ("non-animated sprites") or a sequence of
 * pixmaps which are shown consecutively to produce an animation ("animated
 * sprites"). Non-animated sprites correspond to a single element with the same
 * key in the SVG theme file. The element keys for the pixmaps of an animated
 * sprite are produced by appending the frameSuffix() to the sprite key.
 *
 * @section clients Access to the pixmaps
 *
 * Sprite pixmaps can be retrieved from Tagaro::Renderer in the main thread
 * using the synchronous Tagaro::Renderer::spritePixmap() method. However, it is
 * highly recommended to use the asynchronous interface provided by the
 * interface class Tagaro::RendererClient. A client corresponds to one pixmap
 * and registers itself with the corresponding Tagaro::Renderer instance to get
 * notified when a new pixmap is available.
 *
 * For QGraphicsView-based applications, the Tagaro::SpriteItem class provides a
 * QGraphicsPixmapItem which is a Tagaro::RendererClient and displays the pixmap
 * for a given sprite. If you need a QGraphicsObject, use
 * Tagaro::SpriteObjectItem.
 *
 * @section strategies Rendering strategy
 *
 * For each theme, Tagaro::Renderer keeps two caches around: an in-process cache
 * of QPixmaps, and a disk cache containing QImages (powered by KImageCache).
 * You therefore will not need to implement any caching for the pixmaps provided
 * by Tagaro::Renderer.
 *
 * When requests from a Tagaro::RendererClient cannot be served immediately
 * because the requested sprite is not in the caches, a rendering request is
 * sent to a worker thread.
 *
 * @section legacy Support for legacy themes
 *
 * When porting applications to Tagaro::Renderer, you probably have to support
 * the format of existing themes. Tagaro::Renderer provides the frameBaseIndex()
 * and frameSuffix() properties for this purpose. It is recommended not to
 * change these properties in new applications.
 *
 */
class TAGARO_EXPORT Renderer : public QObject
{
	Q_OBJECT
	public:
		///Constructs a new Tagaro::Renderer.
		///@param provider the theme provider to be used by this renderer
		///@param cacheSize the cache size per theme in megabytes (if not given,
		///a sane default is used)
		///@warning This constructor may only be called from the main thread.
		explicit Renderer(Tagaro::ThemeProvider* provider, const Tagaro::RenderBehavior& behavior = Tagaro::RenderBehavior());
		///Deletes this Tagaro::Renderer instance, and all clients using it.
		virtual ~Renderer();

		///@return the theme provider for this renderer
		Tagaro::ThemeProvider* themeProvider() const;
		///@return the backend used by this renderer
		Tagaro::RenderBackend* backend() const;

		///@return a Tagaro::Sprite instance for the given @a spriteKey
		Tagaro::Sprite* sprite(const QString& spriteKey) const;
	private:
		friend class Tagaro::RendererPrivate;
		Tagaro::RendererPrivate* const d;
};

} //namespace Tagaro

#endif // TAGARO_RENDERER_H
