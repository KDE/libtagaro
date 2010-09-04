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

#ifndef TAGARO_RENDERERMODULE_H
#define TAGARO_RENDERERMODULE_H

class QPainter;
#include <QtCore/QRectF>
#include <QtCore/QString>

#include <libtagaro_export.h>

namespace Tagaro {

/**
 * @class Tagaro::RendererModule renderermodule.h <Tagaro/RendererModule>
 *
 * A renderer module is a backend which can be used by the Tagaro::Renderer
 * to render pixmaps. The RendererModule handles the loading of the
 * graphics file, while the Renderer schedules the actual rendering
 * operations.
 *
 * @warning The implementation of all virtual methods has to be thread-safe!
 */
class TAGARO_EXPORT RendererModule
{
	public:
		///Creates a new Tagaro::RendererModule.
		RendererModule();
		///Destroys this Tagaro::RendererModule instance.
		virtual ~RendererModule();

		///Creates a new Tagaro::RendererModule instance that renders the given
		///@a file. The type of renderer module is inferred from the file type.
		static Tagaro::RendererModule* fromFile(const QString& file);
		///If loading the graphics file is costly, you can delay the loading.
		///(However, any call to the other virtual methods of this class must
		///then result in loading the file.) If this method returns false, the
		///Tagaro::Renderer may try to use its disk cache. The default
		///implementation assumes that delayed loading is not supported, and
		///thus always returns true.
		virtual bool isLoaded() const;
		///@return true if the module has been initialized with a valid file
		virtual bool isValid() const = 0;

		///@return the bounding rectangle of this @a element
		virtual QRectF boundsOnElement(const QString& element) const = 0;
		///@return whether this @a element exists in the loaded file
		///@note This method returns true only for renderable elements. Any
		///      internal named elements which cannot be rendered will be
		///      invisible to this function.
		virtual bool elementExists(const QString& element) const = 0;
		///Renders the given @a element using the given @a painter on the
		///specified @a bounds.
		virtual void render(QPainter* painter, const QString& element, const QRectF& bounds = QRectF()) = 0;
	private:
		class Private;
		Private* const d;
};

/**
 * @class Tagaro::QSvgRendererModule renderermodule.h <Tagaro/QSvgRendererModule>
 *
 * This RendererModule makes QSvgRenderer available to the Tagaro rendering
 * classes. The key difference between QSvgRenderer and
 * Tagaro::QSvgRendererModule is that the latter provides thread-safe
 * rendering.
 */
class TAGARO_EXPORT QSvgRendererModule : public Tagaro::RendererModule
{
	public:
		QSvgRendererModule(const QString& file);
		virtual ~QSvgRendererModule();

		virtual bool isValid() const;
		virtual QRectF boundsOnElement(const QString& element) const;
		virtual bool elementExists(const QString& element) const;
		virtual void render(QPainter* painter, const QString& element, const QRectF& bounds = QRectF());
	private:
		class Private;
		Private* const d;
};

} //namespace Tagaro

#endif // TAGARO_RENDERERMODULE_H
