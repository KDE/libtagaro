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

#ifndef LIBKGAMEVISUALS_CONFIGDIALOG_H
#define LIBKGAMEVISUALS_CONFIGDIALOG_H

#include <KConfigDialog>
class KgvThemeProvider;

#include <libkgame_export.h>

/**
 * @class KgvConfigDialog kgvconfigdialog.h <KgvConfigDialog>
 *
 * This KConfigDialog subclass provides convenience functions for configuring
 * libkgamevisuals components.
 *
 * @section themeselector Theme selectors
 *
 * For example, if you use a KgvRenderer with a KgvDesktopThemeProvider,
 * @code
 * KgvRenderer* r = new KgvRenderer("foobar");
 * @endcode
 * add a theme selector page like this:
 * @code
 * KgvConfigDialog dialog;
 * dialog.addThemeSelector(
 *     KgvDesktopThemeProvider::instance("foobar"),
 *     KgvConfigDialog::NormalThemeSelector,
 *     i18n("Theme"), "games-config-theme", i18n("Choose a theme")
 * );
 * @endcode
 * The KgvRenderer instance will pickup selection changes in KgvThemeProvider
 * automatically. If you need to be informed about the selection change for some
 * other reason, use the KgvThemeProvider::selectedIndexChanged() signal.
 */
class KGAMEVISUALS_EXPORT KgvConfigDialog : public KConfigDialog
{
	public:
		///This enumeration describes optional behavior of a theme selector.
		enum ThemeSelectorOption
		{
			NormalThemeSelector = 0,      ///< Default behavior.
			WithNewStuffDownload = 1 << 0 ///< Enable download of new themes via KNewStuff3.
		};
		Q_DECLARE_FLAGS(ThemeSelectorOptions, ThemeSelectorOption)

		///Creates a new KgvConfigDialog. The arguments are passed to the
		///KConfigDialog constructor.
		KgvConfigDialog(QWidget* parent, const QString& name, KConfigSkeleton* config);
		///Destroys this KgvConfigDialog instance.
		virtual ~KgvConfigDialog();

		///Adds a page to this dialog which allows to select a theme from the
		///given theme @a provider.
		///@param itemName The name of the page.
		///@param iconName The name of the icon that should be used if needed.
		///@param header   The header text to be shown above the page (defaults
		///                to the item name if nothing is given).
		void addThemeSelector(KgvThemeProvider* provider, KgvConfigDialog::ThemeSelectorOptions options, const QString& itemName, const QString& iconName, const QString& header = QString());
	protected:
		virtual bool hasChanged();
		virtual bool isDefault();
		virtual void updateSettings();
		virtual void updateWidgets();
		virtual void updateWidgetsDefault();
	private:
		class Private;
		Private* const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KgvConfigDialog::ThemeSelectorOptions)

#endif // LIBKGAMEVISUALS_CONFIGDIALOG_H
