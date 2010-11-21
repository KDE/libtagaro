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

#include <QtCore/QAbstractListModel>
#include <QtCore/QFileInfo>
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
		int m_selectedIndex;

		Private(Tagaro::ThemeProvider* q_) : QAbstractListModel(q_), q(q_), m_selectedIndex(0) {}

		virtual QVariant data(const QModelIndex& index, int role) const;
		virtual Qt::ItemFlags flags(const QModelIndex& index) const;
		virtual int rowCount(const QModelIndex& index) const;
	private:
		friend class Tagaro::ThemeProvider;
};

QVariant Tagaro::ThemeProvider::Private::data(const QModelIndex& index, int role) const
{
	const Tagaro::Theme* theme = q->theme(index.row());
	return theme ? theme->data(role) : QVariant();
}

Qt::ItemFlags Tagaro::ThemeProvider::Private::flags(const QModelIndex& index) const
{
	Q_UNUSED(index)
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

int Tagaro::ThemeProvider::Private::rowCount(const QModelIndex& index) const
{
	return index.isValid() ? 0 : q->themeCount();
}

Tagaro::ThemeProvider::ThemeProvider(QObject* parent)
	: QObject(parent)
	, d(new Private(this))
{
}

Tagaro::ThemeProvider::~ThemeProvider()
{
	delete d;
}

QAbstractItemModel* Tagaro::ThemeProvider::model() const
{
	return d;
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
	const int themeCount = this->themeCount();
	firstIndex = qBound(0, firstIndex, themeCount);
	lastIndex = qBound(firstIndex, lastIndex, themeCount);
	emit d->dataChanged(d->index(firstIndex), d->index(lastIndex));
}

void Tagaro::ThemeProvider::beginInsertThemes(int firstIndex, int lastIndex)
{
	d->beginInsertRows(QModelIndex(), firstIndex, lastIndex);
}

void Tagaro::ThemeProvider::beginRemoveThemes(int firstIndex, int lastIndex)
{
	d->beginRemoveRows(QModelIndex(), firstIndex, lastIndex);
}

void Tagaro::ThemeProvider::endInsertThemes()
{
	d->endInsertRows();
}

void Tagaro::ThemeProvider::endRemoveThemes()
{
	d->endRemoveRows();
}

//END Tagaro::ThemeProvider
//BEGIN Tagaro::StandardThemeProvider

struct Tagaro::StandardThemeProvider::Private
{
	Tagaro::StandardThemeProvider* m_parent;
	QVector<Tagaro::Theme*> m_themes;
	QByteArray m_configKey;

	Private(const QByteArray& configKey, Tagaro::StandardThemeProvider* parent) : m_parent(parent), m_configKey(configKey) {}
	~Private() { qDeleteAll(m_themes); }

	void _k_saveSelectedIndex(int index);
};

Tagaro::StandardThemeProvider::StandardThemeProvider(const QByteArray& configKey, const QByteArray& ksdResource, const QString& ksdDirectory_, QObject* parent)
	: Tagaro::ThemeProvider(parent)
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
	//create themes
	foreach (const QString& themePath, themePaths)
	{
		const QString themeFile = QFileInfo(themePath).fileName();
		Tagaro::Theme* theme = new Tagaro::StandardTheme(ksdResource, ksdDirectory, themeFile);
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

Tagaro::StandardThemeProvider::~StandardThemeProvider()
{
	delete d;
}

int Tagaro::StandardThemeProvider::themeCount() const
{
	return d->m_themes.count();
}

const Tagaro::Theme* Tagaro::StandardThemeProvider::theme(int index) const
{
	return d->m_themes.value(index, 0);
}

void Tagaro::StandardThemeProvider::Private::_k_saveSelectedIndex(int index)
{
	const Tagaro::Theme* theme = m_parent->Tagaro::StandardThemeProvider::theme(index);
	if (!theme)
	{
		return;
	}
	KConfigGroup config(KGlobal::config(), "Tagaro::StandardThemeProvider");
	config.writeEntry(m_configKey.data(), theme->identifier());
	KGlobal::config()->sync();
}

//END Tagaro::StandardThemeProvider

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
