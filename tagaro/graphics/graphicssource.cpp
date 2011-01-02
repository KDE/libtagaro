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

#include "graphicssource.h"
#include "graphicssourceconfig.h"
#include "settings.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringBuilder>
#include <QtCrypto/QtCrypto>
#include <KDE/KGlobal>
#include <KDE/KImageCache>
#include <KDE/KStandardDirs>

//BEGIN Tagaro::GraphicsSource

struct Tagaro::GraphicsSource::Private
{
	Tagaro::GraphicsSourceConfig m_config;
	QString m_identifier;
	bool m_valid, m_loaded;

	Private(const QString& identifier, const Tagaro::GraphicsSourceConfig& config) : m_config(config), m_identifier(identifier), m_loaded(false) {}
};

Tagaro::GraphicsSource::GraphicsSource(const QString& identifier, const Tagaro::GraphicsSourceConfig& config)
	: d(new Private(identifier, config))
{
}

Tagaro::GraphicsSource::~GraphicsSource()
{
	delete d;
}

const Tagaro::GraphicsSourceConfig& Tagaro::GraphicsSource::config() const
{
	return d->m_config;
}

QString Tagaro::GraphicsSource::identifier() const
{
	return d->m_identifier;
}

bool Tagaro::GraphicsSource::isValid() const
{
	//ensure that load() is only called once
	if (!d->m_loaded)
	{
		d->m_valid = const_cast<Tagaro::GraphicsSource*>(this)->load();
		d->m_loaded = true;
	}
	return d->m_valid;
}

bool Tagaro::GraphicsSource::load()
{
	return true;
}

void Tagaro::GraphicsSource::addConfiguration(const QMap<QString, QString>& configuration)
{
	Q_UNUSED(configuration)
}

uint Tagaro::GraphicsSource::lastModified() const
{
	//see documentation
	return 0;
}

QRectF Tagaro::GraphicsSource::elementBounds(const QString& element) const
{
	Q_UNUSED(element)
	return QRectF();
}

int Tagaro::GraphicsSource::frameCount(const QString& element) const
{
	//look for animated sprite first
	const int fbi = d->m_config.frameBaseIndex();
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

QString Tagaro::GraphicsSource::frameElementKey(const QString& element, int frame, bool useFrameCount) const
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
			const int fbi = d->m_config.frameBaseIndex();
			frame = (frame - fbi) % frameCount + fbi;
		}
	}
	return element + d->m_config.frameSuffix().arg(frame);
}

//END Tagaro::GraphicsSource
//BEGIN Tagaro::CachedProxyGraphicsSource

struct Tagaro::CachedProxyGraphicsSource::Private
{
	Tagaro::GraphicsSource* m_source;
	//disk cache
	KImageCache* m_cache;
	//in-process cache
	QHash<QString, QRectF> m_boundsCache;
	QHash<QString, int> m_frameCountCache;
	//state description
	bool m_valid, m_loaded, m_sourceLoaded, m_useCache;
	//m_useCache refers to the disk cache only, not to the in-process cache

	Private(Tagaro::GraphicsSource* source);

	QRectF elementBounds(const QString& element);
	QImage elementImage(const QString& element, const QSize& size, bool timeConstraint);
};

Tagaro::CachedProxyGraphicsSource::Private::Private(Tagaro::GraphicsSource* source)
	: m_source(source)
	, m_cache(0)
	, m_valid(false)
	, m_loaded(false)
	, m_sourceLoaded(false)
	, m_useCache(Tagaro::Settings::useDiskCache() && source->config().cacheSize() > 0)
{
}

Tagaro::CachedProxyGraphicsSource::CachedProxyGraphicsSource(Tagaro::GraphicsSource* source)
	: Tagaro::GraphicsSource(source->identifier(), source->config())
	, d(new Private(source))
{
}

Tagaro::CachedProxyGraphicsSource::~CachedProxyGraphicsSource()
{
	delete d->m_source;
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

bool Tagaro::CachedProxyGraphicsSource::load()
{
	if (d->m_loaded)
	{
		return d->m_valid;
	}
	d->m_loaded = true; //because we're doing it now
	//no caching -> just forward call to proxied source
	if (!d->m_useCache)
	{
		d->m_sourceLoaded = true;
		return d->m_valid = d->m_source->isValid();
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
		d->m_sourceLoaded = true;
		d->m_valid = d->m_source->isValid();
		if (!d->m_valid)
		{
			return d->m_valid;
		}
	}
	//open cache, check timestamp vs. last write access to graphics file
	//(shift by 20 converts megabytes to bytes)
	d->m_cache = new KImageCache(cacheName, config().cacheSize() << 20);
	d->m_cache->setPixmapCaching(false); //see comment below this method
	if (d->m_cache->timestamp() < d->m_source->lastModified())
	{
		d->m_cache->clear();
		kDebug() << "Theme newer than cache, checking graphics file immediately";
		d->m_sourceLoaded = true;
		d->m_valid = d->m_source->isValid();
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

QRectF Tagaro::CachedProxyGraphicsSource::Private::elementBounds(const QString& element)
{
	if (!m_sourceLoaded)
	{
		m_sourceLoaded = true;
		m_valid = m_source->isValid();
	}
	return m_valid ? m_source->elementBounds(element) : QRectF();
}

QRectF Tagaro::CachedProxyGraphicsSource::elementBounds(const QString& element) const
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
	//ask source and cache for following requests
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

bool Tagaro::CachedProxyGraphicsSource::elementExists(const QString& element) const
{
	//fast return if load() has not been called yet or if graphical source is invalid
	if (!d->m_valid)
	{
		return false;
	}
	//load source if not loaded yet
	if (!d->m_sourceLoaded)
	{
		d->m_sourceLoaded = true;
		d->m_valid = d->m_source->isValid();
	}
	//ask source
	return d->m_valid ? d->m_source->elementExists(element) : false;
}

QImage Tagaro::CachedProxyGraphicsSource::Private::elementImage(const QString& element, const QSize& size, bool timeConstraint)
{
	if (!m_sourceLoaded)
	{
		m_sourceLoaded = true;
		m_valid = m_source->isValid();
	}
	return m_valid ? m_source->elementImage(element, size, timeConstraint) : QImage();
}

QImage Tagaro::CachedProxyGraphicsSource::elementImage(const QString& element, const QSize& size, bool timeConstraint) const
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

int Tagaro::CachedProxyGraphicsSource::frameCount(const QString& element) const
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
		const int count = Tagaro::GraphicsSource::frameCount(element);
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
	//ask source and cache for following requests
	const int count = Tagaro::GraphicsSource::frameCount(element);
	d->m_cache->insert(key, QByteArray::number(count));
	d->m_frameCountCache.insert(key, count);
	return count;
}

//END Tagaro::CachedProxyGraphicsSource
