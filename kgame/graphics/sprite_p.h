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

#ifndef KGAME_SPRITE_P_H
#define KGAME_SPRITE_P_H

#include "sprite.h"
#include "spriteclient.h"

#include <QtCore/QHash>

namespace KGame {

class GraphicsSource;
class SpriteClient;

class SpriteFetcher : public QObject
{
	Q_OBJECT
	public:
		SpriteFetcher(const QSize& size, const QString& processingInstruction, KGame::Sprite::Private* d) : d(d), m_size(size), m_processingInstruction(processingInstruction) {}

		void addClient(KGame::SpriteClient* client);
		void removeClient(KGame::SpriteClient* client);
		void updateClient(KGame::SpriteClient* client);
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

		KGame::Sprite::Private* const d;
		QSize m_size;
		QString m_processingInstruction;

		QHash<int, QPixmap> m_pixmapCache;
		QList<KGame::SpriteClient*> m_clients; //FIXME: utterly broken
};

struct Sprite::Private
{
	public:
		void setSource(const KGame::GraphicsSource* source, const QString& element);

		void addClient(KGame::SpriteClient* client);
		void removeClient(KGame::SpriteClient* client);
		KGame::SpriteFetcher* fetcher(const QSize& size, const QString& processingInstruction);
	private:
		friend class KGame::DeclarativeThemeProvider;
		friend class KGame::Sprite;
		friend class KGame::SpriteFetcher;
		Private();

		const KGame::GraphicsSource* m_source;
		QString m_element;
	
		QHash<QPair<QSize, QString>, SpriteFetcher*> m_fetchers; //key: size, processing instruction
		QList<KGame::SpriteClient*> m_clients;
};

struct SpriteClient::Private
{
	public:
		Private(KGame::Sprite* sprite, KGame::SpriteClient* q);
		void setFetcher(KGame::SpriteFetcher* fetcher);
		void receivePixmap(const QPixmap& pixmap);
	private:
		friend class KGame::SpriteClient;
		KGame::SpriteClient* q;
		KGame::Sprite* m_sprite;
		QSize m_size;
		QString m_processingInstruction;
		KGame::SpriteFetcher* m_fetcher;
		int m_frame;
		QPixmap m_pixmap;
};

} //namespace KGame

inline uint qHash(const QSize& size)
{
	return qHash(qMakePair(size.width(), size.height()));
}

#endif // KGAME_SPRITE_P_H
