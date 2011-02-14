/***************************************************************************
 *   Copyright 2010-2011 Stefan Majewsky <majewsky@gmx.net>                *
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

#ifndef TAGARO_APPLICATION_H
#define TAGARO_APPLICATION_H

#include <KDE/KApplication>

#include <libtagaro_export.h>

namespace Tagaro {

/**
 * @class Tagaro::Application application.h <Tagaro/Application>
 *
 * This KApplication provides additional services specific to Tagaro games:
 * @li an application-global object pool
 *
 * From the restriction to one KApplication instance per process arises an
 * identical limit for Tagaro::Application. The single instance is available
 * via the global variable tapp (similar to kapp and qApp).
 *
 * @sa tObj()
 */
class TAGARO_EXPORT Application : public KApplication
{
	Q_OBJECT
	public:
		///This constructor is identical to the one provided by KApplication.
		Application(bool GUIenabled = true);
		///Destroys the Tagaro::Application instance and everything in its
		///object pool (in no particular order).
		virtual ~Application();
		///@return the Tagaro::Application instance (there may only be one at
		///once) or 0 if none has been instantiated yet
		static Tagaro::Application* instance();

		///Registers an @a object with the application under the name @a key.
		///
		///The application takes ownership of the @a object, though this is not
		///reflected by setting the @a object's QObject::parent explicitly. When
		///the application is destroyed, it cleans up all objects it knows
		///about (in no particular order!).
		///
		///However, it is also allowed to delete objects which are registered
		///with the application. The latter will update is object storage
		///automatically.
		///
		///It is explicitly allowed to insert multiple objects into the same
		///@a key. Only the most recently inserted one will be retrieved by the
		///object() method then. Using this mechanism, long-living objects may
		///be "shadowed" by short-lived objects which use the same key.
		///
		///Empty keys are forbidden.
		void addObject(const QByteArray& key, QObject* object);
		///@return the object registered with the application under the given
		///        @a key (or 0 if this key is not known)
		///
		///@see tObj() for a short-hand
		QObject* object(const QByteArray& key) const;
		///@return all objects registered with the application
		QHash<QByteArray, QObject*> objects() const;
	private:
		class Private;
		Private* const d;
		Q_PRIVATE_SLOT(d, void _t_objectDestroyed(QObject*));
};

/**
 * @class Tagaro::ObjectPointer application.h <Tagaro/ObjectPointer>
 *
 * This smart pointer class extends the public interface of Tagaro::Application
 * to provide a type-safe access method to objects stored in its object pool.
 *
 * Tagaro::Application's object pool makes instances of QObject subclasses
 * available on an application-global scope, manages their destruction, and
 * provides a type-safe access method through this smart pointer class.
 *
 * For example, the following code makes a theme provider globally available:
 * @code
 * app.addObject("myprovider", new Tagaro::StandardThemeProvider(...));
 * @endcode
 * The provider can then be fetched from the pool with the object() method:
 * @code
 * QObject* providerObj = Tagaro::Application::instance()->object("myprovider");
 * Tagaro::ThemeProvider* provider = qobject_cast<Tagaro::ThemeProvider*>(providerObj);
 * provider->...
 * @endcode
 * The casting is done automatically by the Tagaro::ObjectPointer class:
 * @code
 * Tagaro::ThemeProvider* provider = Tagaro::ObjectPointer("myprovider");
 * provider->...
 * @endcode
 * This becomes even shorter when using the global tObj() method:
 * @code
 * Tagaro::ThemeProvider* provider = tObj("myprovider");
 * provider->...
 * @endcode
 * tObj() without template arguments is just a short-hand for the
 * Tagaro::ObjectPointer constructor. There is also an overload with a template
 * argument, that does all the casting directly:
 * @code
 * tObj<Tagaro::ThemeProvider>("myprovider")->...
 * @endcode
 * Tagaro::ObjectPointer is basically a QObject pointer with implicit casting to
 * QObject subclasses. Specification of the exact type is not needed (and
 * discouraged if you strive for good performance) when only a QObject pointer
 * is needed. For example:
 * @code
 * connect(tObj("foo"), SIGNAL(valueChanged(int)), tObj("bar"), SLOT(setValue(int)));
 * @endcode
 *
 * @warning Usages of Tagaro::ObjectPointer and tObj() will crash before a
 *          Tagaro::Application instance has been created (or after it has been
 *          destroyed).
 */
class TAGARO_EXPORT ObjectPointer //krazy:exclude=dpointer
{
	public:
		///Creates a Tagaro::ObjectPointer instance by fetching the pointer to
		///the contained object from the Tagaro::Application object pool. If no
		///object with the right @a key exists in the pool, this pointer will
		///be null.
		///@see Tagaro::Application::object()
		inline ObjectPointer(const QByteArray& key); //krazy:exclude=explicit,inline

		///@return true if the pointer is not null
		inline operator bool() const;
		///@return the pointer encapsulated by this object, qobject_casted to T*
		template<typename T> inline operator T*() const;
	private:
		QObject* m_pointer;
};

} //namespace Tagaro

//See APIDOX for Tagaro::ObjectPointer.
typedef Tagaro::ObjectPointer T_OBJ;

//BEGIN implementation of Tagaro::ObjectPointer

Tagaro::ObjectPointer::ObjectPointer(const QByteArray& key)
	: m_pointer(Tagaro::Application::instance()->object(key))
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

//END implementation of Tagaro::ObjectPointer

///@return the object registered with the Tagaro::Application under the given
///        @a key
///
///@see Tagaro::ObjectPointer class documentation for detailed description
template<typename T> inline T* tObj(const QByteArray& key)
{
	return Tagaro::ObjectPointer(key);
}

///@overload
inline Tagaro::ObjectPointer tObj(const QByteArray& key)
{
	return Tagaro::ObjectPointer(key);
}

#endif // TAGARO_APPLICATION_H
