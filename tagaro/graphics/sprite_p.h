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

#ifndef TAGARO_SPRITE_P_H
#define TAGARO_SPRITE_P_H

#include "sprite.h"
#include "rendererclient.h"

#include <QtCore/QHash>

namespace Tagaro {

class RenderBackend;
class RendererClient;

class SpriteFetcher : public QObject
{
	Q_OBJECT
	public:
		SpriteFetcher(const QSize& size, Tagaro::Sprite::Private* d) : d(d), m_size(size) {}

		void addClient(Tagaro::RendererClient* client);
		void removeClient(Tagaro::RendererClient* client);
		void updateClient(Tagaro::RendererClient* client);
		void updateAllClients();
	public Q_SLOTS:
		//If called with a null @a image, looks in the cache for the given
		//pixmap, or renders it synchronously on the given backend. This
		//interface is used for synchronous pixmap fetching.
		//
		//If @a image is not null, this image is converted and placed in the
		//pixmap cache (if necessary). This interface is used for images
		//returned from rendering threads.
		QPixmap cachePixmap(int frame, const QImage& image);
	private:
		void startJob(int frame);

		Tagaro::Sprite::Private* const d;
		QSize m_size;

		QHash<int, QPixmap> m_pixmapCache;
		QList<Tagaro::RendererClient*> m_clients;
};

struct Sprite::Private
{
	public:
		void setBackend(const Tagaro::RenderBackend* backend, const QString& element);

		void addClient(Tagaro::RendererClient* client);
		void removeClient(Tagaro::RendererClient* client);
		Tagaro::SpriteFetcher* fetcher(const QSize& size);
	private:
		friend class Tagaro::Sprite;
		friend class Tagaro::SpriteFetcher;
		Private();

		const Tagaro::RenderBackend* m_backend;
		QString m_element;
	
		QHash<QSize, SpriteFetcher*> m_fetchers;
		QList<Tagaro::RendererClient*> m_clients;
};

struct RendererClient::Private
{
	public:
		Private(Tagaro::Sprite* sprite, Tagaro::RendererClient* q);
		void receivePixmap(const QPixmap& pixmap);
	private:
		friend class Tagaro::RendererClient;
		Tagaro::RendererClient* q;
		Tagaro::Sprite* m_sprite;
		QSize m_size;
		Tagaro::SpriteFetcher* m_fetcher;
		int m_frame;
		QPixmap m_pixmap;
};

} //namespace Tagaro

inline uint qHash(const QSize& size)
{
	return qHash(qMakePair(size.width(), size.height()));
}

#endif // TAGARO_SPRITE_P_H
