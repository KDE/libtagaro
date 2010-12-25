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

#include "renderer.h"
#include "renderer_p.h"
#include "sprite_p.h"
#include "theme.h"
#include "themeprovider.h"

#include <QtCore/QThreadPool>
#include <KDE/KDebug>

Tagaro::RendererPrivate::RendererPrivate(Tagaro::ThemeProvider* provider)
	: m_themeProvider(provider)
	, m_theme(0)
{
	connect(m_themeProvider, SIGNAL(selectedThemeChanged(const Tagaro::Theme*)), SLOT(loadSelectedTheme()));
	loadSelectedTheme();
}

Tagaro::Renderer::Renderer(Tagaro::ThemeProvider* provider)
	: d(new Tagaro::RendererPrivate(provider))
{
}

Tagaro::Renderer::~Renderer()
{
	//cleanup sprites (without qDeleteAll because that's not a friend of Sprite)
	QHash<QString, Tagaro::Sprite*>::const_iterator it1 = d->m_sprites.constBegin(),
	                                                it2 = d->m_sprites.constEnd();
	for (; it1 != it2; ++it1)
		delete it1.value();
	//cleanup own stuff
	delete d;
}

Tagaro::ThemeProvider* Tagaro::Renderer::themeProvider() const
{
	return d->m_themeProvider;
}

void Tagaro::RendererPrivate::setTheme(const Tagaro::Theme* theme)
{
	if (!theme)
	{
		return;
	}
	const Tagaro::Theme* oldTheme = m_theme;
	if (oldTheme == theme)
	{
		return;
	}
	kDebug() << "Setting theme:" << theme->identifier();
	if (!setThemeInternal(theme))
	{
		const Tagaro::Theme* defaultTheme = m_themeProvider->defaultTheme();
		if (theme != defaultTheme && defaultTheme)
		{
			kDebug() << "Falling back to default theme:" << defaultTheme->identifier();
			setThemeInternal(defaultTheme);
		}
	}
	if (oldTheme != m_theme)
	{
		QThreadPool::globalInstance()->waitForDone(); //TODO: optimize
		//announce change to sprites
		QHash<QString, Tagaro::Sprite*>::const_iterator it1 = m_sprites.constBegin(), it2 = m_sprites.constEnd();
		for (; it1 != it2; ++it1)
		{
			const QPair<const Tagaro::RenderBackend*, QString> renderElement = m_theme->mapSpriteKey(it1.key());
			it1.value()->d->setBackend(renderElement.first, renderElement.second);
		}
	}
}

void Tagaro::RendererPrivate::loadSelectedTheme()
{
	setTheme(m_themeProvider->selectedTheme());
}

bool Tagaro::RendererPrivate::setThemeInternal(const Tagaro::Theme* theme)
{
	if (!theme->isValid())
	{
		return false;
	}
	m_theme = theme;
	return true;
}

Tagaro::Sprite* Tagaro::Renderer::sprite(const QString& key) const
{
	Tagaro::Sprite*& sprite = d->m_sprites[key];
	if (!sprite)
	{
		//instantiate on first use
		sprite = new Tagaro::Sprite;
		const QPair<const Tagaro::RenderBackend*, QString> renderElement = d->m_theme->mapSpriteKey(key);
		sprite->d->setBackend(renderElement.first, renderElement.second);
	}
	return sprite;
}

#include "renderer.moc"
#include "renderer_p.moc"
