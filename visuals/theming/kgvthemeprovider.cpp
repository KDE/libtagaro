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

#include "kgvthemeprovider.h"
#include "kgvtheme.h"

#include <QtCore/QFileInfo>
#include <QtCore/QStack>
#include <QtGui/QPixmap>
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KGlobal>
#include <KDE/KStandardDirs>

enum OperationId { ThemeInsert, ThemeRemove };

struct Operation
{
	OperationId id; //'+' for insertions, '-' for removals
	int first, last, oldCount;
};

//BEGIN KgvThemeProvider

struct KgvThemeProvider::Private
{
	int m_selectedIndex;
	QStack<Operation> m_activeOps;

	Private() : m_selectedIndex(0) {}
};

KgvThemeProvider::KgvThemeProvider(QObject* parent)
	: QObject(parent)
	, d(new Private)
{
}

KgvThemeProvider::~KgvThemeProvider()
{
	delete d;
}

int KgvThemeProvider::selectedIndex() const
{
	return d->m_selectedIndex;
}

void KgvThemeProvider::setSelectedIndex(int index)
{
	if (index < 0 || index >= themeCount())
	{
		//out of bounds
		return;
	}
	if (d->m_selectedIndex != index)
	{
		d->m_selectedIndex = index;
		emit selectedIndexChanged(index);
	}
}

const KgvTheme* KgvThemeProvider::theme(const QByteArray& identifier) const
{
	const int count = themeCount();
	for (int i = 0; i < count; ++i)
	{
		const KgvTheme* theme = this->theme(i);
		if (theme->identifier() == identifier)
		{
			return theme;
		}
	}
	//found no theme with this identifier
	return 0;
}

void KgvThemeProvider::announceChange(int firstIndex, int lastIndex)
{
	//validate
	Q_ASSERT(firstIndex <= lastIndex);
	//announce
	emit themesChanged(firstIndex, lastIndex);
}

void KgvThemeProvider::beginInsertThemes(int firstIndex, int lastIndex)
{
	//validate
	const int count = themeCount();
	Q_ASSERT(0 <= firstIndex && firstIndex <= lastIndex && lastIndex < count + (lastIndex - firstIndex + 1));
	Operation op = { ThemeInsert, firstIndex, lastIndex, count };
	d->m_activeOps.push(op);
	//announce
	emit themesAboutToBeInserted(firstIndex, lastIndex);
}

void KgvThemeProvider::beginRemoveThemes(int firstIndex, int lastIndex)
{
	//validate
	const int count = themeCount();
	Q_ASSERT(0 <= firstIndex && firstIndex <= lastIndex && lastIndex < count);
	Operation op = { ThemeRemove, firstIndex, lastIndex, count };
	d->m_activeOps.push(op);
	//announce
	emit themesAboutToBeRemoved(firstIndex, lastIndex);
}

void KgvThemeProvider::endInsertThemes()
{
	//validate
	Q_ASSERT(!d->m_activeOps.isEmpty());
	const Operation op = d->m_activeOps.pop();
	Q_ASSERT(op.id == ThemeInsert);
	const int countDiff = op.last - op.first + 1;
	Q_ASSERT(frameCount() == op.oldCount + countDiff);
	//update selection index
	if (d->m_selectedIndex >= op.first)
	{
		d->m_selectedIndex += countDiff;
		emit selectedIndexChanged(d->m_selectedIndex);
	}
	//announce
	emit themesInserted(op.first, op.last);
}

void KgvThemeProvider::endRemoveThemes()
{
	//validate
	Q_ASSERT(!d->m_activeOps.isEmpty());
	const Operation op = d->m_activeOps.pop();
	Q_ASSERT(op.id == ThemeRemove);
	const int countDiff = op.last - op.first + 1;
	Q_ASSERT(frameCount() == op.oldCount - countDiff);
	//update selection index
	if (d->m_selectedIndex >= op.first)
	{
		if (d->m_selectedIndex <= op.last)
		{
			//theme has been removed, so reset selection index to first theme
			d->m_selectedIndex = 0;
		}
		else
		{
			d->m_selectedIndex -= countDiff;
		}
		emit selectedIndexChanged(d->m_selectedIndex);
	}
	//announce
	emit themesRemoved(op.first, op.last);
}

//END KgvThemeProvider
//BEGIN KgvDesktopThemeProvider

struct KvgDtpInstantiator : public QHash<QByteArray, KgvDesktopThemeProvider*>
{
	//This class owns (and therefore deletes) all KgvDesktopThemeProvider instances.
	~KvgDtpInstantiator()
	{
		//cannot do this with qDeleteAll() because this method may not call the destructor
		iterator it1 = begin(), it2 = end();
		for (; it1 != it2; ++it1)
		{
			delete it1.value();
		}
	}
};

K_GLOBAL_STATIC(KvgDtpInstantiator, g_instantiator)

KgvDesktopThemeProvider* KgvDesktopThemeProvider::instance(const QByteArray& key)
{
	KgvDesktopThemeProvider*& provider = (*g_instantiator)[key];
	if (!provider) //provider == 0 on first access -> instance needs to be created
	{
		//Because we obtained a reference from the hash, this assignment will also store the new instance there.
		provider = new KgvDesktopThemeProvider(key);
	}
	return provider;
}

struct KgvDesktopThemeProvider::Private
{
	QByteArray m_key;
	KgvDesktopThemeProvider* m_parent;
	QVector<KgvTheme*> m_themes;

	Private(const QByteArray& key, KgvDesktopThemeProvider* parent) : m_key(key), m_parent(parent) {}
	~Private() { qDeleteAll(m_themes); }

	void saveSelectedIndex(int index);
};

KgvDesktopThemeProvider::KgvDesktopThemeProvider(const QByteArray& key)
	: d(new Private(key, this))
{
	static const QString defaultTheme = QLatin1String("default.desktop");
	//read my configuration
	KConfigGroup mainConfig(KGlobal::config(), "KgvDesktopThemeProvider");
	KConfigGroup myConfig(&mainConfig, key.constData());
	const QByteArray resourceType = myConfig.readEntry("ResourceType", QByteArray("appdata"));
	const char* const resourceType_ = resourceType.constData();
	QString directory = myConfig.readEntry("Directory", QString::fromLatin1("themes"));
	if (!directory.endsWith(QChar('/')))
		directory += QChar('/');
	const QByteArray selectedTheme = myConfig.readEntry("Selected", (directory + defaultTheme).toUtf8());
	//find themes
	const QStringList themePaths = KGlobal::dirs()->findAllResources(
		resourceType_, directory + "*.desktop",
		KStandardDirs::NoDuplicates
	);
	foreach (const QString& themePath, themePaths)
	{
		const QString themeFile = QFileInfo(themePath).fileName();
		//create theme from configuration
		KgvTheme* theme = new KgvTheme((directory + themeFile).toUtf8());
		KConfig themeConfigFile(themePath, KConfig::SimpleConfig);
		KConfigGroup themeConfig(&themeConfigFile, "KGameTheme");
		//read standard properties
		const QString graphicsFile = themeConfig.readEntry("FileName", QString());
		theme->setData(KgvTheme::GraphicsFileRole, KStandardDirs::locate(resourceType_, directory + graphicsFile));
		theme->setData(KgvTheme::ThemeFileRole, themePath);
		theme->setData(KgvTheme::NameRole, themeConfig.readEntry("Name", QString()));
		theme->setData(KgvTheme::DescriptionRole, themeConfig.readEntry("Description", QString()));
		theme->setData(KgvTheme::AuthorRole, themeConfig.readEntry("Author", QString()));
		theme->setData(KgvTheme::AuthorEmailRole, themeConfig.readEntry("AuthorEmail", QString()));
		const QString previewFile = themeConfig.readEntry("Preview", QString());
		theme->setData(KgvTheme::PreviewRole, QPixmap(KStandardDirs::locate(resourceType_, directory + previewFile)));
		//write everything except for the standard properties into the theme's custom data
		//some applications use this for per-theme configuration values
		QMap<QString, QString> entryMap = themeConfig.entryMap();
		QMap<QString, QString>::const_iterator it1 = entryMap.constBegin(), it2 = entryMap.constEnd();
		for (; it1 != it2; ++it1)
		{
			theme->setData(it1.key().toUtf8(), it1.value());
		}
		//insert theme into list, place theme with hard-coded default name at the beginning
		if (themeFile == defaultTheme)
		{
			d->m_themes.prepend(theme);
		}
		else
		{
			d->m_themes.append(theme);
		}
	}
	//set selection index - if selected theme cannot be found, fall back to "default.desktop", then to index 0 (i.e. do nothing because that's the default already)
	for (int i = 0; i < d->m_themes.count(); ++i)
	{
		if (d->m_themes[i]->identifier() == selectedTheme)
		{
			setSelectedIndex(i);
		}
	}
	connect(this, SIGNAL(selectedIndexChanged(int)), SLOT(saveSelectedIndex(int)));
}

KgvDesktopThemeProvider::~KgvDesktopThemeProvider()
{
	delete d;
}

int KgvDesktopThemeProvider::themeCount() const
{
	return d->m_themes.count();
}

const KgvTheme* KgvDesktopThemeProvider::theme(int index) const
{
	return d->m_themes.value(index, 0);
}

void KgvDesktopThemeProvider::Private::saveSelectedIndex(int index)
{
	const KgvTheme* theme = m_parent->KgvDesktopThemeProvider::theme(index);
	if (!theme)
	{
		return;
	}
	KConfigGroup mainConfig(KGlobal::config(), "KgvDesktopThemeProvider");
	KConfigGroup myConfig(&mainConfig, m_key.constData());
	myConfig.writeEntry("Selected", theme->identifier());
	KGlobal::config()->sync();
}

//END KgvDesktopThemeProvider

//BEGIN KgvFileThemeProvider

static QByteArray KgvFileThemeProvider_identifier(const QString& file)
{
	return file.section(QChar('/'), -1).toUtf8();
}

struct KgvFileThemeProvider::Private
{
	KgvTheme m_theme;

	Private(const QByteArray& key) : m_theme(key) {}
};

KgvFileThemeProvider::KgvFileThemeProvider(const QString& file, QObject* parent)
	: KgvThemeProvider(parent)
	, d(new Private(KgvFileThemeProvider_identifier(file)))
{
	d->m_theme.setData(KgvTheme::GraphicsFileRole, file);
}

KgvFileThemeProvider::~KgvFileThemeProvider()
{
	delete d;
}

int KgvFileThemeProvider::themeCount() const
{
	return 1;
}

const KgvTheme* KgvFileThemeProvider::theme(int index) const
{
	Q_UNUSED(index)
	return &d->m_theme;
}

//END KgvFileThemeProvider

#include "kgvthemeprovider.moc"
