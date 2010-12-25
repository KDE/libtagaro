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
#include "renderbackends.h"
#include "themeprovider.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QPixmap>
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KDE/KStandardDirs>

//BEGIN Tagaro::Theme

struct ThemeRoute
{
	QRegExp spriteKey;
	QString elementKey;
	const Tagaro::RenderBackend* backend;
};

struct Tagaro::Theme::Private
{
	QByteArray m_identifier;
	const Tagaro::ThemeProvider* m_provider;
	//metadata
	QString m_name, m_description, m_author, m_authorEmail;
	QPixmap m_preview;
	QMap<QString, QString> m_customData;
	//routing table
	QHash<QByteArray, Tagaro::RenderBackend*> m_backends;
	QList<ThemeRoute> m_routes;

	Private(const QByteArray& identifier, const Tagaro::ThemeProvider* provider) : m_identifier(identifier), m_provider(provider) {}
	~Private() { qDeleteAll(m_backends); }
};

Tagaro::Theme::Theme(const QByteArray& identifier, const Tagaro::ThemeProvider* provider)
	: d(new Private(identifier, provider))
{
}

Tagaro::Theme::~Theme()
{
	delete d;
}

QByteArray Tagaro::Theme::identifier() const
{
	return d->m_identifier;
}

const Tagaro::ThemeProvider* Tagaro::Theme::provider() const
{
	return d->m_provider;
}

bool Tagaro::Theme::isValid() const
{
	QHash<QByteArray, Tagaro::RenderBackend*>::const_iterator it1 = d->m_backends.constBegin(), it2 = d->m_backends.constEnd();
	for (; it1 != it2; ++it1)
	{
		if (!(it1.value() && it1.value()->isValid()))
		{
			return false;
		}
	}
	return true;
}

QString Tagaro::Theme::name() const
{
	return d->m_name;
}

void Tagaro::Theme::setName(const QString& name)
{
	d->m_name = name;
}

QString Tagaro::Theme::description() const
{
	return d->m_description;
}

void Tagaro::Theme::setDescription(const QString& description)
{
	d->m_description = description;
}

QString Tagaro::Theme::author() const
{
	return d->m_author;
}

void Tagaro::Theme::setAuthor(const QString& author)
{
	d->m_author = author;
}

QString Tagaro::Theme::authorEmail() const
{
	return d->m_authorEmail;
}

void Tagaro::Theme::setAuthorEmail(const QString& authorEmail)
{
	d->m_authorEmail = authorEmail;
}

QPixmap Tagaro::Theme::preview() const
{
	return d->m_preview;
}

void Tagaro::Theme::setPreview(const QPixmap& preview)
{
	d->m_preview = preview;
}

QMap<QString, QString> Tagaro::Theme::customData() const
{
	return d->m_customData;
}

void Tagaro::Theme::setCustomData(const QMap<QString, QString>& customData)
{
	d->m_customData = customData;
}

void Tagaro::Theme::addBackend(const QByteArray& identifier_, Tagaro::RenderBackend* backend)
{
	const QByteArray identifier = (identifier_ == "default") ? QByteArray() : identifier_;
	QHash<QByteArray, Tagaro::RenderBackend*>::const_iterator it = d->m_backends.constFind(identifier);
	if (it != d->m_backends.constEnd())
	{
		delete it.value();
	}
	d->m_backends.insert(identifier, backend);
}

static QString resolveRelativePath(const QString& fileName, const QList<QDir>& refDirs)
{
	const int size = refDirs.size();
	//quick path for a) absolute paths and b) no refDirs (assume refDir == QDir(".") then)
	QFileInfo file(fileName);
	if (file.isAbsolute() || size == 0)
	{
		return file.absoluteFilePath();
	}
	//find file in the given reference directories
	for (int i = 0; i < size; ++i)
	{
		const QString absPath = refDirs[i].absoluteFilePath(fileName);
		if (KStandardDirs::exists(absPath))
		{
			return absPath;
		}
	}
	//nothing found in the reference directories - fall back to QDir(".") reference
	return file.absoluteFilePath();
}

void Tagaro::Theme::addBackend(const QByteArray& identifier, const QString& specification, const QList<QDir>& refDirs)
{
	const QChar typeSymbol(':');
	QString spec(specification), type(QLatin1String("auto"));
	//parse type specification
	const int pos1 = spec.indexOf(typeSymbol);
	if (pos1 >= 0)
	{
		type = spec.mid(0, pos1);
		spec = spec.mid(pos1 + 1);
	}
	//further format of specification depends on chosen type, so resolve type if not done yet
	if (type == QLatin1String("auto"))
	{
		QFileInfo file(spec);
		const QStringList suffixes = file.completeSuffix().split(QChar('.'));
		if (suffixes.contains(QLatin1String("svg")) || suffixes.contains(QLatin1String("svgz")))
		{
			type = "svg";
		}
	}
	//resolve specifications - Add new types below here.
	if (type == QLatin1String("svg"))
	{
		const QString path = resolveRelativePath(spec, refDirs);
		Tagaro::QtSvgRenderBackend* svgBackend = new Tagaro::QtSvgRenderBackend(path, d->m_provider->behavior());
		addBackend(identifier, new Tagaro::CachedProxyRenderBackend(svgBackend));
	}
	else
	{
		kDebug() << "Failed to create RenderBackend from specification:" << specification;
		addBackend(identifier, 0);
	}
}

const Tagaro::RenderBackend* Tagaro::Theme::backend(const QByteArray& identifier_) const
{
	const QByteArray identifier = (identifier_ == "default") ? QByteArray() : identifier_;
	return d->m_backends.value(identifier);
}

void Tagaro::Theme::addRoute(const QRegExp& spriteKey, const QString& elementKey, const Tagaro::RenderBackend* backend)
{
	Q_ASSERT(d->m_backends.contains(const_cast<Tagaro::RenderBackend*>(backend)));
	ThemeRoute route = { spriteKey, elementKey, backend };
	d->m_routes << route;
}

static void resolveCaptures(QString& pattern, const QStringList& captures)
{
	static const QRegExp exp(QLatin1String("%(\\d+)"));
	int pos;
	while ((pos = exp.indexIn(pattern)) >= 0)
	{
		const int num = exp.capturedTexts()[1].toInt();
		pattern = pattern.mid(0, pos) + captures.value(num) + pattern.mid(pos + exp.matchedLength());
	}
}

QPair<const Tagaro::RenderBackend*, QString> Tagaro::Theme::mapSpriteKey(const QString& spriteKey) const
{
	//check routing table
	const int routeCount = d->m_routes.count();
	for (int i = 0; i < routeCount; ++i)
	{
		const ThemeRoute& route = d->m_routes[i];
		if (route.spriteKey.exactMatch(spriteKey))
		{
			//route matches -> determine mapped element key
			QString elementKey(route.elementKey);
			resolveCaptures(elementKey, route.spriteKey.capturedTexts());
			return qMakePair(route.backend, elementKey);
		}
	}
	//implicit default route: use default backend and unchanged sprite key
	return qMakePair(backend(QByteArray()), spriteKey);
}

//END Tagaro::Theme
//BEGIN Tagaro::StandardTheme

struct Tagaro::StandardTheme::Private
{
	QList<QDir> m_directories;

	void init(Tagaro::StandardTheme* theme, const QString& filePath);
};

Tagaro::StandardTheme::StandardTheme(const QString& filePath, const Tagaro::ThemeProvider* provider)
	: Tagaro::Theme(QFileInfo(filePath).absolutePath().toUtf8(), provider)
	, d(new Private)
{
	d->m_directories << QFileInfo(filePath).dir();
	d->init(this, filePath);
}

static const QByteArray dftIdentifier(const QString& ksdDirectory, const QString& fileName)
{
	if (ksdDirectory.endsWith(QChar('/')))
		return (ksdDirectory + fileName).toUtf8();
	else
		return (ksdDirectory + QChar('/') + fileName).toUtf8();
}

Tagaro::StandardTheme::StandardTheme(const QByteArray& ksdResource, const QString& ksdDirectory, const QString& fileName, const Tagaro::ThemeProvider* provider)
	: Tagaro::Theme(dftIdentifier(ksdDirectory, fileName), provider)
	, d(new Private)
{
	foreach (const QString& dirPath, KGlobal::dirs()->findDirs(ksdResource, ksdDirectory))
	{
		d->m_directories << QDir(dirPath);
	}
	d->init(this, KStandardDirs::locate(ksdResource, ksdDirectory + QChar('/') + fileName));
}

void Tagaro::StandardTheme::Private::init(Tagaro::StandardTheme* theme, const QString& filePath)
{
	//open configuration
	KConfig themeConfigFile(filePath, KConfig::SimpleConfig);
	KConfigGroup themeConfig(&themeConfigFile, "KGameTheme");
	//read standard properties
	const QString graphicsFile = themeConfig.readEntry("FileName", QString());
	if (!graphicsFile.isEmpty())
	{
		theme->addBackend(QByteArray(), graphicsFile, m_directories);
	}
	theme->setName(themeConfig.readEntry("Name", QString()));
	theme->setDescription(themeConfig.readEntry("Description", QString()));
	theme->setAuthor(themeConfig.readEntry("Author", QString()));
	theme->setAuthorEmail(themeConfig.readEntry("AuthorEmail", QString()));
	const QString previewFile = themeConfig.readEntry("Preview", QString());
	if (!previewFile.isEmpty())
	{
		theme->setPreview(QPixmap(resolveRelativePath(previewFile, m_directories)));
	}
	//find custom keys
	QMap<QString, QString> entries = themeConfig.entryMap();
	entries.remove(QLatin1String("FileName"));
	entries.remove(QLatin1String("Name"));
	entries.remove(QLatin1String("Description"));
	entries.remove(QLatin1String("Author"));
	entries.remove(QLatin1String("AuthorEmail"));
	entries.remove(QLatin1String("Preview"));
	theme->setCustomData(entries);
}

//END Tagaro::StandardTheme
