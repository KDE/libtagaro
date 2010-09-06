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
#include "theme.h"

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

//BEGIN Tagaro::ThemeProvider

struct Tagaro::ThemeProvider::Private
{
	int m_selectedIndex;
	QStack<Operation> m_activeOps;

	Private() : m_selectedIndex(0) {}
};

Tagaro::ThemeProvider::ThemeProvider(QObject* parent)
	: QObject(parent)
	, d(new Private)
{
}

Tagaro::ThemeProvider::~ThemeProvider()
{
	delete d;
}

int Tagaro::ThemeProvider::selectedIndex() const
{
	return d->m_selectedIndex;
}

void Tagaro::ThemeProvider::setSelectedIndex(int index)
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

const Tagaro::Theme* Tagaro::ThemeProvider::theme(const QByteArray& identifier) const
{
	const int count = themeCount();
	for (int i = 0; i < count; ++i)
	{
		const Tagaro::Theme* theme = this->theme(i);
		if (theme->identifier() == identifier)
		{
			return theme;
		}
	}
	//found no theme with this identifier
	return 0;
}

void Tagaro::ThemeProvider::announceChange(int firstIndex, int lastIndex)
{
	//validate
	Q_ASSERT(firstIndex <= lastIndex);
	//announce
	emit themesChanged(firstIndex, lastIndex);
}

void Tagaro::ThemeProvider::beginInsertThemes(int firstIndex, int lastIndex)
{
	//validate
	const int count = themeCount();
	Q_ASSERT(0 <= firstIndex && firstIndex <= lastIndex && lastIndex < count + (lastIndex - firstIndex + 1));
	Operation op = { ThemeInsert, firstIndex, lastIndex, count };
	d->m_activeOps.push(op);
	//announce
	emit themesAboutToBeInserted(firstIndex, lastIndex);
}

void Tagaro::ThemeProvider::beginRemoveThemes(int firstIndex, int lastIndex)
{
	//validate
	const int count = themeCount();
	Q_ASSERT(0 <= firstIndex && firstIndex <= lastIndex && lastIndex < count);
	Operation op = { ThemeRemove, firstIndex, lastIndex, count };
	d->m_activeOps.push(op);
	//announce
	emit themesAboutToBeRemoved(firstIndex, lastIndex);
}

void Tagaro::ThemeProvider::endInsertThemes()
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

void Tagaro::ThemeProvider::endRemoveThemes()
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

//END Tagaro::ThemeProvider
//BEGIN Tagaro::DesktopThemeProvider

struct Tagaro::DesktopThemeProvider::Private
{
	Tagaro::DesktopThemeProvider* m_parent;
	QVector<Tagaro::Theme*> m_themes;
	QByteArray m_configKey;

	Private(const QByteArray& configKey, Tagaro::DesktopThemeProvider* parent) : m_parent(parent), m_configKey(configKey) {}
	~Private() { qDeleteAll(m_themes); }

	void _k_saveSelectedIndex(int index);
};

Tagaro::DesktopThemeProvider::DesktopThemeProvider(const QByteArray& configKey, const QByteArray& ksdResource, const QString& ksdDirectory_, QObject* parent)
	: Tagaro::ThemeProvider(parent)
	, d(new Private(configKey, this))
{
	static const QString defaultTheme = QLatin1String("default.desktop");
	const QString ksdDirectory = ksdDirectory_ + QChar('/');
	//read my configuration
	KConfigGroup config(KGlobal::config(), "Tagaro::DesktopThemeProvider");
	const QByteArray selectedTheme = config.readEntry(configKey.data(), (ksdDirectory + defaultTheme).toUtf8());
	//find themes
	const QStringList themePaths = KGlobal::dirs()->findAllResources(
		ksdResource, ksdDirectory + "*.desktop",
		KStandardDirs::NoDuplicates
	);
	foreach (const QString& themePath, themePaths)
	{
		const QString themeFile = QFileInfo(themePath).fileName();
		//create theme from configuration
		Tagaro::Theme* theme = new Tagaro::Theme((ksdDirectory + themeFile).toUtf8());
		KConfig themeConfigFile(themePath, KConfig::SimpleConfig);
		KConfigGroup themeConfig(&themeConfigFile, "KGameTheme");
		//read standard properties
		const QString graphicsFile = themeConfig.readEntry("FileName", QString());
		theme->setData(Tagaro::Theme::GraphicsFileRole, KStandardDirs::locate(ksdResource, ksdDirectory + graphicsFile));
		theme->setData(Tagaro::Theme::ThemeFileRole, themePath);
		theme->setData(Tagaro::Theme::NameRole, themeConfig.readEntry("Name", QString()));
		theme->setData(Tagaro::Theme::DescriptionRole, themeConfig.readEntry("Description", QString()));
		theme->setData(Tagaro::Theme::AuthorRole, themeConfig.readEntry("Author", QString()));
		theme->setData(Tagaro::Theme::AuthorEmailRole, themeConfig.readEntry("AuthorEmail", QString()));
		const QString previewFile = themeConfig.readEntry("Preview", QString());
		theme->setData(Tagaro::Theme::PreviewRole, QPixmap(KStandardDirs::locate(ksdResource, ksdDirectory + previewFile)));
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
	connect(this, SIGNAL(selectedIndexChanged(int)), SLOT(_k_saveSelectedIndex(int)));
}

Tagaro::DesktopThemeProvider::~DesktopThemeProvider()
{
	delete d;
}

int Tagaro::DesktopThemeProvider::themeCount() const
{
	return d->m_themes.count();
}

const Tagaro::Theme* Tagaro::DesktopThemeProvider::theme(int index) const
{
	return d->m_themes.value(index, 0);
}

void Tagaro::DesktopThemeProvider::Private::_k_saveSelectedIndex(int index)
{
	const Tagaro::Theme* theme = m_parent->Tagaro::DesktopThemeProvider::theme(index);
	if (!theme)
	{
		return;
	}
	KConfigGroup config(KGlobal::config(), "Tagaro::DesktopThemeProvider");
	config.writeEntry(m_configKey.data(), theme->identifier());
	KGlobal::config()->sync();
}

//END Tagaro::DesktopThemeProvider

//BEGIN Tagaro::FileThemeProvider

static QByteArray FileThemeProvider_identifier(const QString& file)
{
	return file.section(QChar('/'), -1).toUtf8();
}

struct Tagaro::FileThemeProvider::Private
{
	Tagaro::Theme m_theme;

	Private(const QByteArray& key) : m_theme(key) {}
};

Tagaro::FileThemeProvider::FileThemeProvider(const QString& file, QObject* parent)
	: Tagaro::ThemeProvider(parent)
	, d(new Private(FileThemeProvider_identifier(file)))
{
	d->m_theme.setData(Tagaro::Theme::GraphicsFileRole, file);
}

Tagaro::FileThemeProvider::~FileThemeProvider()
{
	delete d;
}

int Tagaro::FileThemeProvider::themeCount() const
{
	return 1;
}

const Tagaro::Theme* Tagaro::FileThemeProvider::theme(int index) const
{
	Q_UNUSED(index)
	return &d->m_theme;
}

//END Tagaro::FileThemeProvider

#include "themeprovider.moc"
