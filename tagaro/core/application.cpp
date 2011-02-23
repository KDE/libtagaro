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

#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <KDE/KDebug>

static Tagaro::Application* g_tapp = 0;

struct Tagaro::Application::Private
{
	QHash<QByteArray, QObject*> m_objects;
	QDeclarativeEngine m_qmlEngine;

	void _t_objectDestroyed(QObject* object);

	Private(Tagaro::Application* parent) : m_qmlEngine(parent) {}
};

Tagaro::Application::Application(bool GUIenabled)
	: KApplication(GUIenabled)
	, d(new Private(this))
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
	//make QML aware of the new object
	d->m_qmlEngine.rootContext()->setContextProperty(QString::fromUtf8(key), object);
}

void Tagaro::Application::addObject(const QByteArray& key, const QString& file)
{
	QDeclarativeComponent component(&d->m_qmlEngine, QUrl::fromLocalFile(file));
	QObject* object = component.create();
	if (!component.isError())
	{
		addObject(key, object);
	}
	else
	{
		const QList<QDeclarativeError> errors = component.errors();
		kDebug() << errors.count() << "errors while instantiating object" << key << "from QML";
		//not just kDebug() << errors -> every error should be on its own line
		foreach (const QDeclarativeError& error, errors)
		{
			kDebug() << error;
		}
	}
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
	//make QML aware of changes
	if (!affectedKeys.isEmpty())
	{
		QDeclarativeContext* c = m_qmlEngine.rootContext();
		foreach (const QByteArray& key, affectedKeys)
		{
			c->setContextProperty(QString::fromUtf8(key), m_objects.value(key, 0));
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

QDeclarativeEngine* Tagaro::Application::qmlEngine() const
{
	return &d->m_qmlEngine;
}

#include "application.moc"
