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

#ifndef TAGARO_GRAPHICSSOURCECONFIG_H
#define TAGARO_GRAPHICSSOURCECONFIG_H

#include <QtCore/QString>

#include <libtagaro_export.h>

namespace Tagaro {

/**
 * @class Tagaro::GraphicsSourceConfig graphicssourceconfig.h <Tagaro/GraphicsSourceConfig>
 *
 * This class represents general configurable parameters for
 * Tagaro::GraphicsSource instances.
 */
class TAGARO_EXPORT GraphicsSourceConfig
{
	public:
		///Creates a new Tagaro::GraphicsSourceConfig instance with default values:
		///@li cacheSize() == 3 (megabytes)
		///@li frameBaseIndex() == 0
		///@li frameSuffix() = "_%1"
		GraphicsSourceConfig();
		///Copies the given config.
		GraphicsSourceConfig(const Tagaro::GraphicsSourceConfig& other);
		///Copies the given config.
		Tagaro::GraphicsSourceConfig& operator=(const Tagaro::GraphicsSourceConfig& other);
		///Destroys this Tagaro::GraphicsSourceConfig instance.
		~GraphicsSourceConfig();

		///@return the cache size in megabytes @see setCacheSize
		int cacheSize() const;
		///Sets the cache size in megabytes (default: 3 megabytes). This value
		///is only used by sources which can use caches. Set to 0 megabytes to
		///disable caching.
		///
		///@see Tagaro::CachedProxyGraphicsSource
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

} //namespace Tagaro

#endif // TAGARO_GRAPHICSSOURCECONFIG_H
