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

#ifndef TAGARO_THEMEPROVIDER_H
#define TAGARO_THEMEPROVIDER_H

#include <QtCore/QObject>

#include <libtagaro_export.h>

TAGARO_BEGIN_NAMESPACE

class Theme;

/**
 * @class Tagaro::ThemeProvider themeprovider.h <Tagaro/ThemeProvider>
 *
 * A theme provider is used to create Tagaro::Theme instances. If you worked
 * with Qt's model/view classes before, you will feel at home immediately.
 *
 * Subclass implementations need to use the non-virtual protected methods or
 * this class to indicate when the set of themes in this provider changes.
 * @warning Do not use the signals of this class directly.
 *
 * The theme provider has a selection, which is respected by Tagaro::Renderer
 * instances using this theme provider. If a subclass wants to save the
 * selection, it should connect to the selectedIndexChanged() signal.
 *
 * @see Tagaro::DesktopThemeProvider
 */
class TAGARO_EXPORT ThemeProvider : public QObject
{
	Q_OBJECT
	public:
		///Creates a new Tagaro::ThemeProvider instance.
		ThemeProvider(QObject* parent = 0);
		///Destroys this Tagaro::ThemeProvider instance.
		///@warning Subclasses have to take care that their themes are deleted,
		///the Tagaro::ThemeProvider cannot do that by itself.
		virtual ~ThemeProvider();

		///@return the number of themes in this provider
		virtual int themeCount() const = 0;
		///@return the @a index-th theme in this provider. If @a index is
		///negative, bigger than or equal to themeCount(), return 0.
		virtual const Tagaro::Theme* theme(int index) const = 0;
		///@return from this provider the theme with the given @a identifier,
		///or 0 if no such theme can be found.
		const Tagaro::Theme* theme(const QByteArray& identifier) const;

		///@return the index of the selected theme, or 0 if nothing is selected
		int selectedIndex() const;
		///Selects the theme identified by the given @a index (unless @a index
		///is out of bounds). If a Tagaro::Renderer instance is using this theme
		///provider, it will load this theme automatically.
		void setSelectedIndex(int index);
	protected:
		///Announces that the data of the themes between @a firstIndex and
		///@a lastIndex inclusive has changed.
		void announceChange(int firstIndex, int lastIndex);
		///Begins an insertion operation. The new themes will be inserted
		///between @a firstIndex and @a lastIndex inclusive.
		void beginInsertThemes(int firstIndex, int lastIndex);
		///Begins a removal operation. The themes to be removed are those
		///between @a firstIndex and @a lastIndex inclusive.
		void beginRemoveThemes(int firstIndex, int lastIndex);
		///Ends an insertion operation started by beginInsertThemes().
		void endInsertThemes();
		///Ends a removal operation started by beginRemoveThemes().
		void endRemoveThemes();
	Q_SIGNALS:
		///This signal is emitted just before themes are inserted into the
		///provider's theme list. The new themes will be positioned between
		///@a firstIndex and @a lastIndex inclusive.
		void themesAboutToBeInserted(int firstIndex, int lastIndex);
		///This signal is emitted just before themes are removed from the
		///provider's theme list. The themes to be removed are those between
		///@a firstIndex and @a lastIndex inclusive.
		void themesAboutToBeRemoved(int firstIndex, int lastIndex);
		///This signal is emitted when the data in themes changes. The affected
		///themes are those between @a firstIndex and @a lastIndex inclusive.
		void themesChanged(int firstIndex, int lastIndex);
		///This signal is emitted after themes have been inserted into the
		///provider's theme list. The new themes will be positioned between
		///@a firstIndex and @a lastIndex inclusive.
		void themesInserted(int firstIndex, int lastIndex);
		///This signal is emitted after themes have been removed from the
		///provider's theme list. The themes which have been removed are those
		///between @a firstIndex and @a lastIndex inclusive.
		void themesRemoved(int firstIndex, int lastIndex);
		///This signal is emitted when the selected theme changes.
		void selectedIndexChanged(int index);
	private:
		class Private;
		Private* const d;
};

class DtpInstantiator;

/**
 * @class Tagaro::DesktopThemeProvider themeprovider.h <Tagaro/ThemeProvider>
 *
 * This theme provider locates theme files (*.desktop) in the installation
 * directories via KStandardDirs. The SVG file is referenced by the "FileName"
 * key in the desktop file's main config group, and expected to be in the same
 * directory as the desktop file.
 *
 * @section config Configuration
 *
 * The configuration of a Tagaro::DesktopThemeProvider instance is stored in the
 * application's configuration file (i.e. KGlobal::config()). A typical
 * configuration looks like this (in fact, the values below are the defaults):
 * @code
 * [Tagaro::DesktopThemeProvider][foo]
 * ResourceType=appdata
 * Directory=themes
 * @endcode
 * In this example, "foo" is the key which needs to be passed to the instance()
 * method. The "ResourceType" and "Directory" will be used to find the theme
 * files with KStandardDirs like this:
 * @code
 * KGlobal::dirs()->findAllResources(resourceType, directory + "/.desktop");
 * @endcode
 * If you need to differ from the standard configuration, it is recommended to
 * install a system-wide configuration file which includes your custom
 * configuration.
 */
class TAGARO_EXPORT DesktopThemeProvider : public Tagaro::ThemeProvider
{
	Q_OBJECT
	public:
		///@return a Tagaro::DesktopThemeProvider instance
		///Application writers are not allowed to instantiate this class
		///directly, but must use this method instead. If given the same @a key
		///in multiple calls, this method will always return the same instance.
		///Multiple instances can be created by using different @a keys.
		static Tagaro::DesktopThemeProvider* instance(const QByteArray& key);

		virtual int themeCount() const;
		virtual const Tagaro::Theme* theme(int index) const;
	private:
		///Creates a new Tagaro::DesktopThemeProvider instance. For the meaning
		///of the @a key argument, see the instance() method.
		///@see instance()
		DesktopThemeProvider(const QByteArray& key);
		///Destroys this Tagaro::DesktopThemeProvider instance.
		virtual ~DesktopThemeProvider();
	private:
		friend class DtpInstantiator;
		class Private;
		Private* const d;
		Q_PRIVATE_SLOT(d, void saveSelectedIndex(int));
};

/**
 * @class Tagaro::FileThemeProvider themeprovider.h <Tagaro/ThemeProvider>
 *
 * This theme provider provides exactly one theme, which consists only of a
 * single SVG file. You may use this provider if you do not have multiple themes
 * to offer and want to avoid the need to create .desktop theme files.
 */
class TAGARO_EXPORT FileThemeProvider : public Tagaro::ThemeProvider
{
	Q_OBJECT
	public:
		///Creates a new Tagaro::FileThemeProvider instance, which provides the
		///given SVG @a file.
		FileThemeProvider(const QString& file, QObject* parent = 0);
		///Destroys this Tagaro::FileThemeProvider instance.
		virtual ~FileThemeProvider();

		virtual int themeCount() const;
		virtual const Tagaro::Theme* theme(int index) const;
	private:
		class Private;
		Private* const d;
};

TAGARO_END_NAMESPACE

#endif // TAGARO_THEMEPROVIDER_H
