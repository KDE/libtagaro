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

#include "declthemeprovider.h"
#include "graphicssource.h"
#include "sprite.h"
#include "sprite_p.h"
#include "themeprovider.h"

struct KGame::DeclarativeThemeProvider::Private
{
	const KGame::ThemeProvider* m_tp;
	Private(const KGame::ThemeProvider* tp): m_tp(tp) {}
};

KGame::DeclarativeThemeProvider::DeclarativeThemeProvider(const KGame::ThemeProvider* tp)
	: QDeclarativeImageProvider(QDeclarativeImageProvider::Image)
	, d(new Private(tp))
{
}

KGame::DeclarativeThemeProvider::~DeclarativeThemeProvider()
{
	delete d;
}

QImage KGame::DeclarativeThemeProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
	//split "~frame" suffix (where "frame" == uint) from sprite key
	QString spriteKey; int frame;
	const int pos = id.indexOf(QLatin1Char('~'));
	if (pos == -1)
	{
		spriteKey = id;
		frame = -1;
	}
	else
	{
		bool ok;
		frame = id.mid(pos + 1).toUInt(&ok);
		if (ok)
		{
			spriteKey = spriteKey.left(pos);
		}
		else
		{
			spriteKey = id;
			frame = -1;
		}
	}
	//get sprite, default size
	KGame::Sprite* sprite = d->m_tp->sprite(spriteKey);
	const QSize defaultSize = sprite->bounds().size().toSize();
	if (size)
	{
		*size = defaultSize;
	}
	//get render size for this request
	const QSize renderSize(
		requestedSize.width() > 0 ? requestedSize.width() : defaultSize.width(),
		requestedSize.height() > 0 ? requestedSize.height() : defaultSize.height()
	);
	if (renderSize.isEmpty())
	{
		return QImage();
	}
	//fetch image (NOTE: this is completely parallel to KGame::SpriteFetcher,
	//because I expect QML environments not to use the normal SpriteClient etc.
	//at all, so the advantage of populating the other cache too is nil)
	const KGame::GraphicsSource* source = sprite->d->m_source;
	const QString element = source->frameElementKey(sprite->d->m_element, frame);
	return source->elementImage(element, renderSize, QString(), false);
}
