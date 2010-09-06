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

#ifndef TAGARO_CONFIGDIALOG_H
#define TAGARO_CONFIGDIALOG_H

#include <KDE/KConfigDialog>

#include "objectpool.h"
#include <libtagaro_export.h>

namespace Tagaro {

class ThemeProvider;

/**
 * @class Tagaro::ConfigDialog configdialog.h <Tagaro/ConfigDialog>
 *
 * This KConfigDialog subclass provides convenience functions for configuring
 * libtagaro components.
 *
 * @section themeselector Theme selectors
 *
 * The following example code illustrates how to add a theme selector page:
 * @code
 * Tagaro::ConfigDialog dialog;
 * dialog.addThemeSelector(
 *     themeProvider,
 *     Tagaro::ConfigDialog::NormalThemeSelector,
 *     i18n("Theme"), "games-config-theme", i18n("Choose a theme")
 * );
 * @endcode
 * Any Tagaro::Renderer instances connected to the @a themeProvider will pickup
 * selection changes automatically. If you need to be informed about the
 * selection change for some other reason, use the
 * Tagaro::ThemeProvider::selectedIndexChanged() signal.
 */
class TAGARO_EXPORT ConfigDialog : public KConfigDialog
{
	public:
		///This enumeration describes optional behavior of a theme selector.
		enum ThemeSelectorOption
		{
			NormalThemeSelector = 0,      ///< Default behavior.
			WithNewStuffDownload = 1 << 0 ///< Enable download of new themes via KNewStuff3.
		};
		Q_DECLARE_FLAGS(ThemeSelectorOptions, ThemeSelectorOption)

		///Creates a new Tagaro::ConfigDialog. The arguments are passed to the
		///KConfigDialog constructor.
		ConfigDialog(QWidget* parent, const QString& name, KConfigSkeleton* config = 0);
		///Destroys this Tagaro::ConfigDialog instance.
		virtual ~ConfigDialog();

		///Adds a page to this dialog which allows to select a theme from the
		///given theme @a provider.
		///@param itemName The name of the page.
		///@param iconName The name of the icon that should be used if needed.
		///@param header   The header text to be shown above the page (defaults
		///                to the item name if nothing is given).
		void addThemeSelector(Tagaro::ThemeProviderPtr provider, Tagaro::ConfigDialog::ThemeSelectorOptions options, const QString& itemName, const QString& iconName, const QString& header = QString());
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

} //namespace Tagaro

Q_DECLARE_OPERATORS_FOR_FLAGS(Tagaro::ConfigDialog::ThemeSelectorOptions)

#endif // TAGARO_CONFIGDIALOG_H
