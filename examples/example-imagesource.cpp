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
#include <KGame/Board>
#include <KGame/Scene>
#include <KGame/SimpleThemeProvider>
#include <KGame/SpriteObjectItem>
#include <KGame/StandardTheme>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	KGame::SimpleThemeProvider provider;
	provider.addTheme(new KGame::StandardTheme("example-imagesource.desktop", &provider));

	QGraphicsView view;
	KGame::Scene scene(provider.sprite("background"));
	scene.setMainView(&view);

	KGame::Board board;
	board.setLogicalSize(QSizeF(2, 2));
	scene.addItem(&board);
	(new KGame::SpriteObjectItem(provider.sprite("item1"), &board))->setPos(QPointF(0, 0));
	(new KGame::SpriteObjectItem(provider.sprite("item2"), &board))->setPos(QPointF(0, 1));
	(new KGame::SpriteObjectItem(provider.sprite("item3"), &board))->setPos(QPointF(1, 0));
	(new KGame::SpriteObjectItem(provider.sprite("item4"), &board))->setPos(QPointF(1, 1));

	view.show();
	return app.exec();
}

