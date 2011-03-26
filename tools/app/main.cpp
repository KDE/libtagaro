/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#include "applist.h"

#include <QtGui/QListView>
#include <KDE/KAboutData>
#include <KDE/KApplication>
#include <KDE/KCmdLineArgs>

int main(int argc, char** argv)
{
	KAboutData about(
		"tagaro", 0, ki18nc("App name", "Tagaro"), "0.1",
		ki18n("KDE Games Center"),
		KAboutData::License_GPL, ki18n("Copyright 2011 Stefan Majewsky")
	);
	about.addAuthor(ki18n("Stefan Majewsky"), KLocalizedString(), "Majewsky@gmx.net", "http://majewsky.wordpress.com");
	KCmdLineArgs::init(argc, argv, &about);

	KApplication app; //TODO: KUniqueApplication (and redirect open requests to the running instance, like e.g. Konqueror can do)

	TApp::AppListView view;
	view.show();

	return app.exec();
}
