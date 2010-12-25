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

#include <QtCore/QDir>
#include <QtCore/QVariant>

#include <libtagaro_export.h>

namespace Tagaro {

class RenderBackend;
class ThemeProvider;

/**
 * @class Tagaro::Theme theme.h <Tagaro/Theme>
 *
 * A theme is an entity consisting of graphics sources and metadata. Theme
 * instances are usually created and managed by a Tagaro::ThemeProvider.
 *
 * The most important part of a theme is its routing table: The routing table
 * maps the keys of sprites used by the game to elements in graphics sources.
 */
class TAGARO_EXPORT Theme
{
	Q_DISABLE_COPY(Theme)
	public:
		///Creates a new Tagaro::Theme with the given @a identifier. The
		///identifier must be application-unique.
		Theme(const QByteArray& identifier, const Tagaro::ThemeProvider* provider);
		///Destroys this Tagaro::Theme instance.
		~Theme();

		///@return the internal identifier for this theme
		QByteArray identifier() const;
		///@return the provider which manages this theme
		const Tagaro::ThemeProvider* provider() const;
		///@return whether all graphics sources could be loaded successfully
		bool isValid() const;

		///@return the name of this theme
		QString name() const;
		///@see name()
		void setName(const QString& name);
		///@return an additional description beyond the name()
		QString description() const;
		///@see description()
		void setDescription(const QString& description);
		///@return the name of the theme author
		QString author() const;
		///@see author()
		void setAuthor(const QString& author);
		///@return the email address of the theme author
		QString authorEmail() const;
		///@see authorEmail()
		void setAuthorEmail(const QString& authorEmail);
		///@return a preview image showing the visual appearance of the theme
		QPixmap preview() const;
		///@see preview()
		void setPreview(const QPixmap& preview);

		///@return custom data
		///
		///This API is provided for theme files which contains additional
		///application-specific metadata.
		QMap<QString, QString> customData() const;
		///@see customData()
		void setCustomData(const QMap<QString, QString>& customData);

		///Adds a new @a backend to this theme. If @a identifier is empty, it
		///is replaced by the default identifier "default".
		void addBackend(const QByteArray& identifier, Tagaro::RenderBackend* backend);
		///Adds a new backend to this theme by instantiating one with the given
		///@a specification. Relative file paths are resolved against the
		///reference directories in the @a refDirs parameter. If the
		///specification is invalid, a null backend is created to indicate that
		///the theme could not be loaded properly.
		void addBackend(const QByteArray& identifier, const QString& specification, const QList<QDir>& refDirs);
		///@return the backend with the given @a identifier, or 0 if no such
		///backend exists
		///
		///If @a identifier is empty, it is replaced by the default identifier
		///"default".
		const Tagaro::RenderBackend* backend(const QByteArray& identifier) const;
		///Adds a new route to this theme's routing table.
		///@param spriteKey  a regular expression matching the sprite keys
		///                  affected by this route (the sprite key is what you
		///                  give to Tagaro::Renderer::sprite())
		///@param elementKey the element key for the graphics source (this is
		///                  what you give to Tagaro::RenderBackend methods);
		///                  the argument can contain "%0", "%1", "%2", ...
		///                  which are replaced by the regexp's capturedTexts()
		///@param backend    the backend which serves the matching elements
		void addRoute(const QRegExp& spriteKey, const QString& elementKey, const Tagaro::RenderBackend* backend);
		///Resolve sprite keys to backends and element keys with the routing
		///table of this theme.
		QPair<const Tagaro::RenderBackend*, QString> mapSpriteKey(const QString& spriteKey) const;
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
		StandardTheme(const QString& filePath, const Tagaro::ThemeProvider* provider);
		///Creates a new Tagaro::StandardTheme instance by reading in the
		///desktop file at
		///@code
		///KStandardDirs::locate(ksdResource, ksdDirectory + fileName);
		///@endcode
		///The important difference to the single-argument constructor is that
		///references to graphics files are also resolved via
		///KStandardDirs::locate rather than just looking in the same directory.
		StandardTheme(const QByteArray& ksdResource, const QString& ksdDirectory, const QString& fileName, const Tagaro::ThemeProvider* provider);
	private:
		class Private;
		Private* const d;
};

} //namespace Tagaro

#endif // TAGARO_THEME_H
