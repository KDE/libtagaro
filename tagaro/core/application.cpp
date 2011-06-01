/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>                     *
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

#include "application.h"

static Tagaro::Application* g_tapp = 0;

struct Tagaro::Application::Private
{
	QHash<QByteArray, QObject*> m_objects;

	void _t_objectDestroyed(QObject* object);
};

Tagaro::Application::Application(bool GUIenabled)
	: KApplication(GUIenabled)
	, d(new Private)
{
	g_tapp = this;
}

Tagaro::Application::~Application()
{
	//clean object pool (act on a local copy of d->m_objects because that one
	//is modified during the deletions)
	QHash<QByteArray, QObject*> objectsCopy(d->m_objects);
	qDeleteAll(objectsCopy);
	//delete myself
	g_tapp = 0;
	delete d;
}

/*static*/ Tagaro::Application* Tagaro::Application::instance()
{
	return g_tapp;
}

void Tagaro::Application::addObject(const QByteArray& key, QObject* object)
{
	//empty keys are forbidden
	if (key.isEmpty() || !object)
	{
		return;
	}
	//store object
	d->m_objects.insertMulti(key, object);
	connect(object, SIGNAL(destroyed(QObject*)), SLOT(_t_objectDestroyed(QObject*)));
}

void Tagaro::Application::Private::_t_objectDestroyed(QObject* object)
{
	//remove all occurrences of given object from object pool
	QMutableHashIterator<QByteArray, QObject*> it(m_objects);
	QList<QByteArray> affectedKeys;
	while (it.hasNext())
	{
		if (it.next().value() == object)
		{
			affectedKeys << it.key();
			it.remove();
		}
	}
}

QObject* Tagaro::Application::object(const QByteArray& key) const
{
	return d->m_objects.value(key, 0);
}

QHash<QByteArray, QObject*> Tagaro::Application::objects() const
{
	return d->m_objects;
}

#include "application.moc"
