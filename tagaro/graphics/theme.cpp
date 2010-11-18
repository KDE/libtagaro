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

#include "theme.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QPixmap>
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KStandardDirs>

//BEGIN Tagaro::Theme

struct Tagaro::Theme::Private
{
	QByteArray m_identifier;
	QHash<Tagaro::Theme::Role, QVariant> m_values;
	QHash<QByteArray, QVariant> m_customValues;

	Private(const QByteArray& identifier) : m_identifier(identifier) {}
};

Tagaro::Theme::Theme(const QByteArray& identifier)
	: d(new Private(identifier))
{
}

Tagaro::Theme::~Theme()
{
	delete d;
}

QVariant Tagaro::Theme::data(Tagaro::Theme::Role role, const QVariant& defaultValue) const
{
	return d->m_values.value(role, defaultValue);
}

QVariant Tagaro::Theme::data(const QByteArray& key, const QVariant& defaultValue) const
{
	return d->m_customValues.value(key, defaultValue);
}

void Tagaro::Theme::setData(Tagaro::Theme::Role role, const QVariant& value)
{
	d->m_values.insert(role, value);
}

void Tagaro::Theme::setData(const QByteArray& key, const QVariant& value)
{
	d->m_customValues.insert(key, value);
}

QByteArray Tagaro::Theme::identifier() const
{
	return d->m_identifier;
}

uint Tagaro::Theme::modificationTimestamp() const
{
	const QFileInfo svgFile(d->m_values.value(Tagaro::Theme::GraphicsFileRole).toString());
	const QFileInfo themeFile(d->m_values.value(Tagaro::Theme::ThemeFileRole).toString());
	uint timestamp = svgFile.lastModified().toTime_t();
	if (themeFile.exists())
		timestamp = qMax(timestamp, themeFile.lastModified().toTime_t());
	return timestamp;
}

//END Tagaro::Theme
//BEGIN Tagaro::StandardTheme

struct Tagaro::StandardTheme::Private
{
	QByteArray m_ksdResource;
	QString m_ksdDirectory;
	QDir m_directory;

	void init(Tagaro::StandardTheme* theme, const QString& filePath);
	QString resolvePath(const QString& path) const;
};

Tagaro::StandardTheme::StandardTheme(const QString& filePath)
	: Tagaro::Theme(QFileInfo(filePath).absolutePath().toUtf8())
	, d(new Private)
{
	d->m_directory = QFileInfo(filePath).dir();
	d->init(this, filePath);
}

static const QByteArray dftIdentifier(const QString& ksdDirectory, const QString& fileName)
{
	if (ksdDirectory.endsWith(QChar('/')))
		return (ksdDirectory + fileName).toUtf8();
	else
		return (ksdDirectory + QChar('/') + fileName).toUtf8();
}

Tagaro::StandardTheme::StandardTheme(const QByteArray& ksdResource, const QString& ksdDirectory, const QString& fileName)
	: Tagaro::Theme(dftIdentifier(ksdDirectory, fileName))
	, d(new Private)
{
	d->m_ksdResource = ksdResource;
	d->m_ksdDirectory = ksdDirectory.endsWith(QChar('/')) ? ksdDirectory : (ksdDirectory + QChar('/'));
	d->init(this, KStandardDirs::locate(ksdResource, d->m_ksdDirectory + fileName));
}

void Tagaro::StandardTheme::Private::init(Tagaro::StandardTheme* theme, const QString& filePath)
{
	//open configuration
	KConfig themeConfigFile(filePath, KConfig::SimpleConfig);
	KConfigGroup themeConfig(&themeConfigFile, "KGameTheme");
	//read standard properties
	const QString graphicsFile = themeConfig.readEntry("FileName", QString());
	theme->setData(Tagaro::Theme::GraphicsFileRole, KStandardDirs::locate(m_ksdResource, m_ksdDirectory + graphicsFile));
	theme->setData(Tagaro::Theme::ThemeFileRole, filePath);
	theme->setData(Tagaro::Theme::NameRole, themeConfig.readEntry("Name", QString()));
	theme->setData(Tagaro::Theme::DescriptionRole, themeConfig.readEntry("Description", QString()));
	theme->setData(Tagaro::Theme::AuthorRole, themeConfig.readEntry("Author", QString()));
	theme->setData(Tagaro::Theme::AuthorEmailRole, themeConfig.readEntry("AuthorEmail", QString()));
	const QString previewFile = themeConfig.readEntry("Preview", QString());
	theme->setData(Tagaro::Theme::PreviewRole, QPixmap(KStandardDirs::locate(m_ksdResource, m_ksdDirectory + previewFile)));
	//write everything except for the standard properties into the theme's custom data
	//some applications use this for per-theme configuration values
	QMap<QString, QString> entryMap = themeConfig.entryMap();
	QMap<QString, QString>::const_iterator it1 = entryMap.constBegin(), it2 = entryMap.constEnd();
	for (; it1 != it2; ++it1)
	{
		theme->setData(it1.key().toUtf8(), it1.value());
	}
}

QString Tagaro::StandardTheme::Private::resolvePath(const QString& path) const
{
	if (QFileInfo(path).isAbsolute())
		return path;
	//resolve relative paths with what was given in the ctor
	if (m_ksdResource.isEmpty())
		return m_directory.absoluteFilePath(path);
	else
		return KStandardDirs::locate(m_ksdResource, m_ksdDirectory + path);
}

//END Tagaro::StandardTheme
