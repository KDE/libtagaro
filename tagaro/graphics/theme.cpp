/***************************************************************************
 *   Copyright 2010-2011 Stefan Majewsky <majewsky@gmx.net>                *
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
#include "graphicssources.h"
#include "themeprovider.h"

#include <QtCore/QFileInfo>
#include <QtGui/QImageReader>
#include <QtGui/QPixmap>
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KDE/KStandardDirs>

//TODO: share identical sources between Themes at least in the same ThemeProvider -> move source instantiation to ThemeProvider?

//BEGIN Tagaro::Theme

struct ThemeMapping
{
	QRegExp spriteKey;
	QString elementKey;
	const Tagaro::GraphicsSource* source;
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
	QHash<QByteArray, Tagaro::GraphicsSource*> m_sources;
	QList<ThemeMapping> m_mappings;

	Private(const QByteArray& identifier, const Tagaro::ThemeProvider* provider) : m_identifier(identifier), m_provider(provider) {}
	~Private() { qDeleteAll(m_sources); }
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
	QHash<QByteArray, Tagaro::GraphicsSource*>::const_iterator it1 = d->m_sources.constBegin(), it2 = d->m_sources.constEnd();
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

void Tagaro::Theme::addSource(const QByteArray& identifier_, Tagaro::GraphicsSource* source)
{
	const QByteArray identifier = (identifier_ == "default") ? QByteArray() : identifier_;
	QHash<QByteArray, Tagaro::GraphicsSource*>::const_iterator it = d->m_sources.constFind(identifier);
	if (it != d->m_sources.constEnd())
	{
		delete it.value();
	}
	d->m_sources.insert(identifier, source);
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

void Tagaro::Theme::addSource(const QByteArray& identifier, const QString& specification, const QList<QDir>& refDirs, const QMap<QString, QString>& sourceConfig)
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
		if (spec == QLatin1String("color"))
		{
			type = spec;
		}
		else
		{
			//resolve specs that represent graphics files
			QFileInfo file(spec);
			const QStringList suffixes = file.completeSuffix().split(QChar('.'));
			if (suffixes.contains(QLatin1String("svg")) || suffixes.contains(QLatin1String("svgz")))
			{
				type = "svg";
			}
			else
			{
				const QList<QByteArray> imageFormats = QImageReader::supportedImageFormats();
				foreach (const QString& suffix, suffixes)
				{
					if (imageFormats.contains(suffix.toUtf8()))
					{
						type = "image";
						break;
					}
				}
			}
		}
	}
	//resolve specifications - Add new types below here.
	Tagaro::GraphicsSource* source;
	if (type == QLatin1String("svg"))
	{
		const QString path = resolveRelativePath(spec, refDirs);
		Tagaro::QtSvgGraphicsSource* svgSource = new Tagaro::QtSvgGraphicsSource(path, d->m_provider->config());
		source = new Tagaro::CachedProxyGraphicsSource(svgSource);
	}
	else if (type == QLatin1String("image"))
	{
		const QString path = resolveRelativePath(spec, refDirs);
		source = new Tagaro::ImageGraphicsSource(path, d->m_provider->config());
	}
	else if (type == QLatin1String("color"))
	{
		source = new Tagaro::ColorGraphicsSource(d->m_provider->config());
	}
	else
	{
		kDebug() << "Failed to create GraphicsSource from specification:" << specification;
		source = 0;
	}
	//add created source to theme
	if (source)
	{
		source->addConfiguration(sourceConfig);
	}
	addSource(identifier, source);
}

const Tagaro::GraphicsSource* Tagaro::Theme::source(const QByteArray& identifier_) const
{
	const QByteArray identifier = (identifier_ == "default") ? QByteArray() : identifier_;
	return d->m_sources.value(identifier);
}

void Tagaro::Theme::addMapping(const QRegExp& spriteKey, const QString& elementKey, const Tagaro::GraphicsSource* source)
{
	Q_ASSERT(d->m_sources.contains(const_cast<Tagaro::GraphicsSource*>(source)));
	ThemeMapping mapping = { spriteKey, elementKey, source };
	d->m_mappings << mapping;
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

QPair<const Tagaro::GraphicsSource*, QString> Tagaro::Theme::mapSpriteKey(const QString& spriteKey) const
{
	//check routing table
	const int mappingCount = d->m_mappings.count();
	for (int i = 0; i < mappingCount; ++i)
	{
		const ThemeMapping& mapping = d->m_mappings[i];
		if (mapping.spriteKey.exactMatch(spriteKey))
		{
			//mapping matches -> determine mapped element key
			QString elementKey(mapping.elementKey);
			resolveCaptures(elementKey, mapping.spriteKey.capturedTexts());
			return qMakePair(mapping.source, elementKey);
		}
	}
	//implicit default mapping: use default source and unchanged sprite key
	return qMakePair(source(QByteArray()), spriteKey);
}

//END Tagaro::Theme
//BEGIN Tagaro::StandardTheme

struct Tagaro::StandardTheme::Private
{
	QList<QDir> m_directories;

	void init(Tagaro::StandardTheme* theme, const QString& filePath);
};

Tagaro::StandardTheme::StandardTheme(const QString& filePath, const Tagaro::ThemeProvider* provider)
	: Tagaro::Theme(QFileInfo(filePath).absoluteFilePath().toUtf8(), provider)
	, d(new Private)
{
	QFileInfo file(filePath);
	d->m_directories << file.dir();
	d->init(this, file.absoluteFilePath());
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
	const KConfig themeConfigFile(filePath, KConfig::SimpleConfig);
	//find group with theme properties
	const char* groupName = 0;
	if (themeConfigFile.hasGroup("Tagaro Theme"))
	{
		groupName = "Tagaro Theme";
	}
	else if (themeConfigFile.hasGroup("KGameTheme"))
	{
		groupName = "KGameTheme";
	}
	const KConfigGroup themeConfig(&themeConfigFile, groupName);
	//read standard properties
	const QString graphicsFile = themeConfig.readEntry("FileName", QString());
	if (!graphicsFile.isEmpty())
	{
		theme->addSource(QByteArray(), graphicsFile, m_directories, QMap<QString, QString>());
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
	//read sources and mappings
	const KConfigGroup sourcesConfig(&themeConfigFile, "Sources");
	const KConfigGroup mappingsConfig(&themeConfigFile, "Mappings");
	const QStringList sourceList = sourcesConfig.groupList();
	foreach (const QString& sourceName, sourceList)
	{
		const QByteArray sourceNameRaw = sourceName.toUtf8();
		//create source
		const KConfigGroup sourceConfig(&sourcesConfig, sourceName);
		const QString sourceSpec = sourceConfig.readEntry("SourceType", QString());
		QMap<QString, QString> sourceConfigMap = sourceConfig.entryMap();
		sourceConfigMap.remove(QLatin1String("SourceType"));
		theme->addSource(sourceNameRaw, sourceSpec, m_directories, sourceConfigMap);
		const Tagaro::GraphicsSource* source = theme->source(sourceNameRaw);
		//read mappings; each mapping is stored as two key-value pairs
		//   N-sprite=SPRITEKEY (as QRegExp)
		//   N-element=ELEMENTKEY (with %0, %1, %2, etc. corresponding to the captured texts)
		//where N is an integer; these integers establish a precedence (lower N
		//are evaluated first) which cannot be done by KConfig itself because
		//it reads the key-value pairs into a QMap whose internal order is
		//unrelated to the order in the file)
		const KConfigGroup mappingConfig(&mappingsConfig, sourceName);
		const QMap<QString, QString> mapData = mappingConfig.entryMap();
		QMap<QString, QString>::const_iterator it1 = mapData.constBegin(), it2 = mapData.constEnd();
		const QRegExp exp("([0-9]*)-sprite");
		QList<int> nums;
		for (; it1 != it2; ++it1)
		{
			if (exp.exactMatch(it1.key()))
			{
				nums << exp.cap(1).toInt();
			}
		}
		qSort(nums);
		foreach (int num, nums)
		{
			const QString spriteKey = mapData.value(QString::fromLatin1("%1-sprite").arg(num));
			const QString elementKey = mapData.value(QString::fromLatin1("%1-element").arg(num));
			theme->addMapping(QRegExp(spriteKey), elementKey, source);
		}
	}
}

//END Tagaro::StandardTheme
