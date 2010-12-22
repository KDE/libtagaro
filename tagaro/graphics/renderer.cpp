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

#include "renderer.h"
#include "renderer_p.h"
#include "renderbackends.h"
#include "rendererclient.h"
#include "renderermodule.h"
#include "settings.h"
#include "theme.h"
#include "themeprovider.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QThreadPool>
#include <QtGui/QPainter>
#include <KDE/KDebug>

//TODO: automatically schedule pre-rendering of animation frames

static const QString cacheName(QString theme)
{
	const QString appName = QCoreApplication::instance()->applicationName();
	//e.g. "themes/foobar.desktop" -> "themes/foobar"
	if (theme.endsWith(QLatin1String(".desktop")))
		theme.truncate(theme.length() - 8); //8 = strlen(".desktop")
	//"-old" avoids collision with CachedProxyRenderBackend
	return QString::fromLatin1("tagarorenderer-old-%1-%2").arg(appName).arg(theme);
}

Tagaro::RendererPrivate::RendererPrivate(Tagaro::ThemeProvider* provider, const Tagaro::RenderBehavior& behavior, Tagaro::Renderer* parent)
	: m_parent(parent)
	, m_sizePrefix(QString::fromLatin1("%1-%2-"))
	, m_themeProvider(provider)
	//theme will be loaded on first request (not immediately, because the calling context might want to disable some rendering strategies first)
	, m_theme(0)
	, m_behavior(behavior)
	, m_rendererModule(0)
	, m_backend(0)
	, m_imageCache(0)
{
	qRegisterMetaType<Tagaro::RenderJob*>();
	connect(m_themeProvider, SIGNAL(selectedThemeChanged(const Tagaro::Theme*)), SLOT(loadSelectedTheme()));
}

Tagaro::Renderer::Renderer(Tagaro::ThemeProvider* provider, const Tagaro::RenderBehavior& behavior)
	: d(new Tagaro::RendererPrivate(provider, behavior, this))
{
}

Tagaro::Renderer::~Renderer()
{
	//cleanup clients
	while (!d->m_clients.isEmpty())
	{
		delete d->m_clients.constBegin().key();
	}
	//cleanup sprites (without qDeleteAll because that's not a friend of Sprite)
	QHash<QString, Tagaro::Sprite*>::const_iterator it1 = d->m_sprites.constBegin(),
	                                                it2 = d->m_sprites.constEnd();
	for (; it1 != it2; ++it1)
		delete it1.value();
	//cleanup own stuff
	QThreadPool::globalInstance()->waitForDone(); //TODO: optimize
	delete d->m_rendererModule;
	delete d->m_backend;
	delete d->m_imageCache;
	delete d;
}

Tagaro::ThemeProvider* Tagaro::Renderer::themeProvider() const
{
	return d->m_themeProvider;
}

void Tagaro::RendererPrivate::setTheme(const Tagaro::Theme* theme)
{
	if (!theme)
	{
		return;
	}
	const Tagaro::Theme* oldTheme = m_theme;
	if (oldTheme == theme)
	{
		return;
	}
	kDebug() << "Setting theme:" << theme->identifier();
	if (!setThemeInternal(theme))
	{
		const Tagaro::Theme* defaultTheme = m_themeProvider->defaultTheme();
		if (theme != defaultTheme && defaultTheme)
		{
			kDebug() << "Falling back to default theme:" << defaultTheme->identifier();
			setThemeInternal(defaultTheme);
		}
	}
	if (oldTheme != m_theme)
	{
		//announce change to renderer clients
		QHash<Tagaro::RendererClient*, QString>::iterator it1 = m_clients.begin(), it2 = m_clients.end();
		for (; it1 != it2; ++it1)
		{
			it1.value().clear(); //because the pixmap is outdated
			it1.key()->d->fetchPixmap();
		}
	}
}

void Tagaro::RendererPrivate::loadSelectedTheme()
{
	setTheme(m_themeProvider->selectedTheme());
}

bool Tagaro::RendererPrivate::setThemeInternal(const Tagaro::Theme* theme)
{
	const QString graphicsPath = theme->data(Tagaro::Theme::GraphicsFileRole).toString();
#if 1 //This #if marks the code that can be removed when RendererModule is removed.
	Tagaro::RendererModule* rendererModule = Tagaro::RendererModule::fromFile(graphicsPath);
	//open cache (and graphics file, if necessary)
	if (m_behavior.cacheSize() > 0 && Tagaro::Settings::useDiskCache())
	{
		KImageCache* oldCache = m_imageCache;
		const QString imageCacheName = cacheName(theme->identifier());
		m_imageCache = new KImageCache(imageCacheName, m_behavior.cacheSize());
		m_imageCache->setPixmapCaching(false); //see big comment in Tagaro::RendererPrivate class declaration
		//check timestamp of cache vs. last write access to theme/graphics file
		const uint fileTimestamp = theme->modificationTimestamp();
		QByteArray buffer;
		if (!m_imageCache->find(QString::fromLatin1("tagaro_timestamp"), &buffer))
			buffer = "0"; //krazy:exclude=doublequote_chars
		const uint cacheTimestamp = buffer.toInt();
		//try to instantiate renderer immediately if the cache does not exist or is outdated
		//FIXME: This logic breaks if the cache evicts the "tagaro_timestamp" key. We need additional API in KSharedDataCache to make sure that this key does not get evicted.
		if (cacheTimestamp < fileTimestamp)
		{
			kDebug() << "Theme newer than cache, checking graphics file";
			if (rendererModule->isValid())
			{
				QThreadPool::globalInstance()->waitForDone(); //TODO: optimize
				delete m_rendererModule;
				m_rendererModule = rendererModule;
				m_imageCache->insert(QString::fromLatin1("tagaro_timestamp"), QByteArray::number(fileTimestamp));
			}
			else
			{
				//The SVG file is broken, so we deny to change the theme without
				//breaking the previous theme.
				delete rendererModule;
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
			QThreadPool::globalInstance()->waitForDone(); //TODO: optimize
			delete m_rendererModule;
			m_rendererModule = rendererModule;
		}
	}
	else // !(m_strategies & Tagaro::Renderer::UseDiskCache) -> no cache is used
	{
		//load SVG file
		if (rendererModule->isValid())
		{
			QThreadPool::globalInstance()->waitForDone(); //TODO: optimize
			delete m_rendererModule;
			m_rendererModule = rendererModule;
		}
		else
		{
			kDebug() << "Theme change failed: Graphics file broken";
			delete rendererModule;
			return false;
		}
		//disconnect from disk cache (only needed if changing strategy)
		delete m_imageCache;
		m_imageCache = 0;
	}
#endif
	//instantiate backend
	Tagaro::RenderBackend* backend1 = new Tagaro::QtSvgRenderBackend(graphicsPath, m_behavior);
	Tagaro::RenderBackend* backend = new Tagaro::CachedProxyRenderBackend(backend1);
	if (!backend->load())
	{
		kDebug() << "Theme change failed: Graphics source broken";
		delete backend;
		return false;
	}
	//drop old backend
	delete m_backend;
	m_backend = backend;
	//clear in-process caches
	m_pixmapCache.clear();
	//done
	m_theme = theme;
	return true;
}

Tagaro::RenderBackend* Tagaro::Renderer::backend() const
{
	return d->m_backend;
}

Tagaro::Sprite* Tagaro::Renderer::sprite(const QString& key) const
{
	Tagaro::Sprite*& sprite = d->m_sprites[key];
	if (!sprite)
	{
		//instantiate on first use
		sprite = new Tagaro::Sprite(const_cast<Tagaro::Renderer*>(this), key);
	}
	return sprite;
}

QString Tagaro::RendererPrivate::spriteFrameKey(const QString& key, int frame, bool normalizeFrameNo) const
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
			const int fbi = m_behavior.frameBaseIndex();
			frame = (frame - fbi) % frameCount + fbi;
		}
	}
	return key + m_behavior.frameSuffix().arg(frame);
}

int Tagaro::Renderer::frameCount(const QString& key) const
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
	//ask backend
	return d->m_backend->frameCount(key);
}

QRectF Tagaro::Renderer::boundsOnSprite(const QString& key, int frame) const
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
	//ask backend
	return d->m_backend->elementBounds(elementKey);
}

bool Tagaro::Renderer::spriteExists(const QString& key) const
{
	return this->frameCount(key) >= 0;
}

QPixmap Tagaro::Renderer::spritePixmap(const QString& key, const QSize& size, int frame) const
{
	QPixmap result;
	d->requestPixmap(Tagaro::Internal::ClientSpec(key, frame, size), 0, &result);
	return result;
}

//Helper function for Tagaro::RendererPrivate::requestPixmap.
void Tagaro::RendererPrivate::requestPixmap__propagateResult(const QPixmap& pixmap, Tagaro::RendererClient* client, QPixmap* synchronousResult)
{
	if (client)
	{
		client->d->receivePixmapInternal(pixmap);
	}
	if (synchronousResult)
	{
		*synchronousResult = pixmap;
	}
}

void Tagaro::RendererPrivate::requestPixmap(const Tagaro::Internal::ClientSpec& spec, Tagaro::RendererClient* client, QPixmap* synchronousResult)
{
	//NOTE: If client == 0, the request is synchronous and must be finished when this method returns. This behavior is used by Tagaro::Renderer::spritePixmap(). Instead of Tagaro::RendererClient::receivePixmapInternal, the QPixmap* argument is then used to return the result.
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
	if (m_imageCache)
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
	Tagaro::RenderJob* job = new Tagaro::RenderJob;
	job->module = m_rendererModule;
	job->size = spec.size;
	job->elementKey = elementKey;
	const bool synchronous = !client;
	if (synchronous || !Tagaro::Settings::useRenderingThreads())
	{
		jobFinished(job, Tagaro::RenderWorker::run(job), true);
		//if everything worked fine, result is in high-speed cache now
		const QPixmap result = m_pixmapCache.value(cacheKey);
		requestPixmap__propagateResult(result, client, synchronousResult);
	}
	else
	{
		QThreadPool::globalInstance()->start(new Tagaro::RenderWorker(job, this));
		m_pendingRequests << cacheKey;
	}
}

void Tagaro::RendererPrivate::jobFinished(Tagaro::RenderJob* job, const QImage& result, bool isSynchronous)
{
	//read job
	const QString cacheKey = m_sizePrefix.arg(job->size.width()).arg(job->size.height()) + job->elementKey;
	delete job;
	//check who wanted this pixmap
	m_pendingRequests.removeAll(cacheKey);
	const QList<Tagaro::RendererClient*> requesters = m_clients.keys(cacheKey);
	//put result into image cache
	if (m_imageCache)
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
	foreach (Tagaro::RendererClient* requester, requesters)
	{
		requester->d->receivePixmapInternal(pixmap);
	}
}

#include "renderer.moc"
#include "renderer_p.moc"
