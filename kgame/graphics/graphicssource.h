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

#ifndef KGAME_GRAPHICSSOURCE_H
#define KGAME_GRAPHICSSOURCE_H

#include <QtGui/QImage>

#include <libkgame_export.h>

namespace KGame {

class GraphicsSourceConfig;

/**
 * @class KGame::GraphicsSource graphicssource.h <KGame/GraphicsSource>
 *
 * A GraphicsSource represents a source of graphical elements, e.g. a graphics
 * file. (When we talk about "elements", we only mean renderable elements which
 * have been assigned a QString key.)
 *
 * @see KGame::CachedProxyGraphicsSource for sources with complex internal structure
 *
 * Subclasses which load graphical elements from external resources (e.g.
 * files) shall not load these resources in the constructor, but in the load()
 * method. This allows the CachedProxyGraphicsSource to skip the loading when
 * the graphical elements or metadata are available from the cache.
 */
class KGAME_EXPORT GraphicsSource
{
	Q_DISABLE_COPY(GraphicsSource)
	public:
		///Creates a new KGame::GraphicsSource with the given @a config.
		///
		///See identifier() for the meaning of the @a identifier.
		GraphicsSource(const QString& identifier, const KGame::GraphicsSourceConfig& config);
		///Destroys this KGame::GraphicsSource.
		virtual ~GraphicsSource();

		///@return this source's config
		const KGame::GraphicsSourceConfig& config() const;
		///@return the source identifier
		///
		///The identifier is used by CachedProxyGraphicsSource to determine the
		///cache name. It must therefore be unique among all active non-proxy
		///GraphicsSources, but it must also stay the same for each source over
		///multiple application runs (in order to reuse existing caches).
		QString identifier() const;
		///@return whether the source's graphical sources could be loaded
		///successfully
		bool isValid() const;
		///The theme can use this hook to supply source-specific configuration
		///values. The default implementation does nothing, because the meaning
		///of the @a configuration depends on the type of source.
		virtual void addConfiguration(const QMap<QString, QString>& configuration);

		///If graphical elements are loaded from external resources, return
		///the UNIX timestamp of when these resources were modified last. This
		///is used by the CachedProxyGraphicsSource to invalidate its caches
		///when the external resources are updated.
		///
		///The default implementation returns 0, which is interpreted as the
		///absence of external resources.
		virtual uint lastModified() const;
		///@return the bounding rectangle of this @a element
		///
		///The default implementation returns QRectF(). Reimplement this method
		///only if it is meaningful, i.e. if your rendering source places the
		///single elements on a canvas.
		virtual QRectF elementBounds(const QString& element) const;
		///@return whether this @a element exists in the loaded file
		///
		///This method shall return true only for renderable elements.
		virtual bool elementExists(const QString& element) const = 0;
		///@return the given @a element, rendered in the given @a size
		///
		///The names of frame elements (@a frame >= 0) shall be formed with
		///frameElementKey().
		///
		///The @a processingInstruction is additional data specified by the
		///programmer in the sprite client. The format of accepted processing
		///instructions is defined by the implementing GraphicsSource subclass.
		///
		///The "@" sign may, because of Tagaro-internal use, not occur in both
		///element keys and processing instructions.
		///
		///If @a timeConstraint is true, do not do any time-expensive
		///operations, but return a QImage() instead to indicate that the
		///operation should be continued in a separate worker thread.
		///
		///@warning This method must (unless noted above) always return an
		///QImage instance with the right @a size. Especially, if the element
		///does not exist, do not return an invalid (i.e. default-constructed)
		///QImage instance, but a fully transparent image with the correct
		///@a size.
		///
		///@warning This method must be thread-safe when @a timeConstraint is
		///false.
		virtual QImage elementImage(const QString& element, const QSize& size, const QString& processingInstruction, bool timeConstraint) const = 0;
		///@return the frame count of the given @a element
		///
		///The semantics are similar to KGame::Sprite::frameCount:
		///@li Return -1 if the element does not exist at all.
		///@li Return 0 if the element is not animated, i.e. the element
		///    exists, but no frame elements exist for it.
		///@li Return the number of available frame elements if there are such.
		///
		///The names of frame elements shall be formed with frameElementKey().
		///
		///The default implementation uses elementExists() calls to determine
		///the frame count. Reimplement only if there's a more efficient method.
		virtual int frameCount(const QString& element) const;
		///@return the element key of a frame of an animated element
		///
		///This function may use frameCount() for advanced frame number
		///normalizations. If called from frameCount(), the last argument must
		///therefore be set to true to avoid infinite recursion.
		QString frameElementKey(const QString& element, int frame, bool useFrameCount = true) const;
	protected:
		///Load graphical elements from external resources (if there are any).
		///It is guaranteed that this method will be called before any call to
		///elementBounds(), elementExists(), elementImage(), frameCount() and
		///frameElementKey() with @a useFrameCount == true.
		///
		///The default implementation assumes that there are no external
		///resources, and does nothing (returning true).
		virtual bool load();
	private:
		class Private;
		Private* const d;
};

/**
 * @class KGame::CachedProxyGraphicsSource graphicssource.h <KGame/CachedProxyGraphicsSource>
 *
 * Provides disk caching for sources with complex graphics sources.
 * In-process caches are provided for element metadata, but not for images.
 */
class KGAME_EXPORT CachedProxyGraphicsSource : public KGame::GraphicsSource
{
	public:
		///Creates a new KGame::CachedProxyGraphicsSource. The given @a source
		///will be used to actually do the rendering work. The proxy takes
		///ownership of the given @a source.
		explicit CachedProxyGraphicsSource(KGame::GraphicsSource* source);
		///Destroys this KGame::CachedProxyGraphicsSource, and the source
		///behind it.
		virtual ~CachedProxyGraphicsSource();

		virtual QRectF elementBounds(const QString& element) const;
		virtual bool elementExists(const QString& element) const;
		virtual QImage elementImage(const QString& element, const QSize& size, const QString& processingInstruction, bool timeConstraint) const;
		virtual int frameCount(const QString& element) const;
	protected:
		virtual bool load();
	private:
		class Private;
		Private* const d;
};

} //namespace KGame

#endif // KGAME_GRAPHICSSOURCE_H
