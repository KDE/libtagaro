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

#ifndef KGAME_THEMEPROVIDER_H
#define KGAME_THEMEPROVIDER_H

#include "graphicssourceconfig.h"

class QAbstractItemModel;
#include <QtCore/QObject>

#include <libtagaro_export.h>

namespace KGame {

class Sprite;
class Theme;

/**
 * @class KGame::ThemeProvider themeprovider.h <KGame/ThemeProvider>
 *
 * A theme provider is used to create KGame::Theme instances. If you worked
 * with Qt's model/view classes before, you will feel at home immediately.
 *
 * Subclass implementations need to use the non-virtual protected methods or
 * this class to indicate when the set of themes in this provider changes.
 * @warning Do not use the signals of this class directly.
 *
 * The theme provider has a selection, which is respected by KGame::Renderer
 * instances using this theme provider. If a subclass wants to save the
 * selection, it should connect to the selectedThemeChanged() signal.
 *
 * @see KGame::StandardThemeProvider
 */
class KGAME_EXPORT ThemeProvider : public QObject
{
	Q_OBJECT
	public:
		///Creates a new KGame::ThemeProvider instance.
		///@param ownThemes whether the provider will take ownership of Theme
		///                 instances passed to it
		///@param config    the config of GraphicsSources created by the
		///                 themes in this ThemeProvider
		explicit ThemeProvider(bool ownThemes, QObject* parent = 0, const KGame::GraphicsSourceConfig& config = KGame::GraphicsSourceConfig());
		///Destroys this KGame::ThemeProvider instance.
		virtual ~ThemeProvider();

		///@return the config of GraphicsSources created by the themes in this
		///ThemeProvider
		const KGame::GraphicsSourceConfig& config() const;
		///@return a KGame::Sprite instance for the given @a spriteKey
		///
		///The @a spriteKey may not contain "@" characters.
		KGame::Sprite* sprite(const QString& spriteKey) const;

		///@return a list-shaped model exposing the themes' data
		QAbstractItemModel* model() const;
		///@return the themes in this provider
		QList<const KGame::Theme*> themes() const;
		///@return the default theme, or 0 if the provider does not contain any
		///themes
		const KGame::Theme* defaultTheme() const;
		///@return the currently selected theme, or 0 if the provider does not
		///contain any themes
		const KGame::Theme* selectedTheme() const;
	public Q_SLOTS:
		///Selects the given @a theme. If a KGame::Renderer instance is using
		///this theme provider, it will load this theme automatically.
		///
		///Subclasses can reimplement this if there are special metathemes which
		///cannot be selected, but trigger some operation when they are clicked
		///in the theme selector (e.g. the KNewStuff metatheme provided by the
		///KGame::StandardThemeProvider, which triggers a KNewStuff dialog).
		virtual void setSelectedTheme(const KGame::Theme* theme);
	Q_SIGNALS:
		///This signal is emitted when the selected theme changes, or when the
		///currently selected theme is modified.
		void selectedThemeChanged(const KGame::Theme* theme);
		///This signal is emitted when the list of themes is about to change.
		void themesAboutToBeChanged();
		///This signal is emitted when the list of themes has changed.
		void themesChanged();
	protected:
		///@return the themes in this provider
		QList<KGame::Theme*> nonConstThemes();
		///Sets the themes in this provider. It depends on what you specified
		///the constructor whether the provider will take ownership of the
		///themes.
		void setThemes(const QList<KGame::Theme*>& themes);
	private:
		class Private;
		Private* const d;
};

/**
 * @class KGame::StandardThemeProvider themeprovider.h <KGame/StandardThemeProvider>
 *
 * This theme provider locates theme files (*.desktop) in the installation
 * directories via KStandardDirs. The SVG file is referenced by the "FileName"
 * key in the desktop file's main config group, and expected to be in the same
 * directory as the desktop file.
 */
class KGAME_EXPORT StandardThemeProvider : public KGame::ThemeProvider
{
	Q_OBJECT
	public:
		///Creates a new KGame::StandardThemeProvider instance. The @a configKey
		///is used to store the theme selection in the configuration. The
		///following two arguments are passed to KStandardDirs, so this theme
		///provider will list all themes which can be found with:
		///@code
		///KGlobal::dirs()->findAllResources(ksdResource, ksdDirectory + "/*.desktop");
		///@endcode
		StandardThemeProvider(const QByteArray& configKey, const QByteArray& ksdResource, const QString& ksdDirectory, QObject* parent = 0, const KGame::GraphicsSourceConfig& config = KGame::GraphicsSourceConfig());
		///Destroys this KGame::StandardThemeProvider instance.
		virtual ~StandardThemeProvider();

		virtual void setSelectedTheme(const KGame::Theme* theme);
	private:
		class Private;
		Private* const d;
};

/**
 * @class KGame::SimpleThemeProvider themeprovider.h <KGame/SimpleThemeProvider>
 *
 * This theme provider can be filled with themes manually. This is especially
 * useful if your application ships only one theme and you want to avoid the
 * need to create
 */
class KGAME_EXPORT SimpleThemeProvider : public KGame::ThemeProvider
{
	Q_OBJECT
	public:
		///Creates a new KGame::SimpleThemeProvider instance.
		explicit SimpleThemeProvider(QObject* parent = 0, const KGame::GraphicsSourceConfig& config = KGame::GraphicsSourceConfig());
		///Destroys this KGame::SimpleThemeProvider instance.
		virtual ~SimpleThemeProvider();

		///Adds a @a theme to this provider. The theme must be initialized with
		///this provider passed in the constructor. The provider takes
		///ownership of the theme.
		void addTheme(KGame::Theme* theme);
	private:
		class Private;
		Private* const d;
};

} //namespace KGame

#endif // KGAME_THEMEPROVIDER_H
