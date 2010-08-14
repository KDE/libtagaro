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

#ifndef KGVTHEMESELECTOR_P_H
#define KGVTHEMESELECTOR_P_H

#include "kgvconfigdialog.h"

class QListWidget;
#include <QtGui/QWidget>

///@internal
class KgvThemeSelector : public QWidget
{
	Q_OBJECT
	public:
		KgvThemeSelector(KgvThemeProvider* provider, KgvConfigDialog::ThemeSelectorOptions options);

		KgvThemeProvider* provider() const;
		int selectedIndex() const;
		void setSelectedIndex(int selectedIndex);
	Q_SIGNALS:
		void selectedIndexChanged();
	public Q_SLOTS:
		void themesInserted(int firstIndex, int lastIndex);
		void themesChanged(int firstIndex, int lastIndex);
		void themesRemoved(int firstIndex, int lastIndex);
		void openNewStuffDialog();
	private:
		KgvThemeProvider* m_provider;
		QListWidget* m_themeList;
};

#endif // KGVTHEMESELECTOR_P_H
