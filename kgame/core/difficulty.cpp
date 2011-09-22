/***************************************************************************
 *   Copyright 2007 Nicolas Roffet <nicolas-kde@roffet.com>                *
 *   Copyright 2007 Pino Toscano <toscano.pino@tiscali.it>                 *
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>                     *
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

#include "difficulty.h"

#include <QtCore/QVector>
#include <KDE/KConfigGroup>
#include <KDE/KGlobal>
#include <KDE/KGuiItem>
#include <KDE/KLocale>
#include <KDE/KMessageBox>

namespace KGame {

//BEGIN DifficultyLevel

struct DifficultyLevel::Private
{
	int m_hardness;
	StandardLevel m_level;
	QByteArray m_key;
	QString m_title;

	Private(int hardness, const QByteArray& key, const QString& title, StandardLevel level);
	static Private* fromStandardLevel(StandardLevel level);
};

DifficultyLevel::DifficultyLevel(int hardness, const QByteArray& key, const QString& title)
	: d(new Private(hardness, key, title, Custom))
{
}

DifficultyLevel::Private::Private(int hardness, const QByteArray& key, const QString& title, StandardLevel level)
	: m_hardness(hardness)
	, m_level(level)
	, m_key(key)
	, m_title(title)
{
}

DifficultyLevel::DifficultyLevel(StandardLevel level)
	: d(Private::fromStandardLevel(level))
{
}

DifficultyLevel::Private* DifficultyLevel::Private::fromStandardLevel(DifficultyLevel::StandardLevel level)
{
	Q_ASSERT_X(level != Custom,
		"KGame::DifficultyLevel(StandardLevel) constructor",
		"Custom level not allowed here"
	);
	//The first entry in the pair is to be used as a key so don't change it. It doesn't have to match the string to be translated
	QPair<QByteArray, QString> data;
	switch (level)
	{
		case RidiculouslyEasy:
			data = qMakePair(QByteArray("Ridiculously Easy"), i18nc("Game difficulty level 1 out of 8", "Ridiculously Easy"));
			break;
		case VeryEasy:
			data = qMakePair(QByteArray("Very Easy"), i18nc("Game difficulty level 2 out of 8", "Very Easy"));
			break;
		case Easy:
			data = qMakePair(QByteArray("Easy"), i18nc("Game difficulty level 3 out of 8", "Easy"));
			break;
		case Medium:
			data = qMakePair(QByteArray("Medium"), i18nc("Game difficulty level 4 out of 8", "Medium"));
			break;
		case Hard:
			data = qMakePair(QByteArray("Hard"), i18nc("Game difficulty level 5 out of 8", "Hard"));
			break;
		case VeryHard:
			data = qMakePair(QByteArray("Very Hard"), i18nc("Game difficulty level 6 out of 8", "Very Hard"));
			break;
		case ExtremelyHard:
			data = qMakePair(QByteArray("Extremely Hard"), i18nc("Game difficulty level 7 out of 8", "Extremely Hard"));
			break;
		case Impossible:
			data = qMakePair(QByteArray("Impossible"), i18nc("Game difficulty level 8 out of 8", "Impossible"));
			break;
		case Custom:
			return 0;
	}
	return new DifficultyLevel::Private(level, data.first, data.second, level);
}

DifficultyLevel::~DifficultyLevel()
{
	delete d;
}

int DifficultyLevel::hardness() const
{
	return d->m_hardness;
}

QByteArray DifficultyLevel::key() const
{
	return d->m_key;
}

QString DifficultyLevel::title() const
{
	return d->m_title;
}

DifficultyLevel::StandardLevel DifficultyLevel::standardLevel() const
{
	return d->m_level;
}

//END DifficultyLevel
//BEGIN Difficulty

struct Difficulty::Private
{
	QList<const DifficultyLevel*> m_levels;
	const DifficultyLevel* m_currentLevel;
	bool m_gameRunning;

	Private() : m_currentLevel(0), m_gameRunning(false) {}
};

Difficulty::Difficulty()
	: d(new Private)
{
}

Difficulty::~Difficulty()
{
	//save current difficulty level in config file (no sync() call here; this
	//will most likely be called at application shutdown when others are also
	//writing to KGlobal::config(); also KConfig's dtor will sync automatically)
	KConfigGroup cg(KGlobal::config(), "KGame");
	cg.writeEntry("Difficulty", currentLevel()->key());
	//cleanup
	while (!d->m_levels.isEmpty())
	{
		delete const_cast<DifficultyLevel*>(d->m_levels.takeFirst());
	}
}

void Difficulty::addLevel(DifficultyLevel* level)
{
	//ensure that list stays sorted
	QList<const DifficultyLevel*>::iterator it = d->m_levels.begin();
	while (it != d->m_levels.end() && (*it)->hardness() < level->hardness())
	{
		++it;
	}
	d->m_levels.insert(it, level);
}

typedef DifficultyLevel::StandardLevel DS;
void Difficulty::addStandardLevelRange(DS from, DS to)
{
	const QVector<DS> levels = QVector<DS>()
		<< DifficultyLevel::RidiculouslyEasy
		<< DifficultyLevel::VeryEasy
		<< DifficultyLevel::Easy
		<< DifficultyLevel::Medium
		<< DifficultyLevel::Hard
		<< DifficultyLevel::VeryHard
		<< DifficultyLevel::ExtremelyHard
		<< DifficultyLevel::Impossible
	;
	const int fromIndex = levels.indexOf(from);
	const int toIndex = levels.indexOf(to);
	Q_ASSERT_X(fromIndex > 0 && toIndex > 0,
		"KGame::Difficulty::addStandardLevelRange",
		"No argument may be KGame::DifficultyLevel::Custom."
	);
	for (int i = fromIndex; i <= toIndex; ++i)
	{
		addLevel(new DifficultyLevel(levels[i]));
	}
}

QList<const DifficultyLevel*> Difficulty::levels() const
{
	return d->m_levels;
}

const DifficultyLevel* Difficulty::currentLevel() const
{
	if (d->m_currentLevel)
	{
		return d->m_currentLevel;
	}
	Q_ASSERT(!d->m_levels.isEmpty());
	//check configuration file for saved difficulty level
	KConfigGroup cg(KGlobal::config(), "KGame");
	const QByteArray key = cg.readEntry("Difficulty", QByteArray());
	foreach (const DifficultyLevel* level, d->m_levels)
	{
		if (level->key() == key)
		{
			return d->m_currentLevel = level;
		}
	}
	//no level predefined - easiest level is probably a sane default
	return d->m_currentLevel = d->m_levels[0];
}

void Difficulty::setGameRunning(bool running)
{
	d->m_gameRunning = running;
}

void Difficulty::select(const DifficultyLevel* level)
{
	Q_ASSERT(d->m_levels.contains(level));
	if (d->m_currentLevel == level)
	{
		return;
	}
	//ask for confirmation if necessary
	if (d->m_gameRunning)
	{
		const int result = KMessageBox::warningContinueCancel(0,
			i18n("Changing the difficulty level will end the current game!"),
			QString(), KGuiItem(i18n("Change the difficulty level"))
		);
		if (result != KMessageBox::Continue)
		{
			emit selected(d->m_currentLevel);
			return;
		}
	}
	d->m_currentLevel = level;
	emit selected(level);
	emit changed(level);
}

//END Difficulty

} // namespace KGame

#include "difficulty.moc"
