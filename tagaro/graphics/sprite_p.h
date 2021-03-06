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
#include "spriteclient.h"

#include <QtCore/QHash>

namespace Tagaro {

class GraphicsSource;
class SpriteClient;

class SpriteFetcher : public QObject
{
	Q_OBJECT
	public:
		SpriteFetcher(const QSize& size, const QString& processingInstruction, Tagaro::Sprite::Private* d) : d(d), m_size(size), m_processingInstruction(processingInstruction) {}

		void addClient(Tagaro::SpriteClient* client);
		void removeClient(Tagaro::SpriteClient* client);
		void updateClient(Tagaro::SpriteClient* client);
		void updateAllClients();
	public Q_SLOTS:
		//If called with a null @a image, looks in the cache for the given
		//pixmap, or renders it synchronously on the given source. This
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
		QString m_processingInstruction;

		QHash<int, QPixmap> m_pixmapCache;
		QList<Tagaro::SpriteClient*> m_clients; //FIXME: utterly broken
};

struct Sprite::Private
{
	public:
		void setSource(const Tagaro::GraphicsSource* source, const QString& element);

		void addClient(Tagaro::SpriteClient* client);
		void removeClient(Tagaro::SpriteClient* client);
		Tagaro::SpriteFetcher* fetcher(const QSize& size, const QString& processingInstruction);
	private:
		friend class Tagaro::DeclarativeThemeProvider;
		friend class Tagaro::Sprite;
		friend class Tagaro::SpriteFetcher;
		Private();

		const Tagaro::GraphicsSource* m_source;
		QString m_element;
	
		QHash<QPair<QSize, QString>, SpriteFetcher*> m_fetchers; //key: size, processing instruction
		QList<Tagaro::SpriteClient*> m_clients;
};

struct SpriteClient::Private
{
	public:
		Private(Tagaro::Sprite* sprite, Tagaro::SpriteClient* q);
		void setFetcher(Tagaro::SpriteFetcher* fetcher);
		void receivePixmap(const QPixmap& pixmap);
	private:
		friend class Tagaro::SpriteClient;
		Tagaro::SpriteClient* q;
		Tagaro::Sprite* m_sprite;
		QSize m_size;
		QString m_processingInstruction;
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
