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

#ifndef TAGARO_SPRITECLIENT_H
#define TAGARO_SPRITECLIENT_H

#include <QtGui/QPixmap>

#include <libtagaro_export.h>

namespace Tagaro {

class Sprite;
class SpriteFetcher;

/**
 * @class Tagaro::SpriteClient spriteclient.h <Tagaro/SpriteClient>
 * @short An object that receives pixmaps from a Tagaro::Sprite.
 *
 * This class is the asynchronous rendering interface for Tagaro::Sprite (in
 * contrast to the synchronous Tagaro::Sprite::pixmap() method). Given a render
 * size and possibly a frame index, it returns the QPixmap for this sprite
 * (frame) once it becomes available. See @ref tagarographics for details.
 *
 * Subclasses have to reimplement the receivePixmap() method.
 */
class TAGARO_EXPORT SpriteClient
{
	public:
		///Creates a new client which receives pixmaps for the given @a sprite.
		///You may give a null pointer to @a sprite to disable pixmap fetching.
		///The pixmap() will then be invalid.
		SpriteClient(Tagaro::Sprite* sprite);
		///Destroys this Tagaro::SpriteClient.
		virtual ~SpriteClient();

		///@return the sprite rendered by this client
		Tagaro::Sprite* sprite() const;
		///Sets the sprite rendered by this client. Set to a null pointer to
		///disable pixmap fetching.
		void setSprite(Tagaro::Sprite* sprite);

		///@return the current frame number, or -1 for non-animated sprites
		int frame() const;
		///For animated sprites, render another frame. The given frame number is
		///normalized by taking the modulo of the frame count, so the following
		///code works fine:
		///@code
		///    class MyClient : public Tagaro::SpriteClient { ... }
		///    MyClient client;
		///    client.setFrame(client.frame() + 1); //cycle to next frame
		///    client.setFrame(KRandom::random());  //choose a random frame
		///@endcode
		void setFrame(int frame);

		///@return the size of the pixmap requested from Tagaro::Renderer
		QSize renderSize() const;
		///Defines the size of the pixmap that will be requested from
		///Tagaro::Renderer. For pixmaps rendered on the screen, you usually
		///want to set this size such that the pixmap does not have to be scaled
		///when it is rendered onto your primary view (for speed reasons).
		///
		///The default render size is empty, so that pixmap rendering is
		///disabled (i.e. pixmap() is invalid).
		void setRenderSize(const QSize& renderSize);
		///@return the rendered pixmap (or an invalid pixmap if no pixmap has
		///been rendered yet)
		QPixmap pixmap() const;
	protected:
		///This method is called when a new pixmap has been rendered for this
		///client (esp. after theme changes and calls to the client's setters).
		virtual void receivePixmap(const QPixmap& pixmap) = 0;
	private:
		friend class Tagaro::SpriteFetcher;
		class Private;
		Private* const d;
};

} //namespace Tagaro

#endif // TAGARO_SPRITECLIENT_H
