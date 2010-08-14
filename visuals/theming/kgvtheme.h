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

#ifndef KGVTHEME_H
#define KGVTHEME_H

#include <QtCore/QVariant>

#include <libkgame_export.h>

/**
 * @class KgvTheme kgvtheme.h <KgvTheme>
 *
 * A theme is an entity which essentially represents an SVG file plus some
 * metadata. KgvTheme instances are usually created and managed by a 
 * KgvThemeProvider.
 */
class KGAMEVISUALS_EXPORT KgvTheme
{
	Q_DISABLE_COPY(KgvTheme)
	public:
		///Each theme has a set of data elements associated with it, each with
		///its own role. The data in given roles is used by user interfaces to
		///display information about the themes, or to determine the files
		///associated with this theme.
		///@see setData(), data()
		enum Role
		{
			///This role *must* be filled with the path of the SVG file which is
			///represented by this theme. If this role is empty or contains
			///invalid data, loading the theme into a KgvRenderer will fail.
			GraphicsFileRole,
			///If the data in this KgvTheme instance was read from a file, this
			///role may contain the path to that file.
			ThemeFileRole,
			///The name of this theme.
			NameRole,
			///An additional description which goes beyond the Name.
			DescriptionRole,
			///(optional) The name of this theme's author.
			AuthorRole,
			///(optional) The email address of this theme's author.
			AuthorEmailRole,
			///(optional) A preview image showing the visual appearance of the
			///theme.
			PreviewRole
		};

		///Creates a new KgvTheme with the given @a identifier. The identifier
		///must be application-unique.
		///@warning The @a identifier is sometimes used as part of a file path,
		///and should therefore not contain any characters which are not allowed
		///in file paths. (The slash, or backslash on Windows, is allowed.)
		KgvTheme(const QByteArray& identifier);
		///Destroys this KgvTheme instance.
		~KgvTheme();

		///@return the data stored unter the given @a role, or the given
		///@a defaultValue if no data is stored
		QVariant data(KgvTheme::Role role, const QVariant& defaultValue = QVariant()) const;
		///@overload for custom data
		QVariant data(const QByteArray& key, const QVariant& defaultValue = QVariant()) const;
		///Stores the given @a value unter the given @a role.
		void setData(KgvTheme::Role role, const QVariant& value);
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

#endif // KGVTHEME_H
