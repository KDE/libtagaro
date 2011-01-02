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

#include "graphicssources.h"

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtGui/QPainter>
#include <QtSvg/QSvgRenderer>

//BEGIN Tagaro::QtSvgGraphicsSource

struct Tagaro::QtSvgGraphicsSource::Private
{
	Private(const QString& path) : m_path(path), m_checked(false), m_invalid(false) {}

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

QSvgRenderer* Tagaro::QtSvgGraphicsSource::Private::allocRenderer()
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
		renderer = new QSvgRenderer(m_path);
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

void Tagaro::QtSvgGraphicsSource::Private::freeRenderer(QSvgRenderer* renderer)
{
	//mark renderer as available
	QMutexLocker locker(&m_mutex);
	m_hash.insert(renderer, 0);
}

Tagaro::QtSvgGraphicsSource::QtSvgGraphicsSource(const QString& path, const Tagaro::GraphicsSourceConfig& config)
	: Tagaro::GraphicsSource(QFileInfo(path).absoluteFilePath(), config)
	, d(new Private(path))
{
}

Tagaro::QtSvgGraphicsSource::~QtSvgGraphicsSource()
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

bool Tagaro::QtSvgGraphicsSource::load()
{
	if (!d->m_checked)
	{
		//instantiate a renderer to check validity
		d->freeRenderer(d->allocRenderer());
	}
	return !d->m_invalid;
}

uint Tagaro::QtSvgGraphicsSource::lastModified() const
{
	return QFileInfo(d->m_path).lastModified().toTime_t();
}

QRectF Tagaro::QtSvgGraphicsSource::elementBounds(const QString& element) const
{
	QSvgRenderer* r = d->allocRenderer();
	const QRectF result = r->boundsOnElement(element);
	d->freeRenderer(r);
	return result;
}

bool Tagaro::QtSvgGraphicsSource::elementExists(const QString& element) const
{
	QSvgRenderer* r = d->allocRenderer();
	const bool result = r->elementExists(element);
	d->freeRenderer(r);
	return result;
}

QImage Tagaro::QtSvgGraphicsSource::elementImage(const QString& element, const QSize& size, bool timeConstraint) const
{
	if (timeConstraint)
	{
		return QImage();
	}
	//rendering SVG elements is always time-consuming ;-)
	QImage image(size, QImage::Format_ARGB32_Premultiplied);
	image.fill(QColor(Qt::transparent).rgba());
	QPainter painter(&image);
	QSvgRenderer* r = d->allocRenderer();
	r->render(&painter, element);
	d->freeRenderer(r);
	painter.end();
	return image;
}

//END Tagaro::QtSvgGraphicsSource
//BEGIN Tagaro::ColorGraphicsSource

struct Tagaro::ColorGraphicsSource::Private
{
	//reserved for later use
};

Tagaro::ColorGraphicsSource::ColorGraphicsSource(const Tagaro::GraphicsSourceConfig& config)
	: Tagaro::GraphicsSource("color", config)
	, d(0)
{
}

Tagaro::ColorGraphicsSource::~ColorGraphicsSource()
{
	delete d;
}

bool Tagaro::ColorGraphicsSource::elementExists(const QString& element) const
{
	return QColor::isValidColor(element);
}

QImage Tagaro::ColorGraphicsSource::elementImage(const QString& element, const QSize& size, bool timeConstraint) const
{
	Q_UNUSED(timeConstraint) //constructing plain color images is not expensive (compared to setting up a renderer thread)
	QColor color = QColor::isValidColor(element) ? QColor(element) : QColor(Qt::transparent);
	QImage image(size, QImage::Format_ARGB32_Premultiplied);
	image.fill(color.rgba());
	return image;
}

//END Tagaro::ColorGraphicsSource
//BEGIN Tagaro::ImageGraphicsSource

struct Tagaro::ImageGraphicsSource::Private
{
	QString m_path;
	QImage m_image;
	QHash<QString, QRect> m_elements;

	Private(const QString& path) : m_path(path) {}
};

Tagaro::ImageGraphicsSource::ImageGraphicsSource(const QString& path, const Tagaro::GraphicsSourceConfig& config)
	: Tagaro::GraphicsSource(QFileInfo(path).absoluteFilePath(), config)
	, d(new Private(path))
{
}

Tagaro::ImageGraphicsSource::~ImageGraphicsSource()
{
	delete d;
}

void Tagaro::ImageGraphicsSource::addConfiguration(const QMap<QString, QString>& configuration)
{
	//TODO: Tagaro::ImageGraphicsSource::addConfiguration
}

bool Tagaro::ImageGraphicsSource::load()
{
	if (!d->m_image.load(d->m_path))
	{
		return false;
	}
	d->m_elements.insert(QLatin1String("full"), d->m_image.rect());
	return true;
}

QRectF Tagaro::ImageGraphicsSource::elementBounds(const QString& element) const
{
	return d->m_elements.value(element);
}

bool Tagaro::ImageGraphicsSource::elementExists(const QString& element) const
{
	return d->m_elements.contains(element);
}

QImage Tagaro::ImageGraphicsSource::elementImage(const QString& element, const QSize& size, bool timeConstraint) const
{
	Q_UNUSED(timeConstraint) //simple copying of images is not expensive (compared to setting up a renderer thread)
	QImage image(size, QImage::Format_ARGB32_Premultiplied);
	QHash<QString, QRect>::const_iterator it = d->m_elements.constFind(element);
	if (it == d->m_elements.constEnd())
	{
		//unknown element -> return empty image
		image.fill(QColor(Qt::transparent).rgba());
		return image;
	}
	//copy the part of the image which is specified by this element
	QPainter p(&image);
	p.drawImage(image.rect(), d->m_image, it.value());
	p.end();
	return image;
}

//END Tagaro::ImageGraphicsSource
