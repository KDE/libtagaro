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

#include "objectpool.h"

#include <QtCore/QHash>
#include <KDE/KGlobal>

namespace Tagaro
{
	struct ObjectPoolList : public QList<Tagaro::ObjectPool*>
	{
		~ObjectPoolList() { qDeleteAll(*this); }
	};
}

K_GLOBAL_STATIC(Tagaro::ObjectPoolList, g_pools)

struct Tagaro::ObjectPool::Private : public QHash<QByteArray, QObject*>
{
};

Tagaro::ObjectPool::ObjectPool()
	: d(new Private)
{
	g_pools->prepend(this);
}

Tagaro::ObjectPool::~ObjectPool()
{
	g_pools->removeAll(this);
	qDeleteAll(*d);
	delete d;
}

void Tagaro::ObjectPool::insert(const QByteArray& key, QObject* object)
{
	QObject*& entry = (*d)[key];
	delete entry;
	entry = object;
}

QObject* Tagaro::ObjectPool::get(const QByteArray& key)
{
	//new pools are prepended, so forward iteration searches "backwards in time" as desired
	QList<Tagaro::ObjectPool*>::const_iterator it1 = g_pools->constBegin(),
	                                           it2 = g_pools->constEnd();
	for (; it1 != it2; ++it1)
	{
		QObject* obj = (*it1)->d->value(key);
		if (obj)
		{
			return obj;
		}
	}
	return 0;
}
