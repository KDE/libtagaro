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
#include "graphicssource.h"
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

//BEGIN KGame::ThemeProvider

class KGame::ThemeProvider::Private : public QAbstractListModel
{
	public:
		KGame::ThemeProvider* q;
		bool m_ownThemes;
		KGame::GraphicsSourceConfig m_config;
		QHash<QString, KGame::Sprite*> m_sprites;

		QList<KGame::Theme*> m_themes;
		QList<const KGame::Theme*> m_cThemes;
		const KGame::Theme* m_selectedTheme;

		Private(KGame::ThemeProvider* q, bool ownThemes, const KGame::GraphicsSourceConfig& config) : QAbstractListModel(q), q(q), m_ownThemes(ownThemes), m_config(config), m_selectedTheme(0) {}

		virtual QVariant data(const QModelIndex& index, int role) const;
		virtual Qt::ItemFlags flags(const QModelIndex& index) const;
		virtual int rowCount(const QModelIndex& index) const;
	private:
		friend class KGame::ThemeProvider;
};

QVariant KGame::ThemeProvider::Private::data(const QModelIndex& index, int role) const
{
	const KGame::Theme* theme = m_cThemes.value(index.row());
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
		case KGame::GraphicsDelegate::DescriptionRole:
			return theme->description();
		case KGame::GraphicsDelegate::AuthorRole:
			return theme->author();
		case KGame::GraphicsDelegate::AuthorEmailRole:
			return theme->authorEmail();
		default:
			return QVariant();
	}
}

Qt::ItemFlags KGame::ThemeProvider::Private::flags(const QModelIndex& index) const
{
	Q_UNUSED(index)
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

int KGame::ThemeProvider::Private::rowCount(const QModelIndex& index) const
{
	return index.isValid() ? 0 : m_cThemes.count();
}

KGame::ThemeProvider::ThemeProvider(bool ownThemes, QObject* parent, const KGame::GraphicsSourceConfig& config)
	: QObject(parent)
	, d(new Private(this, ownThemes, config))
{
}

KGame::ThemeProvider::~ThemeProvider()
{
	//cleanup sprites (without qDeleteAll because that's not a friend of Sprite)
	QHash<QString, KGame::Sprite*>::const_iterator it1 = d->m_sprites.constBegin(),
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

KGame::Sprite* KGame::ThemeProvider::sprite(const QString& spriteKey) const
{
	KGame::Sprite*& sprite = d->m_sprites[spriteKey];
	if (!sprite)
	{
		//instantiate on first use
		sprite = new KGame::Sprite;
		if (d->m_selectedTheme)
		{
			const QPair<const KGame::GraphicsSource*, QString> renderElement = d->m_selectedTheme->mapSpriteKey(spriteKey);
			sprite->d->setSource(renderElement.first, renderElement.second);
		}
		else
		{
			sprite->d->setSource(0, QString());
		}
	}
	return sprite;
}

QAbstractItemModel* KGame::ThemeProvider::model() const
{
	return d;
}

const KGame::GraphicsSourceConfig& KGame::ThemeProvider::config() const
{
	return d->m_config;
}

QList<const KGame::Theme*> KGame::ThemeProvider::themes() const
{
	return d->m_cThemes;
}

QList<KGame::Theme*> KGame::ThemeProvider::nonConstThemes()
{
	return d->m_themes;
}

const KGame::Theme* KGame::ThemeProvider::defaultTheme() const
{
	return d->m_cThemes.value(0);
}

const KGame::Theme* KGame::ThemeProvider::selectedTheme() const
{
	return d->m_selectedTheme;
}

void KGame::ThemeProvider::setSelectedTheme(const KGame::Theme* theme)
{
	if (!d->m_cThemes.contains(theme))
	{
		return;
	}
	if (d->m_selectedTheme != theme && theme->isValid())
	{
		//if necessary, clear rendering threads
		if (KGame::Settings::useRenderingThreads())
		{
			QThreadPool::globalInstance()->waitForDone(); //TODO: optimize
		}
		//do theme change
		d->m_selectedTheme = theme;
		//announce change to sprites
		QHash<QString, KGame::Sprite*>::const_iterator it1 = d->m_sprites.constBegin(), it2 = d->m_sprites.constEnd();
		for (; it1 != it2; ++it1)
		{
			const QPair<const KGame::GraphicsSource*, QString> renderElement = theme->mapSpriteKey(it1.key());
			it1.value()->d->setSource(renderElement.first, renderElement.second);
		}
		//announce change publicly (AFTER announce to sprites, because slots
		//connected to this signal may want to refetch synchronous pixmaps)
		emit selectedThemeChanged(theme);
	}
}

void KGame::ThemeProvider::setThemes(const QList<KGame::Theme*>& themes)
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
	const QList<KGame::Theme*> deleteableThemes = d->m_themes;
	//update theme list (and the const variant, which is built only once for speed)
	d->m_themes = themes;
	d->m_cThemes.clear();
	for (int i = 0; i < themes.size(); ++i)
	{
		d->m_cThemes << themes[i];
	}
	//announce change
	emit themesChanged();
	//update theme selection
	if (!d->m_selectedTheme)
	{
		if (d->m_themes.size() > 0)
		{
			setSelectedTheme(defaultTheme());
		}
	}
	else if (!d->m_cThemes.contains(d->m_selectedTheme))
	{
		//this also sets selection to null if d->m_themes.isEmpty()
		setSelectedTheme(defaultTheme());
	}
	//cleanup
	qDeleteAll(deleteableThemes);
}

//END KGame::ThemeProvider
//BEGIN KGame::StandardThemeProvider

struct KGame::StandardThemeProvider::Private
{
	KGame::StandardThemeProvider* m_parent;
	QList<KGame::Theme*> m_themes;
	QByteArray m_configKey;

	Private(const QByteArray& configKey, KGame::StandardThemeProvider* parent) : m_parent(parent), m_configKey(configKey) {}
};

KGame::StandardThemeProvider::StandardThemeProvider(const QByteArray& configKey, const QByteArray& ksdResource, const QString& ksdDirectory_, QObject* parent, const KGame::GraphicsSourceConfig& gsConfig)
	: KGame::ThemeProvider(true, parent, gsConfig)
	, d(new Private(configKey, this))
{
	static const QString defaultTheme = QLatin1String("default.desktop");
	const QString ksdDirectory = ksdDirectory_ + QChar('/');
	//read my configuration
	KConfigGroup config(KGlobal::config(), "KGame::StandardThemeProvider");
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
		KGame::Theme* theme = new KGame::StandardTheme(ksdResource, ksdDirectory, themeFile, this);
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

KGame::StandardThemeProvider::~StandardThemeProvider()
{
	delete d;
}

void KGame::StandardThemeProvider::setSelectedTheme(const KGame::Theme* theme)
{
	KGame::ThemeProvider::setSelectedTheme(theme);
	KConfigGroup config(KGlobal::config(), "KGame::StandardThemeProvider");
	config.writeEntry(d->m_configKey.data(), selectedTheme()->identifier());
	KGlobal::config()->sync();
}

//END KGame::StandardThemeProvider
//BEGIN KGame::SimpleThemeProvider

struct KGame::SimpleThemeProvider::Private
{
	QList<KGame::Theme*> m_themes;
};

KGame::SimpleThemeProvider::SimpleThemeProvider(QObject* parent, const KGame::GraphicsSourceConfig& config)
	: KGame::ThemeProvider(true, parent, config)
	, d(new Private)
{
}

KGame::SimpleThemeProvider::~SimpleThemeProvider()
{
	delete d;
}

void KGame::SimpleThemeProvider::addTheme(KGame::Theme* theme)
{
	setThemes(d->m_themes << theme);
}

//END KGame::SimpleThemeProvider

#include "themeprovider.moc"
