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

#ifndef TAGARO_OBJECTPOOL_H
#define TAGARO_OBJECTPOOL_H

#include <QtCore/QObject>

#include <libtagaro_export.h>

namespace Tagaro {

/**
 * @class Tagaro::ObjectPool objectpool.h <Tagaro/ObjectPool>
 *
 * Many classes are commonly instantiated on a global scope, e.g. as a
 * singleton. Examples include Tagaro::Renderer and Tagaro::ThemeProvider.
 *
 * Tagaro::ObjectPool makes instances of QObject subclasses available on an
 * application-global scope, manages their destruction, and provides through the
 * smart pointer class Tagaro::ObjectPointer a type-safe access method.
 *
 * For example, the following code can be added to main() to make a renderer
 * globally available:
 * @code
 * Tagaro::ObjectPool pool;
 * pool.insert("myrenderer", new Tagaro::Renderer(...));
 * @endcode
 * The renderer can then be fetched from the pool with the static get() method:
 * @code
 * QObject* rendererObj = Tagaro::ObjectPool::get("myrenderer");
 * Tagaro::Renderer* renderer = qobject_cast<Tagaro::Renderer*>(rendererObj);
 * renderer->...
 * @endcode
 * The casting is done automatically by the Tagaro::ObjectPointer class:
 * @code
 * Tagaro::Renderer* renderer = Tagaro::ObjectPointer("myrenderer");
 * renderer->...
 * @endcode
 * This becomes even shorter when using the T_OBJ typedef:
 * @code
 * Tagaro::Renderer* renderer = T_OBJ("myrenderer");
 * renderer->...
 * @endcode
 * Please note though that it is not possible to use Tagaro::Renderer methods
 * directly on T_OBJ(), because that is basically a QObject pointer with
 * implicit casting to QObject subclasses. You can, however, omit the exact type
 * when passing the pointer to a method. In this case, the argument type will
 * cast the ObjectPointer. For example:
 * @code
 * connect(T_OBJ("foo"), SIGNAL(valueChanged(int)), T_OBJ("bar"), SLOT(setValue(int)));
 * @endcode
 * If you want more type safety in such situations, you can use the as() method:
 * @code
 * connect(T_OBJ("foo").as<QSlider>(), SIGNAL(valueChanged(int)), T_OBJ("bar").as<QProgressBar>(), SLOT(setValue(int)));
 * @endcode
 */
class TAGARO_EXPORT ObjectPool
{
	public:
		///Creates a new Tagaro::ObjectPool instance.
		ObjectPool();
		///Destroys this Tagaro::ObjectPool instance, as well as all objects
		///which have been inserted into it.
		~ObjectPool();

		///Adds an @a object to this pool. The object is stored under the given
		///@a key. If another object was previously available under this key in
		///this pool, that object will be destroyed.
		///
		///@note If there are multiple pools, these may contain objects with the
		///      same key. The key is unique only in a single pool.
		void insert(const QByteArray& key, QObject* object);
		///Searches all existing object pools, and returns the first object
		///which is stored in a pool under the given @a key. If no such object
		///exists, returns a null pointer.
		///
		///The search order of the pools is determined by their instantiation
		///time: The newest pools are searched first. Objects in short-lived
		///pools can thus shadow objects with identical keys in long-living
		///pools.
		static QObject* get(const QByteArray& key);
	private:
		class Private;
		Private* const d;
};

// NOTE: Although the interface of the ObjectPool may look too limiting at
// first, it has been designed carefully to convey the semantics of the
// application-global scope.
// Any objects may be published by creating a new ObjectPool, and the duration
// of their global availability can be controlled by the lifetime of the pool.
// But the other side, the code which retrieves the objects from the pool, does
// not know anything about the hierarchy of the currently active ObjectPools.

/**
 * @class Tagaro::ObjectPointer objectpool.h <Tagaro/ObjectPointer>
 *
 * This smart pointer class extends the public interface of Tagaro::ObjectPool
 * to provide a type-safe access method to objects stored in object pools.
 *
 * @see Tagaro::ObjectPool for example code
 */
class TAGARO_EXPORT ObjectPointer
{
	public:
		///Creates a Tagaro::ObjectPointer instance by fetching the pointer to
		///the contained T instance from the global object pool. If no object
		///with the right @a key exists in the pool, or if this object exists,
		///but has the wrong type, this pointer will be null.
		///@see Tagaro::ObjectPool::get
		inline ObjectPointer(const QByteArray& key); //krazy:exclude=explicit

		///@return true if the pointer is not null
		inline operator bool() const;
		///@return the pointer encapsulated by this object, qobject_casted to T*
		template<typename T> inline operator T*() const;
		///A synonym for the implicit cast to T*.
		template<typename T> T* as() const;
	private:
		QObject* m_pointer;
};

} //namespace Tagaro

//See APIDOX for Tagaro::ObjectPointer.
typedef Tagaro::ObjectPointer T_OBJ;

//BEGIN implementation of Tagaro::ObjectPointer

Tagaro::ObjectPointer::ObjectPointer(const QByteArray& key)
	: m_pointer(Tagaro::ObjectPool::get(key))
{
}

Tagaro::ObjectPointer::operator bool() const
{
	return (bool) m_pointer;
}

template <typename T>
Tagaro::ObjectPointer::operator T*() const
{
	return qobject_cast<T*>(m_pointer);
}

template <typename T>
T* Tagaro::ObjectPointer::as() const
{
	return qobject_cast<T*>(m_pointer);
}

//END implementation of Tagaro::ObjectPointer

#endif // TAGARO_OBJECTPOOL_H
