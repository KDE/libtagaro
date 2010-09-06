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
 * This becomes even simpler when using the Tagaro::ObjectPointer class:
 * @code
 * Tagaro::ObjectPointer<Tagaro::Renderer> renderer("myrenderer");
 * renderer->...
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
 *
 * As usual for smart pointer classes, this class can always be used when a
 * pointer to T is expected. Because its constructors can be used implicitly,
 * when a method takes an ObjectPointer argument, you can give a QByteArray key
 * to tell the method to fetch the object from the object pool.
 *
 * For more advise on how to use smart pointers, see e.g. the documentation for
 * the QScopedPointer class from QtCore.
 */
template<typename T> class TAGARO_EXPORT ObjectPointer
{
	public:
		///Creates a Tagaro::ObjectPointer instance by fetching the pointer to
		///the contained T instance from the global object pool. If no object
		///with the right @a key exists in the pool, or if this object exists,
		///but has the wrong type, this pointer will be null.
		///@see Tagaro::ObjectPool::get
		inline ObjectPointer(const QByteArray& key); //krazy:exclude=explicit
		///Creates a Tagaro::ObjectPointer instance and sets its pointer to @a p.
		inline ObjectPointer(T* p); //krazy:exclude=explicit

		///@return true if the pointer is not null
		inline operator bool() const;
		///@return the pointer encapsulated by this object
		inline operator T*() const;
		///Provides access to the pointer's object.
		inline T& operator*() const;
		///Provides access to the pointer's object.
		inline T* operator->() const;
	private:
		T* m_pointer;
};

//BEGIN typedefs for commonly used ObjectPointers

#define DEFINE_TYPEDEF(CLASS) \
	class CLASS; \
	typedef Tagaro::ObjectPointer<Tagaro::CLASS> CLASS##Ptr;

DEFINE_TYPEDEF(Renderer)

#undef DEFINE_TYPEDEF

//END typedefs for commonly used ObjectPointers

} //namespace Tagaro

//BEGIN implementation of Tagaro::ObjectPointer

template <typename T>
Tagaro::ObjectPointer<T>::ObjectPointer(const QByteArray& key)
	: m_pointer(qobject_cast<T*>(Tagaro::ObjectPool::get(key)))
{
}

template <typename T>
Tagaro::ObjectPointer<T>::ObjectPointer(T* p)
	: m_pointer(p)
{
}

template <typename T>
Tagaro::ObjectPointer<T>::operator bool() const
{
	return (bool) m_pointer;
}

template <typename T>
Tagaro::ObjectPointer<T>::operator T*() const
{
	return m_pointer;
}

template <typename T>
T& Tagaro::ObjectPointer<T>::operator*() const
{
	return *m_pointer;
}

template <typename T>
T* Tagaro::ObjectPointer<T>::operator->() const
{
	return m_pointer;
}

//END implementation of Tagaro::ObjectPointer

#endif // TAGARO_OBJECTPOOL_H
