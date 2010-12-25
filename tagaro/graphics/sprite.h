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

#ifndef TAGARO_SPRITE_H
#define TAGARO_SPRITE_H

#include <QtGui/QPixmap>
#include <libtagaro_export.h>

namespace Tagaro {

class RendererClient;
class SpriteFetcher;
class ThemeProvider;

/**
 * @class Tagaro::Sprite sprite.h <Tagaro/Sprite>
 *
 * Sprites are basic graphical elements of an application. They need not be
 * graphical primitives like lines or circles, but they are primitive graphical
 * elements for the application, like the single cards in a card game.
 *
 * A sprite is either a single pixmap ("non-animated sprites") or a sequence of
 * pixmaps which are shown consecutively to produce an animation ("animated
 * sprites"). Non-animated sprites correspond to a single element with the same
 * key in the theme file. The element keys for the pixmaps of an animated
 * sprite are produced by appending the renderer()'s frameSuffix().
 *
 * Tagaro::Sprite instances can be obtained from a Tagaro::ThemeProvider.
 */
class TAGARO_EXPORT Sprite
{
	public:
		///@return the bounding rectangle of this sprite
		///
		///The meaning of the bounding rectangle is determined by the actual
		///renderer module which renders this sprite. For example, SVG renderer
		///modules will return the bounding rectangle of the SVG element
		///corresponding to this sprite.
		QRectF bounds(int frame = -1) const;
		///@return whether this sprite exists
		///
		///Same as frameCount() >= 0.
		bool exists() const;
		///@return the count of frames available for the sprite
		///
		///If this sprite is not animated (i.e. there are no theme elements for
		///any frames), this method returns 0. If the sprite does not exist at
		///all, -1 is returned.
		///
		///If the sprite is animated, the method counts frames starting at zero
		///(unless you change the frameBaseIndex()), and returns the number of
		///frames for which corresponding elements exist in the theme file.
		///
		///For example, if the theme contains the elements "foo_0", "foo_1" and
		///"foo_3", frameCount("foo") returns 2 for the default frame suffix.
		///(The element "foo_3" is ignored because "foo_2" is missing.)
		int frameCount() const;
		///@return the name of this sprite
		QString key() const;

		///@return a rendered pixmap
		///@param size the size of the resulting pixmap
		///@param frame the number of the frame which you want (set to -1 or
		///             omit for non-animated frames)
		///@warning Call only from GUI thread!
		///
		///The pixmap will always be rendered synchronously, i.e. in the same
		///thread.
		QPixmap pixmap(const QSize& size, int frame = -1) const;
	private:
		class Private;
		Private* const d;

		friend class Tagaro::RendererClient;
		friend class Tagaro::SpriteFetcher;
		friend class Tagaro::ThemeProvider;
		Sprite();
		~Sprite();
};

} //namespace Tagaro

#endif // TAGARO_SPRITE_H
