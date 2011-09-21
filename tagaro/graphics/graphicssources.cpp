/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>                     *
 *   Copyright 2011 Jeffrey Kelling <kelling.jeffrey@ages-skripte.org>     *
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

#include "graphicssources.h"

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtGui/QPainter>
#include <QtSvg/QSvgRenderer>
#include <KDE/KDebug> //for kWarning
#include <KDE/KFilterDev>

static QByteArray readSVG(const QString& path)
{
	QIODevice* dev;
	if(path.endsWith(QLatin1String(".svgz"), Qt::CaseInsensitive) || path.endsWith(QLatin1String(".svg.gz"), Qt::CaseInsensitive))
	{
		dev = KFilterDev::deviceForFile(path, QLatin1String("application/x-gzip"));
	}
	else
	{
		dev = new QFile(path);
	}
	QByteArray file;
	if(!dev->open(QIODevice::ReadOnly))
	{
		kWarning() << "could not open file" << path;
		return QByteArray();
	}
	else
	{
		file = dev->readAll();
	}
	delete dev;
	return file;
}

//BEGIN KGame::QtSvgGraphicsSource

struct KGame::QtSvgGraphicsSource::Private
{
	Private(const QByteArray& svgData = QByteArray(), const QString& path = QString())
		: m_svgData(svgData), m_path(path), m_checked(false), m_invalid(false) {}

	QByteArray m_svgData;
	QString m_path;
	bool m_checked, m_invalid;

	//This class uses a pool of renderer instances to implement the required
	//thread-safety. Access this only with the two helper functions below!
	QHash<QSvgRenderer*, QThread*> m_hash;
	QMutex m_mutex;

	//Returns a SVG renderer instance that can be used in the calling thread.
	inline QSvgRenderer* allocRenderer();
	//Marks this renderer as available for allocation by other threads.
	inline void freeRenderer(QSvgRenderer* renderer);
};

QSvgRenderer* KGame::QtSvgGraphicsSource::Private::allocRenderer()
{
	//quick check: was the file found to be invalid already?
	if (m_checked && m_invalid)
	{
		return 0;
	}
	//look for an available renderer
	QThread* thread = QThread::currentThread();
	QMutexLocker locker(&m_mutex);
	QSvgRenderer* renderer = m_hash.key(0);
	if (!renderer)
	{
		//instantiate a new renderer
		renderer = new QSvgRenderer(m_svgData);
		if (!m_checked)
		{
			m_checked = true;
			m_invalid = !renderer->isValid();
		}
	}
	//mark renderer as used
	m_hash.insert(renderer, thread);
	return m_invalid ? 0 : renderer;
}

void KGame::QtSvgGraphicsSource::Private::freeRenderer(QSvgRenderer* renderer)
{
	//mark renderer as available
	QMutexLocker locker(&m_mutex);
	m_hash.insert(renderer, 0);
}

KGame::QtSvgGraphicsSource::QtSvgGraphicsSource(const QString& path, const KGame::GraphicsSourceConfig& config)
	: KGame::GraphicsSource(QFileInfo(path).absoluteFilePath(), config)
	, d(new Private(readSVG(path), path))
{
}

KGame::QtSvgGraphicsSource::QtSvgGraphicsSource(const QByteArray& svgData, const KGame::GraphicsSourceConfig& config)
	: KGame::GraphicsSource(QString(), config)
	, d(new Private(svgData))
{
}

KGame::QtSvgGraphicsSource::~QtSvgGraphicsSource()
{
	{
		QMutexLocker l(&d->m_mutex);
		QHash<QSvgRenderer*, QThread*>::const_iterator it1 = d->m_hash.constBegin(), it2 = d->m_hash.constEnd();
		for (; it1 != it2; ++it1)
		{
			Q_ASSERT(it1.value() == 0); //nobody may be using our renderers at this point
			delete it1.key();
		}
	} //unlock d->m_mutex before deleting it
	delete d;
}

bool KGame::QtSvgGraphicsSource::load()
{
	if (!d->m_checked)
	{
		//instantiate a renderer to check validity
		d->freeRenderer(d->allocRenderer());
	}
	return !d->m_invalid;
}

uint KGame::QtSvgGraphicsSource::lastModified() const
{
	return QFileInfo(d->m_path).lastModified().toTime_t();
}

QRectF KGame::QtSvgGraphicsSource::elementBounds(const QString& element) const
{
	QSvgRenderer* r = d->allocRenderer();
	if (!r)
	{
		return QRectF();
	}
	const QRectF result = r->boundsOnElement(element);
	d->freeRenderer(r);
	return result;
}

bool KGame::QtSvgGraphicsSource::elementExists(const QString& element) const
{
	QSvgRenderer* r = d->allocRenderer();
	if (!r)
	{
		return false;
	}
	const bool result = r->elementExists(element);
	d->freeRenderer(r);
	return result;
}

QImage KGame::QtSvgGraphicsSource::elementImage(const QString& element, const QSize& size, const QString& processingInstruction, bool timeConstraint) const
{
	Q_UNUSED(processingInstruction) //does not define any processing instructions
	if (timeConstraint)
	{
		return QImage();
	}
	//rendering SVG elements is always time-consuming ;-)
	QImage image(size, QImage::Format_ARGB32_Premultiplied);
	image.fill(QColor(Qt::transparent).rgba());
	QPainter painter(&image);
	QSvgRenderer* r = d->allocRenderer();
	if (!r)
	{
		return QImage();
	}
	r->render(&painter, element);
	d->freeRenderer(r);
	painter.end();
	return image;
}

//END KGame::QtSvgGraphicsSource
//BEGIN KGame::QtColoredSvgGraphicsSource

struct KGame::QtColoredSvgGraphicsSource::Private
{
	Private(const QByteArray& svgData, const QString& path)
		: m_svgData(svgData), m_path(path), m_colorkey(QLatin1String("#ff8989")) {}
	~Private();

	QByteArray m_svgData;
	QString m_path, m_colorkey;

	//This class uses a pool of QtSvgGraphicsSource, one for each color
	mutable QHash<QString, QtSvgGraphicsSource*> m_hash;
};

KGame::QtColoredSvgGraphicsSource::Private::~Private()
{
	qDeleteAll(m_hash);
}

KGame::QtColoredSvgGraphicsSource::QtColoredSvgGraphicsSource(const QString& path, const KGame::GraphicsSourceConfig& config)
	: GraphicsSource(QFileInfo(path).absoluteFilePath(), config), d(new Private(readSVG(path), path))
{
	d->m_hash[QString()] = new QtSvgGraphicsSource(d->m_svgData, config);
}

KGame::QtColoredSvgGraphicsSource::~QtColoredSvgGraphicsSource()
{
	delete d;
}

void KGame::QtColoredSvgGraphicsSource::addConfiguration(const QMap<QString, QString>& configuration)
{
	QMap<QString, QString>::const_iterator a = configuration.constFind(QLatin1String("ColorKey"));
	if(a == configuration.end())
	{
		return;
	}
	QColor c(a.value());
	if(!c.isValid())
	{
		kWarning() << "invalid ColorKey" << a.value();
	}
	else
	{
		d->m_colorkey = c.name(); //be shure to use the #.. notation
	}
}

uint KGame::QtColoredSvgGraphicsSource::lastModified() const
{
	return QFileInfo(d->m_path).lastModified().toTime_t();
}

QRectF KGame::QtColoredSvgGraphicsSource::elementBounds(const QString& element) const
{
	return d->m_hash.begin().value()->elementBounds(element);
}

bool KGame::QtColoredSvgGraphicsSource::elementExists(const QString& element) const
{
	return d->m_hash.begin().value()->elementExists(element);
}

QImage KGame::QtColoredSvgGraphicsSource::elementImage(const QString& element, const QSize& size, const QString& processingInstruction, bool timeConstraint) const
{
	if(processingInstruction.isEmpty())
	{
		return d->m_hash[processingInstruction]->elementImage(element, size, QString(), timeConstraint);
	}
	QColor c(processingInstruction);
	if(!c.isValid())
	{
		kWarning() << "invalid color" << processingInstruction;
		QImage image(size, QImage::Format_ARGB32_Premultiplied);
		image.fill(QColor(Qt::transparent).rgba());
		return image;
	}
	QtSvgGraphicsSource*& r = d->m_hash[c.name()]; //be shure to use the #.. notation
	if(!r)
	{
		QByteArray s = d->m_svgData;
		s.replace(d->m_colorkey.toUtf8(), processingInstruction.toUtf8());
		r = new QtSvgGraphicsSource(s, config());
	}
	return r->elementImage(element, size, QString(), timeConstraint);
}

//END KGame::QtColoredSvgGraphicsSource
//BEGIN KGame::ColorGraphicsSource

struct KGame::ColorGraphicsSource::Private
{
	//reserved for later use
};

KGame::ColorGraphicsSource::ColorGraphicsSource(const KGame::GraphicsSourceConfig& config)
	: KGame::GraphicsSource("color", config)
	, d(0)
{
}

KGame::ColorGraphicsSource::~ColorGraphicsSource()
{
	delete d;
}

bool KGame::ColorGraphicsSource::elementExists(const QString& element) const
{
	return QColor::isValidColor(element);
}

QImage KGame::ColorGraphicsSource::elementImage(const QString& element, const QSize& size, const QString& processingInstruction, bool timeConstraint) const
{
	Q_UNUSED(processingInstruction) //does not define any processing instructions
	Q_UNUSED(timeConstraint) //constructing plain color images is not expensive (compared to setting up a renderer thread)
	QColor color = QColor::isValidColor(element) ? QColor(element) : QColor(Qt::transparent);
	QImage image(size, QImage::Format_ARGB32_Premultiplied);
	image.fill(color.rgba());
	return image;
}

//END KGame::ColorGraphicsSource
//BEGIN KGame::ImageGraphicsSource

struct KGame::ImageGraphicsSource::Private
{
	QString m_path;
	QImage m_image;
	QHash<QString, QRect> m_elements;

	Private(const QString& path) : m_path(path) {}
};

KGame::ImageGraphicsSource::ImageGraphicsSource(const QString& path, const KGame::GraphicsSourceConfig& config)
	: KGame::GraphicsSource(QFileInfo(path).absoluteFilePath(), config)
	, d(new Private(path))
{
}

KGame::ImageGraphicsSource::~ImageGraphicsSource()
{
	delete d;
}

void KGame::ImageGraphicsSource::addConfiguration(const QMap<QString, QString>& configuration)
{
	const QRegExp rx("(\\d+)x(\\d+)\\+(\\d+)\\+(\\d+)");
	QMap<QString, QString>::const_iterator it1 = configuration.constBegin(), it2 = configuration.constEnd();
	for (; it1 != it2; ++it1)
	{
		//parse location specification; format: WIDTHxHEIGHT+XOFF+YOFF
		if (rx.exactMatch(it1.value()))
		{
			const QRect rect(rx.cap(3).toInt(), rx.cap(4).toInt(), rx.cap(1).toInt(), rx.cap(2).toInt());
			d->m_elements.insert(it1.key(), rect);
		}
	}
}

bool KGame::ImageGraphicsSource::load()
{
	if (!d->m_image.load(d->m_path))
	{
		return false;
	}
	d->m_elements.insert(QLatin1String("full"), d->m_image.rect());
	return true;
}

QRectF KGame::ImageGraphicsSource::elementBounds(const QString& element) const
{
	return d->m_elements.value(element);
}

bool KGame::ImageGraphicsSource::elementExists(const QString& element) const
{
	return d->m_elements.contains(element);
}

QImage KGame::ImageGraphicsSource::elementImage(const QString& element, const QSize& size, const QString& processingInstruction, bool timeConstraint) const
{
	Q_UNUSED(processingInstruction) //does not define any processing instructions
	Q_UNUSED(timeConstraint) //simple copying of images is not expensive (compared to setting up a renderer thread)
	QImage image(size, QImage::Format_ARGB32_Premultiplied);
	image.fill(QColor(Qt::transparent).rgba());
	QHash<QString, QRect>::const_iterator it = d->m_elements.constFind(element);
	if (it == d->m_elements.constEnd())
	{
		//unknown element -> return empty image
		return image;
	}
	//copy the part of the image which is specified by this element
	QPainter p(&image);
	p.drawImage(image.rect(), d->m_image, it.value());
	p.end();
	return image;
}

//END KGame::ImageGraphicsSource
