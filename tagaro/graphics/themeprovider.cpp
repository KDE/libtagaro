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

#include "themeprovider.h"
#include "graphicsdelegate_p.h"
#include "renderbackend.h"
#include "settings.h"
#include "sprite.h"
#include "sprite_p.h"
#include "theme.h"

#include <QtCore/QAbstractListModel>
#include <QtCore/QFileInfo>
#include <QtCore/QThreadPool>
#include <QtCore/QVector>
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KGlobal>
#include <KDE/KStandardDirs>

//BEGIN Tagaro::ThemeProvider

class Tagaro::ThemeProvider::Private : public QAbstractListModel
{
	public:
		Tagaro::ThemeProvider* q;
		bool m_ownThemes;
		Tagaro::RenderBehavior m_behavior;
		QHash<QString, Tagaro::Sprite*> m_sprites;

		QList<Tagaro::Theme*> m_themes;
		QList<const Tagaro::Theme*> m_cThemes;
		const Tagaro::Theme* m_selectedTheme;

		Private(Tagaro::ThemeProvider* q, bool ownThemes, const Tagaro::RenderBehavior& behavior) : QAbstractListModel(q), q(q), m_ownThemes(ownThemes), m_behavior(behavior), m_selectedTheme(0) {}

		virtual QVariant data(const QModelIndex& index, int role) const;
		virtual Qt::ItemFlags flags(const QModelIndex& index) const;
		virtual int rowCount(const QModelIndex& index) const;
	private:
		friend class Tagaro::ThemeProvider;
};

QVariant Tagaro::ThemeProvider::Private::data(const QModelIndex& index, int role) const
{
	const Tagaro::Theme* theme = m_cThemes.value(index.row());
	if (!theme)
	{
		return QVariant();
	}
	switch (role)
	{
		case Qt::DisplayRole:
			return theme->name();
		case Qt::DecorationRole:
			return theme->preview();
		case Tagaro::GraphicsDelegate::DescriptionRole:
			return theme->description();
		case Tagaro::GraphicsDelegate::AuthorRole:
			return theme->author();
		case Tagaro::GraphicsDelegate::AuthorEmailRole:
			return theme->authorEmail();
		default:
			return QVariant();
	}
}

Qt::ItemFlags Tagaro::ThemeProvider::Private::flags(const QModelIndex& index) const
{
	Q_UNUSED(index)
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

int Tagaro::ThemeProvider::Private::rowCount(const QModelIndex& index) const
{
	return index.isValid() ? 0 : m_cThemes.count();
}

Tagaro::ThemeProvider::ThemeProvider(bool ownThemes, const Tagaro::RenderBehavior& behavior, QObject* parent)
	: QObject(parent)
	, d(new Private(this, ownThemes, behavior))
{
}

Tagaro::ThemeProvider::~ThemeProvider()
{
	//cleanup sprites (without qDeleteAll because that's not a friend of Sprite)
	QHash<QString, Tagaro::Sprite*>::const_iterator it1 = d->m_sprites.constBegin(),
	                                                it2 = d->m_sprites.constEnd();
	for (; it1 != it2; ++it1)
		delete it1.value();
	//cleanup themes
	if (d->m_ownThemes)
	{
		qDeleteAll(d->m_themes);
	}
	//cleanup the rest
	delete d;
}

Tagaro::Sprite* Tagaro::ThemeProvider::sprite(const QString& spriteKey) const
{
	Tagaro::Sprite*& sprite = d->m_sprites[spriteKey];
	if (!sprite)
	{
		//instantiate on first use
		sprite = new Tagaro::Sprite;
		const QPair<const Tagaro::RenderBackend*, QString> renderElement = d->m_selectedTheme->mapSpriteKey(spriteKey);
		sprite->d->setBackend(renderElement.first, renderElement.second);
	}
	return sprite;
}

QAbstractItemModel* Tagaro::ThemeProvider::model() const
{
	return d;
}

const Tagaro::RenderBehavior& Tagaro::ThemeProvider::behavior() const
{
	return d->m_behavior;
}

QList<const Tagaro::Theme*> Tagaro::ThemeProvider::themes() const
{
	return d->m_cThemes;
}

QList<Tagaro::Theme*> Tagaro::ThemeProvider::nonConstThemes()
{
	return d->m_themes;
}

const Tagaro::Theme* Tagaro::ThemeProvider::defaultTheme() const
{
	return d->m_cThemes.value(0);
}

const Tagaro::Theme* Tagaro::ThemeProvider::selectedTheme() const
{
	return d->m_selectedTheme;
}

void Tagaro::ThemeProvider::setSelectedTheme(const Tagaro::Theme* theme)
{
	if (!d->m_cThemes.contains(theme))
	{
		return;
	}
	if (d->m_selectedTheme != theme && theme->isValid())
	{
		//if necessary, clear rendering threads
		if (Tagaro::Settings::useRenderingThreads())
		{
			QThreadPool::globalInstance()->waitForDone(); //TODO: optimize
		}
		//do theme change
		d->m_selectedTheme = theme;
		emit selectedThemeChanged(theme);
		//announce change to sprites
		QHash<QString, Tagaro::Sprite*>::const_iterator it1 = d->m_sprites.constBegin(), it2 = d->m_sprites.constEnd();
		for (; it1 != it2; ++it1)
		{
			const QPair<const Tagaro::RenderBackend*, QString> renderElement = theme->mapSpriteKey(it1.key());
			it1.value()->d->setBackend(renderElement.first, renderElement.second);
		}
	}
}

void Tagaro::ThemeProvider::setThemes(const QList<Tagaro::Theme*>& themes)
{
	if (d->m_themes == themes)
	{
		return;
	}
	emit themesAboutToBeChanged();
	//find themes which can be deleted
	if (d->m_ownThemes)
	{
		//keep only these themes which are still used
		for (int i = 0; i < themes.size(); ++i)
		{
			d->m_themes.removeAll(themes[i]);
		}
	}
	else
	{
		d->m_themes.clear();
	}
	const QList<Tagaro::Theme*> deleteableThemes = d->m_themes;
	//update theme list (and the const variant, which is built only once for speed)
	d->m_themes = themes;
	d->m_cThemes.clear();
	for (int i = 0; i < themes.size(); ++i)
	{
		d->m_cThemes << themes[i];
	}
	//update theme selection
	const Tagaro::Theme* oldSelected = d->m_selectedTheme;
	if (!d->m_selectedTheme)
	{
		if (d->m_themes.size() > 0)
		{
			d->m_selectedTheme = defaultTheme();
		}
	}
	else if (!d->m_cThemes.contains(d->m_selectedTheme))
	{
		//this also sets selection to null if d->m_themes.isEmpty()
		d->m_selectedTheme = defaultTheme();
	}
	//announce changes
	emit themesChanged();
	if (d->m_selectedTheme != oldSelected)
	{
		emit selectedThemeChanged(d->m_selectedTheme);
	}
	//cleanup
	qDeleteAll(deleteableThemes);
}

//END Tagaro::ThemeProvider
//BEGIN Tagaro::StandardThemeProvider

struct Tagaro::StandardThemeProvider::Private
{
	Tagaro::StandardThemeProvider* m_parent;
	QList<Tagaro::Theme*> m_themes;
	QByteArray m_configKey;

	Private(const QByteArray& configKey, Tagaro::StandardThemeProvider* parent) : m_parent(parent), m_configKey(configKey) {}
};

Tagaro::StandardThemeProvider::StandardThemeProvider(const QByteArray& configKey, const QByteArray& ksdResource, const QString& ksdDirectory_, const Tagaro::RenderBehavior& behavior, QObject* parent)
	: Tagaro::ThemeProvider(true, behavior, parent)
	, d(new Private(configKey, this))
{
	static const QString defaultTheme = QLatin1String("default.desktop");
	const QString ksdDirectory = ksdDirectory_ + QChar('/');
	//read my configuration
	KConfigGroup config(KGlobal::config(), "Tagaro::StandardThemeProvider");
	const QByteArray selectedTheme = config.readEntry(configKey.data(), (ksdDirectory + defaultTheme).toUtf8());
	//find themes
	const QStringList themePaths = KGlobal::dirs()->findAllResources(
		ksdResource, ksdDirectory + "*.desktop",
		KStandardDirs::NoDuplicates
	);
	//TODO cleanup StandardTheme and StandardThemeProvider, integrate KNS
	//create themes
	foreach (const QString& themePath, themePaths)
	{
		const QString themeFile = QFileInfo(themePath).fileName();
		Tagaro::Theme* theme = new Tagaro::StandardTheme(ksdResource, ksdDirectory, themeFile, this);
		//insert theme into list, place theme with hard-coded default name at the beginning
		if (themeFile == defaultTheme)
		{
			d->m_themes.prepend(theme);
		}
		else
		{
			d->m_themes.append(theme);
		}
		setThemes(d->m_themes);
	}
	//set selection index - if selected theme cannot be found, fall back to "default.desktop", then to index 0 (i.e. do nothing because that's the default already)
	for (int i = 0; i < d->m_themes.count(); ++i)
	{
		if (d->m_themes[i]->identifier() == selectedTheme)
		{
			setSelectedTheme(d->m_themes[i]);
		}
	}
}

Tagaro::StandardThemeProvider::~StandardThemeProvider()
{
	delete d;
}

void Tagaro::StandardThemeProvider::setSelectedTheme(const Tagaro::Theme* theme)
{
	Tagaro::ThemeProvider::setSelectedTheme(theme);
	KConfigGroup config(KGlobal::config(), "Tagaro::StandardThemeProvider");
	config.writeEntry(d->m_configKey.data(), selectedTheme()->identifier());
	KGlobal::config()->sync();
}

//END Tagaro::StandardThemeProvider
//BEGIN Tagaro::SimpleThemeProvider

struct Tagaro::SimpleThemeProvider::Private
{
	QList<Tagaro::Theme*> m_themes;
};

Tagaro::SimpleThemeProvider::SimpleThemeProvider(const Tagaro::RenderBehavior& behavior, QObject* parent)
	: Tagaro::ThemeProvider(true, behavior, parent)
	, d(new Private)
{
}

Tagaro::SimpleThemeProvider::~SimpleThemeProvider()
{
	delete d;
}

void Tagaro::SimpleThemeProvider::addTheme(Tagaro::Theme* theme)
{
	setThemes(d->m_themes << theme);
}

//END Tagaro::SimpleThemeProvider

#include "themeprovider.moc"
