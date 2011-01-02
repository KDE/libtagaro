/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>                     *
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

#include <QtGui/QApplication>
#include <QtGui/QGraphicsView>
#include <Tagaro/Board>
#include <Tagaro/Scene>
#include <Tagaro/SimpleThemeProvider>
#include <Tagaro/SpriteObjectItem>
#include <Tagaro/StandardTheme>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	Tagaro::SimpleThemeProvider provider;
	provider.addTheme(new Tagaro::StandardTheme("example-imagesource.desktop", &provider));

	QGraphicsView view;
	Tagaro::Scene scene(provider.sprite("background"));
	scene.setMainView(&view);

	Tagaro::Board board;
	board.setLogicalSize(QSizeF(2, 2));
	scene.addItem(&board);
	(new Tagaro::SpriteObjectItem(provider.sprite("item1"), &board))->setPos(QPointF(0, 0));
	(new Tagaro::SpriteObjectItem(provider.sprite("item2"), &board))->setPos(QPointF(0, 1));
	(new Tagaro::SpriteObjectItem(provider.sprite("item3"), &board))->setPos(QPointF(1, 0));
	(new Tagaro::SpriteObjectItem(provider.sprite("item4"), &board))->setPos(QPointF(1, 1));

	view.show();
	return app.exec();
}

