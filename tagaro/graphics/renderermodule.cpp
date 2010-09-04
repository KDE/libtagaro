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

#include "renderermodule.h"

#include <QtCore/QHash>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>
#include <QtSvg/QSvgRenderer>

//BEGIN Tagaro::RendererModule

struct Tagaro::RendererModule::Private
{
	//unused ATM
};

Tagaro::RendererModule::RendererModule()
	: d(0) //unused
{
}

Tagaro::RendererModule::~RendererModule()
{
	delete d;
}

bool Tagaro::RendererModule::isLoaded() const
{
	return true;
}

Tagaro::RendererModule* Tagaro::RendererModule::fromFile(const QString& file)
{
	//There are no other files at the moment.
	return new Tagaro::QSvgRendererModule(file);
}

//END Tagaro::RendererModule

//BEGIN Tagaro::QSvgRendererModule

struct Tagaro::QSvgRendererModule::Private
{
	Private(const QString& file) : m_file(file), m_checked(false), m_invalid(false) {}

	QString m_file;
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

Tagaro::QSvgRendererModule::QSvgRendererModule(const QString& file)
	: Tagaro::RendererModule()
	, d(new Private(file))
{
}

Tagaro::QSvgRendererModule::~QSvgRendererModule()
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

bool Tagaro::QSvgRendererModule::isValid() const
{
	if (!d->m_checked)
	{
		//instantiate a renderer to check validity
		d->freeRenderer(d->allocRenderer());
	}
	return !d->m_invalid;
}

QRectF Tagaro::QSvgRendererModule::boundsOnElement(const QString& element) const
{
	QSvgRenderer* r = d->allocRenderer();
	const QRectF result = r->boundsOnElement(element);
	d->freeRenderer(r);
	return result;
}

bool Tagaro::QSvgRendererModule::elementExists(const QString& element) const
{
	QSvgRenderer* r = d->allocRenderer();
	const bool result = r->elementExists(element);
	d->freeRenderer(r);
	return result;
}

void Tagaro::QSvgRendererModule::render(QPainter* painter, const QString& element, const QRectF& bounds)
{
	QSvgRenderer* r = d->allocRenderer();
	r->render(painter, element, bounds);
	d->freeRenderer(r);
}

QSvgRenderer* Tagaro::QSvgRendererModule::Private::allocRenderer()
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
		renderer = new QSvgRenderer(m_file);
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

void Tagaro::QSvgRendererModule::Private::freeRenderer(QSvgRenderer* renderer)
{
	//mark renderer as available
	QMutexLocker locker(&m_mutex);
	m_hash.insert(renderer, 0);
}

//END Tagaro::QSvgRendererModule
