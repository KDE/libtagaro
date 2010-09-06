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

#ifndef TAGARO_RENDERERCLIENT_H
#define TAGARO_RENDERERCLIENT_H

#include <QtGui/QPixmap>

#include "../objectpool.h" //for Tagaro::RendererPtr
#include <libtagaro_export.h>

namespace Tagaro {

class RendererClientPrivate;
class Renderer;
class RendererPrivate;

/**
 * @class Tagaro::RendererClient rendererclient.h <Tagaro/RendererClient>
 * @short An object that receives pixmaps from a Tagaro::Renderer.
 *
 * This class abstracts a sprite rendered by Tagaro::Renderer. Given a sprite
 * key, render size and possibly a frame index, it returns the QPixmap for this
 * sprite (frame) once it becomes available. See the Tagaro::Renderer class
 * documentation for details.
 *
 * Subclasses have to reimplement the receivePixmap() method.
 */
class TAGARO_EXPORT RendererClient
{
	public:
		///Creates a new client which receives pixmaps for the sprite with the
		///given @a spriteKey as provided by the given @a renderer.
		///You may give a null pointer to @a renderer, or an empty string to
		///@a spriteKey. In this case, no pixmap is fetched.
		RendererClient(Tagaro::RendererPtr renderer, const QString& spriteKey);
		virtual ~RendererClient();

		///@return the renderer used by this client
		Tagaro::Renderer* renderer() const;
		///Sets the renderer used by this client. Set to a null pointer to
		///disable pixmap fetching.
		void setRenderer(Tagaro::RendererPtr renderer);
		///@return the key of the sprite currently rendered by this client
		QString spriteKey() const;
		///Defines the key of the sprite which is rendered by this client. Set
		///to an empty string to disable pixmap fetching.
		void setSpriteKey(const QString& spriteKey);

		///@return the frame count, or 0 for non-animated sprites, or -1 if the
		///sprite does not exist at all
		///@see Tagaro::Renderer::frameCount()
		int frameCount() const;
		///@return the current frame number, or -1 for non-animated sprites
		int frame() const;
		///For animated sprites, render another frame. The given frame number is
		///normalized by taking the modulo of the frame count, so the following
		///code works fine:
		///@code
		///    class MyClient : public Tagaro::RendererClient { ... }
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
		///The default render size is very small (width = height = 3 pixels), so
		///that you notice when you forget to set this. ;-)
		void setRenderSize(const QSize& renderSize);
		///@return the rendered pixmap (or an invalid pixmap if no pixmap has
		///been rendered yet)
		QPixmap pixmap() const;
	protected:
		///This method is called when the Tagaro::Renderer has provided a new
		///pixmap for this client (esp. after theme changes and after calls to
		///setFrame(), setRenderSize() and setSpriteKey()).
		virtual void receivePixmap(const QPixmap& pixmap) = 0;
	private:
		friend class Tagaro::RendererClientPrivate;
		friend class Tagaro::Renderer;
		friend class Tagaro::RendererPrivate;
		Tagaro::RendererClientPrivate* const d;
};

} //namespace Tagaro

#endif // TAGARO_RENDERERCLIENT_H
