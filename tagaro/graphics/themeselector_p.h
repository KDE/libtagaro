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

#ifndef TAGARO_THEMESELECTOR_P_H
#define TAGARO_THEMESELECTOR_P_H

class QItemSelection;
class QListView;
#include <QtGui/QWidget>

namespace Tagaro {

class ThemeProvider;

///@internal
class ThemeSelector : public QWidget
{
	Q_OBJECT
	public:
		ThemeSelector(Tagaro::ThemeProvider* provider);

		Tagaro::ThemeProvider* provider() const;
	private Q_SLOTS:
		void updateSelectedIndex(int selectedIndex);
		void storeSelection(const QItemSelection& selection);
	private:
		Tagaro::ThemeProvider* m_provider;
		QListView* m_themeList;
};

} //namespace Tagaro

#endif // TAGARO_THEMESELECTOR_P_H
