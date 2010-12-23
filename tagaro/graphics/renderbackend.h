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

#ifndef TAGARO_RENDERBACKEND_H
#define TAGARO_RENDERBACKEND_H

#include <QtGui/QImage>

#include <libtagaro_export.h>

namespace Tagaro {

/**
 * @class Tagaro::RenderBehavior renderbackend.h <Tagaro/RenderBehavior>
 *
 * This class represents general configurable parameters for
 * Tagaro::RenderBackends.
 */
class TAGARO_EXPORT RenderBehavior
{
	public:
		///Creates a new Tagaro::RenderBehavior instance with default values:
		///@li cacheSize() == 3 (megabytes)
		///@li frameBaseIndex() == 0
		///@li frameSuffix() = "_%1"
		RenderBehavior();
		///Copies the given behavior.
		RenderBehavior(const Tagaro::RenderBehavior& other);
		///Copies the given behavior.
		Tagaro::RenderBehavior& operator=(const Tagaro::RenderBehavior& other);
		///Destroys this Tagaro::RenderBehavior instance.
		~RenderBehavior();

		///@return the cache size in megabytes @see setCacheSize
		int cacheSize() const;
		///Sets the cache size in megabytes (default: 3 megabytes). This value
		///is only used by backends which can use caches. Set to 0 megabytes to
		///disable caching.
		///
		///@see Tagaro::CachedProxyRenderBackend
		void setCacheSize(int cacheSize);
		///@return the frame base index @see setFrameBaseIndex()
		int frameBaseIndex() const;
		///Sets the frame base index, i.e. the lowest frame index. Usually,
		///frame numbering starts at zero, so the frame base index is zero.
		///
		///For example, if you set the frame base index to 42, and use the
		///default frame suffix, the 3 frames of an animated sprite "foo" are
		///provided by the SVG elements "foo_42", "foo_43" and "foo_44".
		///
		///It is recommended not to alter the frame base index unless you need
		///to support legacy themes.
		void setFrameBaseIndex(int frameBaseIndex);
		///@return the frame suffix @see setFrameSuffix()
		QString frameSuffix() const;
		///Sets the frame suffix. This suffix will be added to a sprite key
		///to create the corresponding SVG element key, after any occurrence of
		///"%1" in the suffix has been replaced by the frame number.
		///
		///For example, if the frame suffix is set to "_%1" (the default), the
		///SVG element key for the frame no. 23 of the sprite "foo" is "foo_23".
		///@note Frame numbering starts at zero unless you setFrameBaseIndex().
		///
		///It is recommended not to alter the frame suffix unless you need to
		///support legacy themes. Giving a @a suffix which does not include
		///the pattern "%1" will reset to the default suffix "_%1".
		void setFrameSuffix(const QString& suffix);
	private:
		class Private;
		Private* const d;
};

/**
 * @class Tagaro::RenderBackend renderbackend.h <Tagaro/RenderBackend>
 *
 * A RenderBackend represents a source of graphical elements, e.g. a graphics
 * file. (When we talk about "elements", we only mean renderable elements which
 * have been assigned a QString key.)
 *
 * @see Tagaro::CachedProxyRenderBackend for sources with complex internal structure
 *
 * Subclasses which load graphical elements from external resources (e.g.
 * files) shall not load these resources in the constructor, but in the load()
 * method. This allows the CachedProxyRenderBackend to skip the loading when
 * the graphical elements or metadata are available from the cache.
 */
class TAGARO_EXPORT RenderBackend
{
	Q_DISABLE_COPY(RenderBackend)
	public:
		///Creates a new Tagaro::RenderBackend with the given @a behavior.
		///
		///See identifier() for the meaning of the @a identifier.
		RenderBackend(const QString& identifier, const Tagaro::RenderBehavior& behavior);
		///Destroys this Tagaro::RenderBackend.
		virtual ~RenderBackend();

		///@return this backend's rendering behavior
		const Tagaro::RenderBehavior& behavior() const;
		///@return the backend identifier
		///
		///The identifier is used by CachedProxyRenderBackend to determine the
		///cache name. It must therefore be unique among all active non-proxy
		///RenderBackends, but it must also stay the same for each backend over
		///multiple application runs (in order to reuse existing caches).
		QString identifier() const;

		///Load graphical elements from external resources (if there are any).
		///It is guaranteed that this method will be called before any call to
		///elementBounds(), elementExists(), elementImage(), frameCount() and
		///frameElementKey() with @a useFrameCount == true.
		///
		///The default implementation assumes that there are no external
		///resources, and does nothing (returning true).
		virtual bool load();
		///If graphical elements are loaded from external resources, return
		///the UNIX timestamp of when these resources were modified last. This
		///is used by the CachedProxyRenderBackend to invalidate its caches
		///when the external resources are updated.
		///
		///The default implementation returns 0, which is interpreted as the
		///absence of external resources.
		virtual uint lastModified() const;
		///@return the bounding rectangle of this @a element
		///
		///The default implementation returns QRectF(). Reimplement this method
		///only if it is meaningful, i.e. if your rendering backend places the
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
		virtual QImage elementImage(const QString& element, const QSize& size, bool timeConstraint) const = 0;
		///@return the frame count of the given @a element
		///
		///The semantics are similar to Tagaro::Sprite::frameCount:
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
	private:
		class Private;
		Private* const d;
};

/**
 * @class Tagaro::CachedProxyRenderBackend renderbackend.h <Tagaro/CachedProxyRenderBackend>
 *
 * Provides disk caching for backends with complex graphics sources.
 * In-process caches are provided for element metadata, but not for images.
 */
class TAGARO_EXPORT CachedProxyRenderBackend : public Tagaro::RenderBackend
{
	public:
		///Creates a new Tagaro::CachedProxyRenderBackend. The given @a backend
		///will be used to actually do the rendering work. The proxy takes
		///ownership of the given @a backend.
		explicit CachedProxyRenderBackend(Tagaro::RenderBackend* backend);
		///Destroys this Tagaro::CachedProxyRenderBackend, and the backend
		///behind it.
		virtual ~CachedProxyRenderBackend();

		virtual bool load();
		virtual QRectF elementBounds(const QString& element) const;
		virtual bool elementExists(const QString& element) const;
		virtual QImage elementImage(const QString& element, const QSize& size, bool timeConstraint) const;
		virtual int frameCount(const QString& element) const;
	private:
		class Private;
		Private* const d;
};

} //namespace Tagaro

#endif // TAGARO_RENDERBACKEND_H
