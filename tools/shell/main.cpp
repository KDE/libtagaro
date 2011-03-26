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

#include <iostream>
#include <QtGui/QMainWindow>
#include <KDE/KAboutData>
#include <KDE/KApplication>
#include <KDE/KCmdLineArgs>
#include <KDE/KCmdLineOptions>
#include <KDE/KServiceTypeTrader>

int main(int argc, char** argv)
{
	KAboutData about(
		"tagaroshell", 0, ki18nc("App name", "Tagaro Shell"), "0.1",
		ki18n("Runtime environment for Tagaro games"),
		KAboutData::License_GPL, ki18n("Copyright 2011 Stefan Majewsky")
	);
	about.addAuthor(ki18n("Stefan Majewsky"), KLocalizedString(), "Majewsky@gmx.net", "http://majewsky.wordpress.com");
	KCmdLineArgs::init(argc, argv, &about);

	KCmdLineOptions options;
	options.add("+plugin", ki18n("Name of game plugin to load"));
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
	const QString plugin = args->arg(0);
	args->clear();

	//find plugin with KServiceTypeTrader
	QString error;
	QWidget* game = KServiceTypeTrader::createInstanceFromQuery<QWidget>(
		QString::fromLatin1("Tagaro/Game"),
		QString::fromLatin1("'%1' == Library").arg(plugin),
		0, QVariantList(), &error
	);
	if (!game)
	{
		std::cerr << "Could not load game. Error was: " << qPrintable(error) << std::endl;
		return 1;
	}

	//show game in window
	QMainWindow window;
	window.setCentralWidget(game);
	window.show();

	return app.exec();
}