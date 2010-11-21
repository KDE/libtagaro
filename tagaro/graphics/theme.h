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

#ifndef TAGARO_THEME_H
#define TAGARO_THEME_H

#include <QtCore/QVariant>

#include <libtagaro_export.h>

namespace Tagaro {

/**
 * @class Tagaro::Theme theme.h <Tagaro/Theme>
 *
 * A theme is an entity which essentially represents an SVG file plus some
 * metadata. Tagaro::Theme instances are usually created and managed by a
 * Tagaro::ThemeProvider.
 */
class TAGARO_EXPORT Theme
{
	Q_DISABLE_COPY(Theme)
	public:
		///Each theme has a set of data elements associated with it, each with
		///its own role. The data in given roles is used by user interfaces to
		///display information about the themes, or to determine the files
		///associated with this theme.
		///@see setData(), data()
		///
		///The roles are aligned with Qt::ItemDataRole because of
		///Tagaro::ThemeProvider::model(). Themes behave similar to
		///QStandardItems in this regard.
		enum Roles
		{
			///The name of this theme.
			NameRole = Qt::DisplayRole,
			///(optional) A preview image showing the visual appearance of the
			///theme.
			PreviewRole = Qt::DecorationRole,
			///This role *must* be filled with the path of the SVG file which is
			///represented by this theme. If this role is empty or the data is
			///invalid, loading the theme into a Tagaro::Renderer will fail.
			GraphicsFileRole = Qt::UserRole,
			///If the data in this Tagaro::Theme instance was read from a file,
			///this role may contain the path to that file.
			ThemeFileRole,
			///An additional description which goes beyond the Name.
			DescriptionRole,
			///(optional) The name of this theme's author.
			AuthorRole,
			///(optional) The email address of this theme's author.
			AuthorEmailRole
		};

		///Creates a new Tagaro::Theme with the given @a identifier. The
		///identifier must be application-unique.
		///@warning The @a identifier is sometimes used as part of a file path,
		///and should therefore not contain any characters which are not allowed
		///in file paths. (The slash, or backslash on Windows, is allowed.)
		Theme(const QByteArray& identifier);
		///Destroys this Tagaro::Theme instance.
		~Theme();

		///@return the data stored unter the given @a role, or the given
		///@a defaultValue if no data is stored
		///@see Tagaro::Theme::Roles
		QVariant data(int role, const QVariant& defaultValue = QVariant()) const;
		///@overload for custom data
		QVariant data(const QByteArray& key, const QVariant& defaultValue = QVariant()) const;
		///Stores the given @a value unter the given @a role.
		///@see Tagaro::Theme::Roles
		void setData(int role, const QVariant& value);
		///@overload for custom data
		void setData(const QByteArray& key, const QVariant& value);
		///@return the internal identifier for this theme
		QByteArray identifier() const;

		///@return the time (as a UNIX timestamp) when the SVG file and other
		///files associated with this time have been modified last
		uint modificationTimestamp() const;
	private:
		class Private;
		Private* const d;
};

/**
 * @class Tagaro::StandardTheme theme.h <Tagaro/StandardTheme>
 *
 * This subclass implements loading of themes which are stored in a format
 * loosely based on the freedesktop.org Desktop File Specification.
 *
 * Also, this class provides legacy support for KGameTheme.
 */
//TODO: provide legacy support for KMahjonggBackground, KMahjonggTileset
class TAGARO_EXPORT StandardTheme : public Tagaro::Theme
{
	public:
		///Creates a new Tagaro::StandardTheme instance by reading in the
		///desktop file at the given @a filePath.
		StandardTheme(const QString& filePath);
		///Creates a new Tagaro::StandardTheme instance by reading in the
		///desktop file at
		///@code
		///KStandardDirs::locate(ksdResource, ksdDirectory + fileName);
		///@endcode
		///The important difference to the single-argument constructor is that
		///references to graphics files are also resolved via
		///KStandardDirs::locate rather than just looking in the same directory.
		StandardTheme(const QByteArray& ksdResource, const QString& ksdDirectory, const QString& fileName);
	private:
		class Private;
		Private* const d;
};

} //namespace Tagaro

#endif // TAGARO_THEME_H
