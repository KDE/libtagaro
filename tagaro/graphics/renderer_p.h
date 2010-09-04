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

#ifndef TAGARO_RENDERER_P_H
#define TAGARO_RENDERER_P_H

#include "renderermodule.h"

#include <QtCore/QHash>
#include <QtCore/QMetaType>
#include <QtCore/QMutex>
#include <QtCore/QRunnable>
#include <QtCore/QThreadPool>
#include <KDE/KImageCache>

namespace Tagaro {

namespace Internal
{
	//Describes the state of a Tagaro::RendererClient.
	struct ClientSpec
	{
		inline ClientSpec(const QString& spriteKey = QString(), int frame = -1, const QSize& size = QSize());
		QString spriteKey;
		int frame;
		QSize size;
	};
	ClientSpec::ClientSpec(const QString& spriteKey_, int frame_, const QSize& size_)
		: spriteKey(spriteKey_)
		, frame(frame_)
		, size(size_)
	{
	}

	//Describes a rendering job which is delegated to a worker thread.
	struct Job
	{
		Tagaro::RendererModule* rendererModule;
		ClientSpec spec;
		QString cacheKey, elementKey;
		QImage result;
	};

	//Describes a worker thread.
	class Worker : public QRunnable
	{
		public:
			Worker(Job* job, bool isSynchronous, Tagaro::RendererPrivate* parent);

			virtual void run();
		private:
			Job* m_job;
			bool m_synchronous;
			Tagaro::RendererPrivate* m_parent;
	};
};

class RendererPrivate : public QObject
{
	Q_OBJECT
	public:
		RendererPrivate(Tagaro::ThemeProvider* provider, unsigned cacheSize, Tagaro::Renderer* parent);
		void setTheme(const Tagaro::Theme* theme);
		bool setThemeInternal(const Tagaro::Theme* theme);
		inline QString spriteFrameKey(const QString& key, int frame, bool normalizeFrameNo = false) const;
		void requestPixmap(const Internal::ClientSpec& spec, Tagaro::RendererClient* client, QPixmap* synchronousResult = 0);
	private:
		inline void requestPixmap__propagateResult(const QPixmap& pixmap, Tagaro::RendererClient* client, QPixmap* synchronousResult);
	public Q_SLOTS:
		void loadSelectedTheme();
		void jobFinished(Tagaro::Internal::Job* job, bool isSynchronous); //NOTE: This is invoked from Internal::Worker::run.
	public:
		Tagaro::Renderer* m_parent;

		QString m_frameSuffix, m_sizePrefix, m_frameCountPrefix, m_boundsPrefix;
		unsigned m_cacheSize;
		Tagaro::Renderer::Strategies m_strategies;
		int m_frameBaseIndex;
		Tagaro::ThemeProvider* m_themeProvider;
		const Tagaro::Theme* m_theme;

		QThreadPool m_workerPool;
		Tagaro::RendererModule* m_rendererModule;

		QHash<Tagaro::RendererClient*, QString> m_clients; //maps client -> cache key of current pixmap
		QStringList m_pendingRequests; //cache keys of pixmaps which are currently being rendered

		KImageCache* m_imageCache;
		//In multi-threaded scenarios, there are two possible ways to use KIC's
		//pixmap cache.
		//1. The worker renders a QImage and stores it in the cache. The main
		//   thread reads the QImage again and converts it into a QPixmap,
		//   storing it inthe pixmap cache for later re-use.
		//i.e. QImage -> diskcache -> QImage -> QPixmap -> pixmapcache -> serve
		//2. The worker renders a QImage and sends it directly to the main
		//   thread, which converts it to a QPixmap. The QPixmap is stored in
		//   KIC's pixmap cache, and converted to QImage to be written to the
		//   shared data cache.
		//i.e. QImage -> QPixmap -> pixmapcache -> serve
		//                      \-> QImage -> diskcache
		//We choose a third way:
		//3. The worker renders a QImage which is converted to a QPixmap by the
		//   main thread. The main thread caches the QPixmap itself, and stores
		//   the QImage in the cache.
		//i.e. QImage -> QPixmap -> pixmapcache -> serve
		//           \-> diskcache
		//As you see, implementing an own pixmap cache saves us one conversion.
		//We therefore disable KIC's pixmap cache because we do not need it.
		QHash<QString, QPixmap> m_pixmapCache;
		QHash<QString, int> m_frameCountCache;
		QHash<QString, QRectF> m_boundsCache;
};

class RendererClientPrivate : public QObject
{
	Q_OBJECT
	public:
		RendererClientPrivate(Tagaro::Renderer* renderer, const QString& spriteKey, Tagaro::RendererClient* parent);

		void receivePixmapInternal(const QPixmap& pixmap);
	public Q_SLOTS:
		void fetchPixmap();
	public:
		Tagaro::RendererClient* m_parent;
		Tagaro::Renderer* m_renderer;

		QPixmap m_pixmap;
		Internal::ClientSpec m_spec;
		bool m_fetching;
};

} //namespace Tagaro

Q_DECLARE_METATYPE(Tagaro::Internal::Job*)

#endif // TAGARO_RENDERER_P_H
