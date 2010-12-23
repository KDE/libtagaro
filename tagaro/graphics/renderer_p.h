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

#ifndef TAGARO_RENDERER_P_H
#define TAGARO_RENDERER_P_H

#include "renderer.h"
#include "renderbackend.h"
#include "sprite.h"

#include <QtCore/QHash>
#include <KDE/KImageCache>

namespace Tagaro {

class Theme;

namespace Internal
{
	//Describes the state of a Tagaro::RendererClient.
	struct ClientSpec
	{
		inline ClientSpec(const QString& spriteKey = QString(), int frame = -1, const QSize& size = QSize());
		QString spriteKey;
		int frame;
		QSize size;
	};
	ClientSpec::ClientSpec(const QString& spriteKey_, int frame_, const QSize& size_)
		: spriteKey(spriteKey_)
		, frame(frame_)
		, size(size_)
	{
	}
};

class RendererPrivate : public QObject
{
	Q_OBJECT
	public:
		RendererPrivate(Tagaro::ThemeProvider* provider, const Tagaro::RenderBehavior& behavior, Tagaro::Renderer* parent);
		void setTheme(const Tagaro::Theme* theme);
		bool setThemeInternal(const Tagaro::Theme* theme);
	public Q_SLOTS:
		void loadSelectedTheme();
	public:
		Tagaro::Renderer* m_parent;

		Tagaro::ThemeProvider* m_themeProvider;
		const Tagaro::Theme* m_theme;

		const Tagaro::RenderBehavior m_behavior;
		Tagaro::RenderBackend* m_backend;

		QHash<QString, Tagaro::Sprite*> m_sprites; //maps sprite keys -> sprite instances
};

} //namespace Tagaro

#endif // TAGARO_RENDERER_P_H
