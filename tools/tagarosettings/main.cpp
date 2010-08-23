/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include <KDE/KAboutData>
#include <KDE/KApplication>
#include <KDE/KCmdLineArgs>
#include <KDE/KConfigDialog>
#include <KDE/KLocale>

#include <Tagaro/Settings>
#include "ui_visuals.h"

int main(int argc, char** argv)
{
	const KLocalizedString appName = ki18nc("application name", "TagaroSettings");
	KAboutData about("tagarosettings", 0, appName, "0.1",
		ki18n("Global configuration interface for libtagaro-based applications"),
		KAboutData::License_GPL, ki18n("Copyright 2010 Stefan Majewsky"));
	about.addAuthor(ki18n("Stefan Majewsky"), KLocalizedString(), "majewsky@gmx.net");
	KCmdLineArgs::init(argc, argv, &about);

	KApplication app;

	KConfigDialog dialog(0, QString(), Tagaro::Settings::self());
	QWidget* visualsPage = new QWidget;
	Ui_Visuals visualsUi;
	visualsUi.setupUi(visualsPage);
	dialog.setWindowTitle(appName.toString());
	dialog.addPage(visualsPage, i18nc("@item:inlistbox on the left pane of a config dialog", "Visuals"), "games-config-board", i18nc("@title:tab specifically the title of a config dialog page", "Configure advanced aspects of KDE games visuals"));

	dialog.show();
	return app.exec();
}
