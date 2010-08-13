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

#include "kgvrenderer.h"
#include "kgvrenderer_p.h"
#include "kgvrendererclient.h"
#include "kgvtheme.h"
#include "kgvthemeprovider.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtGui/QPainter>
#include <KDebug>

//TODO: automatically schedule pre-rendering of animation frames

static const QString cacheName(QString theme)
{
	const QString appName = QCoreApplication::instance()->applicationName();
	//e.g. "themes/foobar.desktop" -> "themes/foobar"
	if (theme.endsWith(QLatin1String(".desktop")))
		theme.truncate(theme.length() - 8); //8 = strlen(".desktop")
	return QString::fromLatin1("kgvrenderer-%1-%2").arg(appName).arg(theme);
}

KgvRendererPrivate::KgvRendererPrivate(KgvThemeProvider* provider, unsigned cacheSize, KgvRenderer* parent)
	: m_parent(parent)
	, m_frameSuffix(QString::fromLatin1("_%1"))
	, m_sizePrefix(QString::fromLatin1("%1-%2-"))
	, m_frameCountPrefix(QString::fromLatin1("fc-"))
	, m_boundsPrefix(QString::fromLatin1("br-"))
	//default cache size: 3 MiB = 3 << 20 bytes
	, m_cacheSize((cacheSize == 0 ? 3 : cacheSize) << 20)
	, m_strategies(KgvRenderer::UseDiskCache | KgvRenderer::UseRenderingThreads)
	, m_frameBaseIndex(0)
	, m_themeProvider(provider)
	//theme will be loaded on first request (not immediately, because the calling context might want to disable some rendering strategies first)
	, m_theme(0)
	, m_rendererPool(&m_workerPool)
	, m_imageCache(0)
{
	qRegisterMetaType<KGVRInternal::Job*>();
	connect(m_themeProvider, SIGNAL(selectedIndexChanged(int)), SLOT(loadSelectedTheme()));
}

KgvRenderer::KgvRenderer(KgvThemeProvider* provider, unsigned cacheSize)
	: d(new KgvRendererPrivate(provider, cacheSize, this))
{
}

KgvRenderer::~KgvRenderer()
{
	//cleanup clients (I explicitly take a copy instead of iterating over m_clients directly, because m_clients changes during the cleanup, when the clients deregister themselves from this renderer)
	const QList<KgvRendererClient*> clients = d->m_clients.keys();
	qDeleteAll(clients);
	//cleanup own stuff
	d->m_workerPool.waitForDone();
	delete d->m_imageCache;
	delete d;
}

int KgvRenderer::frameBaseIndex() const
{
	return d->m_frameBaseIndex;
}

void KgvRenderer::setFrameBaseIndex(int frameBaseIndex)
{
	d->m_frameBaseIndex = frameBaseIndex;
}

QString KgvRenderer::frameSuffix() const
{
	return d->m_frameSuffix;
}

void KgvRenderer::setFrameSuffix(const QString& suffix)
{
	d->m_frameSuffix = suffix.contains(QLatin1String("%1")) ? suffix : QLatin1String("_%1");
}

KgvRenderer::Strategies KgvRenderer::strategies() const
{
	return d->m_strategies;
}

void KgvRenderer::setStrategyEnabled(KgvRenderer::Strategy strategy, bool enabled)
{
	const bool oldEnabled = d->m_strategies & strategy;
	if (enabled)
	{
		d->m_strategies |= strategy;
	}
	else
	{
		d->m_strategies &= ~strategy;
	}
	if (strategy == KgvRenderer::UseDiskCache && oldEnabled != enabled)
	{
		const KgvTheme* theme = d->m_theme;
		d->m_theme = 0; //or setTheme() will return immediately
		d->setTheme(theme);
	}
}

KgvThemeProvider* KgvRenderer::themeProvider() const
{
	return d->m_themeProvider;
}

const KgvTheme* KgvRenderer::theme() const
{
	return d->m_theme;
}

void KgvRendererPrivate::setTheme(const KgvTheme* theme)
{
	if (!theme)
	{
		return;
	}
	const KgvTheme* oldTheme = m_theme;
	if (oldTheme == theme)
	{
		return;
	}
	kDebug() << "Setting theme:" << theme->identifier();
	if (!setThemeInternal(theme))
	{
		const KgvTheme* defaultTheme = m_themeProvider->theme(0);
		if (theme != defaultTheme && defaultTheme)
		{
			kDebug() << "Falling back to default theme:" << defaultTheme->identifier();
			setThemeInternal(defaultTheme);
		}
	}
	//announce change to KgvRendererClients
	QHash<KgvRendererClient*, QString>::iterator it1 = m_clients.begin(), it2 = m_clients.end();
	for (; it1 != it2; ++it1)
	{
		it1.value().clear(); //because the pixmap is outdated
		it1.key()->d->fetchPixmap();
	}
	//announce change publicly
	if (oldTheme != m_theme)
	{
		emit m_parent->themeChanged(m_theme);
	}
}

void KgvRendererPrivate::loadSelectedTheme()
{
	setTheme(m_themeProvider->theme(m_themeProvider->selectedIndex()));
}

bool KgvRendererPrivate::setThemeInternal(const KgvTheme* theme)
{
	const QString svgPath = theme->data(KgvTheme::GraphicsFileRole).toString();
	//open cache (and SVG file, if necessary)
	if (m_strategies & KgvRenderer::UseDiskCache)
	{
		KImageCache* oldCache = m_imageCache;
		const QString imageCacheName = cacheName(m_theme->identifier());
		m_imageCache = new KImageCache(imageCacheName, m_cacheSize);
		m_imageCache->setPixmapCaching(false); //see big comment in KGVRPrivate class declaration
		//check timestamp of cache vs. last write access to theme/SVG
		const uint svgTimestamp = theme->modificationTimestamp();
		QByteArray buffer;
		if (!m_imageCache->find(QString::fromLatin1("kgvr_timestamp"), &buffer))
			buffer = "0";
		const uint cacheTimestamp = buffer.toInt();
		//try to instantiate renderer immediately if the cache does not exist or is outdated
		//FIXME: This logic breaks if the cache evicts the "kgvr_timestamp" key. We need additional API in KSharedDataCache to make sure that this key does not get evicted.
		if (cacheTimestamp < svgTimestamp)
		{
			kDebug() << "Theme newer than cache, checking SVG";
			QSvgRenderer* renderer = new QSvgRenderer(svgPath);
			if (renderer->isValid())
			{
				m_rendererPool.setPath(svgPath, renderer);
				m_imageCache->insert(QString::fromLatin1("kgvr_timestamp"), QByteArray::number(svgTimestamp));
			}
			else
			{
				//The SVG file is broken, so we deny to change the theme without
				//breaking the previous theme.
				delete m_imageCache;
				KSharedDataCache::deleteCache(imageCacheName);
				m_imageCache = oldCache;
				kDebug() << "Theme change failed: SVG file broken";
				return false;
			}
		}
		//theme is cached - just delete the old renderer after making sure that no worker threads are using it anymore
		else if (m_theme != theme)
		{
			m_rendererPool.setPath(svgPath);
		}
	}
	else // !(m_strategies & KgvRenderer::UseDiskCache) -> no cache is used
	{
		//load SVG file
		QSvgRenderer* renderer = new QSvgRenderer(svgPath);
		if (renderer->isValid())
		{
			m_rendererPool.setPath(svgPath, renderer);
		}
		else
		{
			kDebug() << "Theme change failed: SVG file broken";
			return false;
		}
		//disconnect from disk cache (only needed if changing strategy)
		delete m_imageCache;
		m_imageCache = 0;
	}
	//clear in-process caches
	m_pixmapCache.clear();
	m_frameCountCache.clear();
	m_boundsCache.clear();
	//done
	m_theme = theme;
	return true;
}

QString KgvRendererPrivate::spriteFrameKey(const QString& key, int frame, bool normalizeFrameNo) const
{
	//fast path for non-animated sprites
	if (frame < 0)
	{
		return key;
	}
	//normalize frame number
	if (normalizeFrameNo)
	{
		const int frameCount = m_parent->frameCount(key);
		if (frameCount <= 0)
		{
			//non-animated sprite
			return key;
		}
		else
		{
			frame = (frame - m_frameBaseIndex) % frameCount + m_frameBaseIndex;
		}
	}
	return key + m_frameSuffix.arg(frame);
}

int KgvRenderer::frameCount(const QString& key) const
{
	//ensure that some theme is loaded
	if (!d->m_theme)
	{
		d->loadSelectedTheme();
		if (!d->m_theme)
		{
			return -1;
		}
	}
	//look up in in-process cache
	QHash<QString, int>::const_iterator it = d->m_frameCountCache.constFind(key);
	if (it != d->m_frameCountCache.constEnd())
	{
		return it.value();
	}
	//look up in shared cache (if SVG is not yet loaded)
	int count = -1;
	bool countFound = false;
	const QString cacheKey = d->m_frameCountPrefix + key;
	if (d->m_rendererPool.hasAvailableRenderers() && (d->m_strategies & KgvRenderer::UseDiskCache))
	{
		QByteArray buffer;
		if (d->m_imageCache->find(cacheKey, &buffer))
		{
			count = buffer.toInt();
			countFound = true;
		}
	}
	//determine from SVG
	if (!countFound)
	{
		QSvgRenderer* renderer = d->m_rendererPool.allocRenderer();
		//look for animated sprite first
		count = d->m_frameBaseIndex;
		while (renderer->elementExists(d->spriteFrameKey(key, count, false)))
		{
			++count;
		}
		count -= d->m_frameBaseIndex;
		//look for non-animated sprite instead
		if (count == 0)
		{
			if (!renderer->elementExists(key))
			{
				count = -1;
			}
		}
		d->m_rendererPool.freeRenderer(renderer);
		//save in shared cache for following requests
		if (d->m_strategies & KgvRenderer::UseDiskCache)
		{
			d->m_imageCache->insert(cacheKey, QByteArray::number(count));
		}
	}
	d->m_frameCountCache.insert(key, count);
	return count;
}

QRectF KgvRenderer::boundsOnSprite(const QString& key, int frame) const
{
	const QString elementKey = d->spriteFrameKey(key, frame);
	//ensure that some theme is loaded
	if (!d->m_theme)
	{
		d->loadSelectedTheme();
		if (!d->m_theme)
		{
			return QRectF();
		}
	}
	//look up in in-process cache
	QHash<QString, QRectF>::const_iterator it = d->m_boundsCache.constFind(elementKey);
	if (it != d->m_boundsCache.constEnd())
	{
		return it.value();
	}
	//look up in shared cache (if SVG is not yet loaded)
	QRectF bounds;
	bool boundsFound = false;
	const QString cacheKey = d->m_boundsPrefix + elementKey;
	if (!d->m_rendererPool.hasAvailableRenderers() && (d->m_strategies & KgvRenderer::UseDiskCache))
	{
		QByteArray buffer;
		if (d->m_imageCache->find(cacheKey, &buffer))
		{
			QDataStream stream(buffer);
			stream >> bounds;
			boundsFound = true;
		}
	}
	//determine from SVG
	if (!boundsFound)
	{
		QSvgRenderer* renderer = d->m_rendererPool.allocRenderer();
		bounds = renderer->boundsOnElement(elementKey);
		d->m_rendererPool.freeRenderer(renderer);
		//save in shared cache for following requests
		if (d->m_strategies & KgvRenderer::UseDiskCache)
		{
			QByteArray buffer;
			{
				QDataStream stream(&buffer, QIODevice::WriteOnly);
				stream << bounds;
			}
			d->m_imageCache->insert(cacheKey, buffer);
		}
	}
	d->m_boundsCache.insert(elementKey, bounds);
	return bounds;
}

bool KgvRenderer::spriteExists(const QString& key) const
{
	return this->frameCount(key) >= 0;
}

QPixmap KgvRenderer::spritePixmap(const QString& key, const QSize& size, int frame) const
{
	QPixmap result;
	d->requestPixmap(KGVRInternal::ClientSpec(key, frame, size), 0, &result);
	return result;
}

//Helper function for KgvRendererPrivate::requestPixmap.
void KgvRendererPrivate::requestPixmap__propagateResult(const QPixmap& pixmap, KgvRendererClient* client, QPixmap* synchronousResult)
{
	if (client)
	{
		client->receivePixmap(pixmap);
	}
	if (synchronousResult)
	{
		*synchronousResult = pixmap;
	}
}

void KgvRendererPrivate::requestPixmap(const KGVRInternal::ClientSpec& spec, KgvRendererClient* client, QPixmap* synchronousResult)
{
	//NOTE: If client == 0, the request is synchronous and must be finished when this method returns. This behavior is used by KGVR::spritePixmap(). Instead of KgvRendererClient::receivePixmap, the QPixmap* argument is then used to return the result.
	//parse request
	if (spec.size.isEmpty())
	{
		requestPixmap__propagateResult(QPixmap(), client, synchronousResult);
		return;
	}
	const QString elementKey = spriteFrameKey(spec.spriteKey, spec.frame);
	const QString cacheKey = m_sizePrefix.arg(spec.size.width()).arg(spec.size.height()) + elementKey;
	//check if update is needed
	if (client)
	{
		if (m_clients.value(client) == cacheKey)
		{
			return;
		}
		m_clients[client] = cacheKey;
	}
	//ensure that some theme is loaded
	if (!m_theme)
	{
		loadSelectedTheme();
		if (!m_theme)
		{
			return;
		}
	}
	//try to serve from high-speed cache
	QHash<QString, QPixmap>::const_iterator it = m_pixmapCache.constFind(cacheKey);
	if (it != m_pixmapCache.constEnd())
	{
		requestPixmap__propagateResult(it.value(), client, synchronousResult);
		return;
	}
	//try to serve from low-speed cache
	if (m_strategies & KgvRenderer::UseDiskCache)
	{
		QPixmap pix;
		if (m_imageCache->findPixmap(cacheKey, &pix))
		{
			m_pixmapCache.insert(cacheKey, pix);
			requestPixmap__propagateResult(pix, client, synchronousResult);
			return;
		}
	}
	//if asynchronous request, is such a rendering job already running?
	if (client && m_pendingRequests.contains(cacheKey))
	{
		return;
	}
	//create job
	KGVRInternal::Job* job = new KGVRInternal::Job;
	job->rendererPool = &m_rendererPool;
	job->cacheKey = cacheKey;
	job->elementKey = elementKey;
	job->spec = spec;
	const bool synchronous = !client;
	KGVRInternal::Worker* worker = new KGVRInternal::Worker(job, synchronous, this);
	if (synchronous || !(m_strategies & KgvRenderer::UseRenderingThreads))
	{
		worker->run();
		//if everything worked fine, result is in high-speed cache now
		const QPixmap result = m_pixmapCache.value(cacheKey);
		if (synchronousResult)
		{
			*synchronousResult = result;
		}
		if (client)
		{
			client->receivePixmap(result);
		}
	}
	else
	{
		m_workerPool.start(new KGVRInternal::Worker(job, !client, this));
		m_pendingRequests << cacheKey;
	}
}

void KgvRendererPrivate::jobFinished(KGVRInternal::Job* job, bool isSynchronous)
{
	//read job
	const QString cacheKey = job->cacheKey;
	const QImage result = job->result;
	delete job;
	//check who wanted this pixmap
	m_pendingRequests.removeAll(cacheKey);
	const QList<KgvRendererClient*> requesters = m_clients.keys(cacheKey);
	//put result into image cache
	if (m_strategies & KgvRenderer::UseDiskCache)
	{
		m_imageCache->insertImage(cacheKey, result);
		//convert result to pixmap (and put into pixmap cache) only if it is needed now
		//This optimization saves the image-pixmap conversion for intermediate sizes which occur during smooth resize events or window initializations.
		if (!isSynchronous && requesters.isEmpty())
		{
			return;
		}
	}
	const QPixmap pixmap = QPixmap::fromImage(result);
	m_pixmapCache.insert(cacheKey, pixmap);
	foreach (KgvRendererClient* requester, requesters)
	{
		requester->receivePixmap(pixmap);
	}
}

//BEGIN KGVRInternal::Job/Worker

KGVRInternal::Worker::Worker(KGVRInternal::Job* job, bool isSynchronous, KgvRendererPrivate* parent)
	: m_job(job)
	, m_synchronous(isSynchronous)
	, m_parent(parent)
{
}

static const uint transparentRgba = QColor(Qt::transparent).rgba();

void KGVRInternal::Worker::run()
{
	QImage image(m_job->spec.size, QImage::Format_ARGB32_Premultiplied);
	image.fill(transparentRgba);
	//do renderering
	QPainter painter(&image);
	QSvgRenderer* renderer = m_job->rendererPool->allocRenderer();
	renderer->render(&painter, m_job->elementKey);
	m_job->rendererPool->freeRenderer(renderer);
	painter.end();
	//talk back to the main thread
	m_job->result = image;
	QMetaObject::invokeMethod(
		m_parent, "jobFinished", Qt::AutoConnection,
		Q_ARG(KGVRInternal::Job*, m_job), Q_ARG(bool, m_synchronous)
	);
	//NOTE: KGVR::spritePixmap relies on Qt::DirectConnection when this method is run in the main thread.
}

//END KGVRInternal::Job/Worker

//BEGIN KGVRInternal::RendererPool

KGVRInternal::RendererPool::RendererPool(QThreadPool* threadPool)
	: m_valid(Checked_Invalid) //don't try to allocate renderers until given a valid SVG file
	, m_threadPool(threadPool)
{
}

KGVRInternal::RendererPool::~RendererPool()
{
	//This deletes all renderers.
	setPath(QString());
}

void KGVRInternal::RendererPool::setPath(const QString& svgPath, QSvgRenderer* renderer)
{
	QMutexLocker locker(&m_mutex);
	//delete all renderers
	m_threadPool->waitForDone();
	QHash<QSvgRenderer*, QThread*>::const_iterator it1 = m_hash.constBegin(), it2 = m_hash.constEnd();
	for (; it1 != it2; ++it1)
	{
		Q_ASSERT(it1.value() == 0); //nobody may be using our renderers anymore now
		delete it1.key();
	}
	m_hash.clear();
	//set path
	m_path = svgPath;
	//existence of a renderer instance is evidence for the validity of the SVG file
	if (renderer)
	{
		m_valid = Checked_Valid;
		m_hash.insert(renderer, 0);
	}
	else
	{
		m_valid = Unchecked;
	}
}

bool KGVRInternal::RendererPool::hasAvailableRenderers() const
{
	//look for a renderer which is not associated with a thread
	QMutexLocker locker(&m_mutex);
	return m_hash.key(0) != 0;
}

QSvgRenderer* KGVRInternal::RendererPool::allocRenderer()
{
	QThread* thread = QThread::currentThread();
	//look for an available renderer
	QMutexLocker locker(&m_mutex);
	QSvgRenderer* renderer = m_hash.key(0);
	if (!renderer)
	{
		//instantiate a new renderer (only if the SVG file has not been found to be invalid yet)
		if (m_valid == Checked_Invalid)
		{
			return 0;
		}
		renderer = new QSvgRenderer(m_path);
		m_valid = renderer->isValid() ? Checked_Valid : Checked_Invalid;
	}
	//mark renderer as used
	m_hash.insert(renderer, thread);
	return renderer;
}

void KGVRInternal::RendererPool::freeRenderer(QSvgRenderer* renderer)
{
	//mark renderer as available
	QMutexLocker locker(&m_mutex);
	m_hash.insert(renderer, 0);
}

//END KGVRInternal::RendererPool

#include "kgvrenderer.moc"
#include "kgvrenderer_p.moc"
