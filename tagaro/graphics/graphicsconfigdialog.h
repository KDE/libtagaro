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

#ifndef KGAME_GRAPHICSCONFIGDIALOG_H
#define KGAME_GRAPHICSCONFIGDIALOG_H

#include <libtagaro_export.h>

#include <KDE/KPageDialog>

namespace KGame {

class ThemeProvider;

/**
 * @class KGame::GraphicsConfigDialog graphicsconfigdialog.h <KGame/GraphicsConfigDialog>
 *
 * This dialog provides convenience functions for configuring graphical
 * components.
 *
 * The following example code illustrates how to add a theme selector page:
 * @code
 * KGame::GraphicsConfigDialog dialog;
 * dialog.addThemeSelector(themeProvider,
 *     i18n("Theme"), "games-config-theme",
 *     i18n("Choose a theme")
 * );
 * @endcode
 * Any KGame::Renderer instances connected to the @a themeProvider will pickup
 * selection changes automatically. If you need to be informed about the
 * selection change for some other reason, use the theme provider's
 * selectedThemeChanged() signal.
 */
class KGAME_EXPORT GraphicsConfigDialog : public KPageDialog
{
	Q_OBJECT
	public:
		///Creates a new KGame::GraphicsConfigDialog.
		explicit GraphicsConfigDialog(const QString& title = QString(), QWidget* parent = 0);
		///Destroys this KGame::ConfigDialog instance.
		virtual ~GraphicsConfigDialog();

		///Adds a page to this dialog which allows to change a theme selection.
		///@param provider The theme provider whose selection shall be configured.
		///@param itemName The name of the page.
		///@param icon     The icon that should be used if needed.
		///@param header   The header text to be shown above the page (defaults
		///                to the item name if nothing is given).
		void addThemeSelector(KGame::ThemeProvider* provider, const QString& itemName, const KIcon& icon, const QString& header = QString());
	private:
		class Private;
		Private* const d;
		Q_PRIVATE_SLOT(d, void _k_restoreDefault());
		Q_PRIVATE_SLOT(d, void _k_selectionChanged());
};

} //namespace KGame

#endif // KGAME_GRAPHICSCONFIGDIALOG_H
