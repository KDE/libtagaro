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

#ifndef LIBKGAMEVISUALS_RENDERER_P_H
#define LIBKGAMEVISUALS_RENDERER_P_H

#include <QtCore/QHash>
#include <QtCore/QMetaType>
#include <QtCore/QMutex>
#include <QtCore/QRunnable>
#include <QtCore/QThreadPool>
#include <QtSvg/QSvgRenderer>
#include <KGameTheme>
#include <KImageCache>

namespace KGVRInternal
{
	//Describes the state of a KgvRendererClient.
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

	//Instantiates QSvgRenderer instances from one SVG file for multiple threads.
	class RendererPool
	{
		public:
			//The renderer pool needs the thread pool instance of
			//KgvRendererPrivate to terminate workers when a new SVG is loaded.
			//WARNING Call this only from the main thread.
			inline RendererPool(QThreadPool* threadPool);
			inline ~RendererPool();

			//The second argument can be used to pass an instance which has been
			//used earlier to check the validity of the SVG file.
			inline void setPath(const QString& svgPath, QSvgRenderer* renderer = 0);
			//This can be used to determine whether a call to allocRenderer()
			//would need to create a new renderer instance.
			inline bool hasAvailableRenderers() const;

			//Returns a SVG renderer instance that can be used in the calling thread.
			inline QSvgRenderer* allocRenderer();
			//Marks this renderer as available for allocation by other threads.
			inline void freeRenderer(QSvgRenderer* renderer);
		private:
			QString m_path;   //path to SVG file
			enum Validity { Checked_Invalid, Checked_Valid, Unchecked };
			Validity m_valid; //holds whether m_path points to a valid file

			mutable QMutex m_mutex;
			QThreadPool* m_threadPool;
			QHash<QSvgRenderer*, QThread*> m_hash;
	};

	//Describes a rendering job which is delegated to a worker thread.
	struct Job
	{
		KGVRInternal::RendererPool* rendererPool;
		ClientSpec spec;
		QString cacheKey, elementKey;
		QImage result;
	};

	//Describes a worker thread.
	class Worker : public QRunnable
	{
		public:
			Worker(Job* job, bool isSynchronous, KgvRendererPrivate* parent);

			virtual void run();
		private:
			Job* m_job;
			bool m_synchronous;
			KgvRendererPrivate* m_parent;
	};
};

Q_DECLARE_METATYPE(KGVRInternal::Job*)

class KgvRendererPrivate : public QObject
{
	Q_OBJECT
	public:
		KgvRendererPrivate(const QString& defaultTheme, unsigned cacheSize, KgvRenderer* parent);
		bool setTheme(const QString& theme);
		inline QString spriteFrameKey(const QString& key, int frame, bool normalizeFrameNo = false) const;
		void requestPixmap(const KGVRInternal::ClientSpec& spec, KgvRendererClient* client, QPixmap* synchronousResult = 0);
	private:
		inline void requestPixmap__propagateResult(const QPixmap& pixmap, KgvRendererClient* client, QPixmap* synchronousResult);
	public Q_SLOTS:
		void jobFinished(KGVRInternal::Job* job, bool isSynchronous); //NOTE: This is invoked from KGVRInternal::Worker::run.
	public:
		KgvRenderer* m_parent;

		QString m_defaultTheme, m_currentTheme;
		QString m_frameSuffix, m_sizePrefix, m_frameCountPrefix, m_boundsPrefix;
		unsigned m_cacheSize;
		KgvRenderer::Strategies m_strategies;
		int m_frameBaseIndex;
		KGameTheme m_theme;
		QGraphicsView* m_defaultPrimaryView;

		QThreadPool m_workerPool;
		KGVRInternal::RendererPool m_rendererPool;

		QHash<KgvRendererClient*, QString> m_clients; //maps client -> cache key of current pixmap
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

class KgvRendererClientPrivate : public QObject
{
	Q_OBJECT
	public:
		KgvRendererClientPrivate(KgvRenderer* renderer, const QString& spriteKey, KgvRendererClient* parent);
	public Q_SLOTS:
		void fetchPixmap();
	public:
		KgvRendererClient* m_parent;
		KgvRenderer* m_renderer;

		QPixmap m_pixmap;
		KGVRInternal::ClientSpec m_spec;
};

#endif // LIBKGAMEVISUALS_RENDERER_P_H
