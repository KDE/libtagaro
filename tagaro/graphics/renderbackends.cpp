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

#include "renderbackends.h"

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtGui/QPainter>
#include <QtSvg/QSvgRenderer>

//BEGIN Tagaro::QtSvgRenderBackend

struct Tagaro::QtSvgRenderBackend::Private
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

QSvgRenderer* Tagaro::QtSvgRenderBackend::Private::allocRenderer()
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

void Tagaro::QtSvgRenderBackend::Private::freeRenderer(QSvgRenderer* renderer)
{
	//mark renderer as available
	QMutexLocker locker(&m_mutex);
	m_hash.insert(renderer, 0);
}

Tagaro::QtSvgRenderBackend::QtSvgRenderBackend(const QString& path, const Tagaro::RenderBehavior& behavior)
	: Tagaro::RenderBackend(QFileInfo(path).absoluteFilePath(), behavior)
	, d(new Private(path))
{
}

Tagaro::QtSvgRenderBackend::~QtSvgRenderBackend()
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

bool Tagaro::QtSvgRenderBackend::load()
{
	if (!d->m_checked)
	{
		//instantiate a renderer to check validity
		d->freeRenderer(d->allocRenderer());
	}
	return !d->m_invalid;
}

uint Tagaro::QtSvgRenderBackend::lastModified() const
{
	return QFileInfo(d->m_path).lastModified().toTime_t();
}

QRectF Tagaro::QtSvgRenderBackend::elementBounds(const QString& element) const
{
	QSvgRenderer* r = d->allocRenderer();
	const QRectF result = r->boundsOnElement(element);
	d->freeRenderer(r);
	return result;
}

bool Tagaro::QtSvgRenderBackend::elementExists(const QString& element) const
{
	QSvgRenderer* r = d->allocRenderer();
	const bool result = r->elementExists(element);
	d->freeRenderer(r);
	return result;
}

QImage Tagaro::QtSvgRenderBackend::elementImage(const QString& element, const QSize& size, bool timeConstraint) const
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

//END Tagaro::QtSvgRenderBackend
