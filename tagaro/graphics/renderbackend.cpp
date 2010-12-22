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

#include "renderbackend.h"
#include "settings.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringBuilder>
#include <QtCrypto/QtCrypto>
#include <KDE/KGlobal>
#include <KDE/KImageCache>
#include <KDE/KStandardDirs>

//BEGIN Tagaro::RenderBehavior

struct Tagaro::RenderBehavior::Private
{
	int m_cacheSize, m_frameBaseIndex;
	QString m_frameSuffix;

	Private();
};

Tagaro::RenderBehavior::Private::Private()
	: m_cacheSize(3) //in megabytes
	, m_frameBaseIndex(0)
	, m_frameSuffix(QLatin1String("_%1"))
{
}

Tagaro::RenderBehavior::RenderBehavior()
	: d(new Private)
{
}

Tagaro::RenderBehavior::RenderBehavior(const Tagaro::RenderBehavior& other)
	: d(new Private(*other.d))
{
}

Tagaro::RenderBehavior& Tagaro::RenderBehavior::operator=(const Tagaro::RenderBehavior& other)
{
	*d = *other.d;
	return *this;
}

Tagaro::RenderBehavior::~RenderBehavior()
{
	delete d;
}

int Tagaro::RenderBehavior::cacheSize() const
{
	return d->m_cacheSize;
}

void Tagaro::RenderBehavior::setCacheSize(int cacheSize)
{
	d->m_cacheSize = cacheSize;
}

int Tagaro::RenderBehavior::frameBaseIndex() const
{
	return d->m_frameBaseIndex;
}

void Tagaro::RenderBehavior::setFrameBaseIndex(int frameBaseIndex)
{
	d->m_frameBaseIndex = frameBaseIndex;
}

QString Tagaro::RenderBehavior::frameSuffix() const
{
	return d->m_frameSuffix;
}

void Tagaro::RenderBehavior::setFrameSuffix(const QString& suffix)
{
	d->m_frameSuffix = suffix.contains(QLatin1String("%1")) ? suffix : QLatin1String("_%1");
}

//END Tagaro::RenderBehavior
//BEGIN Tagaro::RenderBackend

struct Tagaro::RenderBackend::Private
{
	Tagaro::RenderBehavior m_behavior;
	QString m_identifier;

	Private(const QString& identifier, const Tagaro::RenderBehavior& behavior) : m_behavior(behavior), m_identifier(identifier) {}
};

Tagaro::RenderBackend::RenderBackend(const QString& identifier, const Tagaro::RenderBehavior& behavior)
	: d(new Private(identifier, behavior))
{
}

Tagaro::RenderBackend::~RenderBackend()
{
	delete d;
}

const Tagaro::RenderBehavior& Tagaro::RenderBackend::behavior() const
{
	return d->m_behavior;
}

QString Tagaro::RenderBackend::identifier() const
{
	return d->m_identifier;
}

bool Tagaro::RenderBackend::load()
{
	return true;
}

uint Tagaro::RenderBackend::lastModified() const
{
	//see documentation
	return 0;
}

QRectF Tagaro::RenderBackend::elementBounds(const QString& element) const
{
	Q_UNUSED(element)
	return QRectF();
}

int Tagaro::RenderBackend::frameCount(const QString& element) const
{
	//look for animated sprite first
	const int fbi = d->m_behavior.frameBaseIndex();
	int count = fbi;
	while (elementExists(frameElementKey(element, count, false)))
	{
		++count;
	}
	count -= fbi;
	//look for non-animated sprite instead
	if (count == 0)
	{
		if (!elementExists(element))
		{
			count = -1;
		}
	}
	return count;
}

QString Tagaro::RenderBackend::frameElementKey(const QString& element, int frame, bool useFrameCount) const
{
	//fast path for non-animated sprites
	if (frame < 0)
	{
		return element;
	}
	//normalize frame number
	if (useFrameCount)
	{
		const int frameCount = this->frameCount(element);
		if (frameCount <= 0)
		{
			//non-animated or missing sprite
			return element;
		}
		else
		{
			const int fbi = d->m_behavior.frameBaseIndex();
			frame = (frame - fbi) % frameCount + fbi;
		}
	}
	return element + d->m_behavior.frameSuffix().arg(frame);
}

//END Tagaro::RenderBackend
//BEGIN Tagaro::CachedProxyRenderBackend

struct Tagaro::CachedProxyRenderBackend::Private
{
	Tagaro::RenderBackend* m_backend;
	//disk cache
	KImageCache* m_cache;
	//in-process cache
	QHash<QString, QRectF> m_boundsCache;
	QHash<QString, int> m_frameCountCache;
	//state description
	bool m_valid, m_loaded, m_backendLoaded, m_useCache;
	//m_useCache refers to the disk cache only, not to the in-process cache

	Private(Tagaro::RenderBackend* backend);

	QRectF elementBounds(const QString& element);
	QImage elementImage(const QString& element, const QSize& size, bool timeConstraint);
};

Tagaro::CachedProxyRenderBackend::Private::Private(Tagaro::RenderBackend* backend)
	: m_backend(backend)
	, m_cache(0)
	, m_valid(false)
	, m_loaded(false)
	, m_backendLoaded(false)
	, m_useCache(Tagaro::Settings::useDiskCache() && backend->behavior().cacheSize() > 0)
{
}

Tagaro::CachedProxyRenderBackend::CachedProxyRenderBackend(Tagaro::RenderBackend* backend)
	: Tagaro::RenderBackend(backend->identifier(), backend->behavior())
	, d(new Private(backend))
{
}

Tagaro::CachedProxyRenderBackend::~CachedProxyRenderBackend()
{
	delete d->m_backend;
	delete d->m_cache;
	delete d;
}

struct Tagaro_QCAStuff : public QCA::Initializer
{
	QCA::Hash hash;

	Tagaro_QCAStuff()
		: hash("sha1")
	{
		Q_ASSERT(QCA::isSupported("sha1"));
	}
};
K_GLOBAL_STATIC(Tagaro_QCAStuff, g_qcaStuff)

bool Tagaro::CachedProxyRenderBackend::load()
{
	if (d->m_loaded)
	{
		return d->m_valid;
	}
	d->m_loaded = true; //because we're doing it now
	//no caching -> just forward call to proxied backend
	if (!d->m_useCache)
	{
		d->m_backendLoaded = true;
		return d->m_valid = d->m_backend->load();
	}
	//hash identifier to find name for cache
	const QString cacheHash = g_qcaStuff->hash.hashToString(identifier().toUtf8());
	const QString appName = QCoreApplication::instance()->applicationName();
	const QString cacheName = QString::fromLatin1("tagarorenderer/") % appName % QChar('/') % cacheHash;
	kDebug() << "Opening cache:" << cacheName;
	//does cache exist at all? if not, check graphics source before continuing
	const bool cacheExists = KStandardDirs::exists(KStandardDirs::locate("cache", cacheName + QLatin1String(".kcache")));
	if (cacheExists)
	{
		d->m_valid = true;
	}
	else
	{
		kDebug() << "Cache does not exist, checking graphics source immediately";
		d->m_backendLoaded = true;
		d->m_valid = d->m_backend->load();
		if (!d->m_valid)
		{
			return d->m_valid;
		}
	}
	//open cache, check timestamp vs. last write access to graphics file
	//(shift by 20 converts megabytes to bytes)
	d->m_cache = new KImageCache(cacheName, behavior().cacheSize() << 20);
	d->m_cache->setPixmapCaching(false); //see comment below this method
	//TODO: use Tagaro::Theme::modificationTimestamp()
	if (d->m_cache->timestamp() < d->m_backend->lastModified())
	{
		d->m_cache->clear();
		kDebug() << "Theme newer than cache, checking graphics file immediately";
		d->m_backendLoaded = true;
		d->m_valid = d->m_backend->load();
	}
	return d->m_valid;
}

//The long explanation for setPixmapCaching(false) above: In multi-threaded
//scenarios, there are two possible ways to use KIC's pixmap cache.
//1. The worker thread renders a QImage and stores it in the cache. The main
//   thread reads the QImage again and converts it into a QPixmap, storing it
//   in the pixmap cache for later re-use.
//i.e. QImage -> diskcache -> QImage -> QPixmap -> pixmapcache -> serve
//2. The worker renders a QImage and sends it directly to the main thread,
//   which converts it to a QPixmap. The QPixmap is stored in KIC's pixmap
//   cache, and converted to QImage to be written to the shared data cache.
//i.e. QImage -> QPixmap -> pixmapcache -> serve
//                      \-> QImage -> diskcache
//We choose a third way:
//3. The worker renders a QImage which is converted to a QPixmap by the main
//   thread. The main thread caches the QPixmap itself, and stores the QImage
//   in the cache.
//i.e. QImage -> QPixmap -> pixmapcache -> serve
//           \-> diskcache
//As you see, implementing an own pixmap cache saves us one conversion. We
//therefore disable KIC's pixmap cache because we do not need it.

QRectF Tagaro::CachedProxyRenderBackend::Private::elementBounds(const QString& element)
{
	if (!m_backendLoaded)
	{
		m_backendLoaded = true;
		m_valid = m_backend->load();
	}
	return m_valid ? m_backend->elementBounds(element) : QRectF();
}

QRectF Tagaro::CachedProxyRenderBackend::elementBounds(const QString& element) const
{
	//fast return if load() has not been called yet or if graphical source is invalid
	if (!d->m_valid)
	{
		return QRectF();
	}
	//check fast cache
	QHash<QString, QRectF>::const_iterator it = d->m_boundsCache.constFind(element);
	if (it != d->m_boundsCache.constEnd())
	{
		return it.value();
	}
	//if there's no slow cache...
	if (!d->m_cache)
	{
		const QRectF bounds = d->elementBounds(element);
		d->m_boundsCache.insert(element, bounds);
		return bounds;
	}
	//...else check slow cache
	const QString key = QLatin1String("br-") + element;
	QByteArray buffer;
	if (d->m_cache->find(key, &buffer))
	{
		QDataStream stream(buffer);
		QRectF bounds;
		stream >> bounds;
		d->m_boundsCache.insert(element, bounds);
		return bounds;
	}
	//ask backend and cache for following requests
	const QRectF bounds = d->elementBounds(element);
	buffer.clear();
	{
		QDataStream stream(&buffer, QIODevice::WriteOnly);
		stream << bounds;
	}
	d->m_cache->insert(key, buffer);
	d->m_boundsCache.insert(element, bounds);
	return bounds;
}

bool Tagaro::CachedProxyRenderBackend::elementExists(const QString& element) const
{
	//fast return if load() has not been called yet or if graphical source is invalid
	if (!d->m_valid)
	{
		return false;
	}
	//load backend if not loaded yet
	if (!d->m_backendLoaded)
	{
		d->m_backendLoaded = true;
		d->m_valid = d->m_backend->load();
	}
	//ask backend
	return d->m_valid ? d->m_backend->elementExists(element) : false;
}

QImage Tagaro::CachedProxyRenderBackend::Private::elementImage(const QString& element, const QSize& size, bool timeConstraint)
{
	if (!m_backendLoaded)
	{
		m_backendLoaded = true;
		m_valid = m_backend->load();
	}
	return m_valid ? m_backend->elementImage(element, size, timeConstraint) : QImage();
}

QImage Tagaro::CachedProxyRenderBackend::elementImage(const QString& element, const QSize& size, bool timeConstraint) const
{
	//fast return if load() has not been called yet or if graphical source is invalid
	if (!d->m_valid)
	{
		return QImage();
	}
	//the no-cache case
	if (!d->m_cache)
	{
		return d->elementImage(element, size, timeConstraint);
	}
	//check cache
	static const QString prefix = QLatin1String("%1-%2-");
	const QString key = prefix.arg(size.width()).arg(size.height()) + element;
	QImage result;
	if (d->m_cache->findImage(key, &result))
	{
		return result;
	}
	//render image and cache for the following requests
	result = d->elementImage(element, size, timeConstraint);
	if (d->m_cache && !result.isNull())
	{
		d->m_cache->insertImage(key, result);
	}
	return result;
}

void Tagaro::CachedProxyRenderBackend::insertIntoCache(const QString& element, const QSize& size, const QImage& image)
{
	if (image.size() == size && d->m_cache)
	{
		static const QString prefix = QLatin1String("%1-%2-");
		const QString key = prefix.arg(size.width()).arg(size.height()) + element;
		d->m_cache->insertImage(key, image);
	}
}

int Tagaro::CachedProxyRenderBackend::frameCount(const QString& element) const
{
	//fast return if load() has not been called yet or if graphical source is invalid
	if (!d->m_valid)
	{
		return -1;
	}
	//check fast cache
	QHash<QString, int>::const_iterator it = d->m_frameCountCache.constFind(element);
	if (it != d->m_frameCountCache.constEnd())
	{
		return it.value();
	}
	//if there's no slow cache...
	if (!d->m_cache)
	{
		const int count = Tagaro::RenderBackend::frameCount(element);
		d->m_frameCountCache.insert(element, count);
		return count;
	}
	//...else check slow cache
	const QString key = QLatin1String("fc-") + element;
	QByteArray buffer;
	if (d->m_cache->find(key, &buffer))
	{
		return buffer.toInt();
	}
	//ask backend and cache for following requests
	const int count = Tagaro::RenderBackend::frameCount(element);
	d->m_cache->insert(key, QByteArray::number(count));
	d->m_frameCountCache.insert(key, count);
	return count;
}

const Tagaro::RenderBackend* Tagaro::CachedProxyRenderBackend::proxiedBackend() const
{
	return d->m_backend;
}

Tagaro::RenderBackend* Tagaro::CachedProxyRenderBackend::proxiedBackend()
{
	return d->m_backend;
}

//END Tagaro::CachedProxyRenderBackend
