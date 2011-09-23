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

#ifndef KGAME_DIFFICULTY_H
#define KGAME_DIFFICULTY_H

#include <QtCore/QObject>

#include <libkgame_export.h>

namespace KGame {

/**
 * @class KGame::DifficultyLevel difficulty.h <KGame/DifficultyLevel>
 * @see KGame::Difficulty
 */
class KGAME_EXPORT DifficultyLevel
{
	public:
		enum StandardLevel
		{
			Custom = -1, ///< standardLevel() returns this for custom levels.
			RidiculouslyEasy = 10,
			VeryEasy = 20,
			Easy = 30,
			Medium = 40,
			Hard = 50,
			VeryHard = 60,
			ExtremelyHard = 70,
			Impossible = 80
		};

		///Refer to the getters' documentation for details on the params.
		DifficultyLevel(int hardness, const QByteArray& key, const QString& title);
		DifficultyLevel(StandardLevel level);
		virtual ~DifficultyLevel();

		///@return a numeric key which is used to sort the levels by difficulty
		///        (smaller values mean easier levels)
		///@note For standard levels, this equals the numeric value of the level
		///      in the StandardLevel enumeration.
		int hardness() const;
		///@return a @b non-localized key for this level
		QByteArray key() const;
		///@return a @b localized title for this level
		QString title() const;
		///@return the standard level which was used to create this level, or
		///        KGame::DifficultyLevel::Custom for custom levels
		StandardLevel standardLevel() const;
	private:
		class Private;
		Private* const d;
};

/**
 * @class KGame::Difficulty difficulty.h <KGame/Difficulty>
 *
 * KGame::Difficulty manages difficulty levels of a game in a standard way.
 * The difficulty can be a type of game (like in KMines: small or big field) or
 * the AI skills (like in Bovo: how deep should the computer search to find the
 * best move) or a combination of both of them. On the user point of view, it's
 * not really different: either is the game easy or hard to play.
 *
 * KGame::Difficulty contains a list of KGame::DifficultyLevel instances. One of
 * these levels is selected; this selection will be recorded when the
 * application is closed. A set of standard difficulty levels is provided by
 * KGame::DifficultyLevel, but custom levels can be defined at the same time.
 */
class KGAME_EXPORT Difficulty : public QObject
{
	Q_OBJECT
	public:
		Difficulty();
		///Destroys this instance and all DifficultyLevel instances in it.
		virtual ~Difficulty();

		///Adds a difficulty level to this instance. This will not affect the
		///currentLevel() if there is one.
		void addLevel(KGame::DifficultyLevel* level);
		///This convenience method adds a range of standard levels to this
		///instance (including the boundaries). For example:
		///@code
		///difficulty.addStandardLevelRange(
		///    KGame::DifficultyLevel::Easy,
		///    KGame::DifficultyLevel::VeryHard
		///);
		///@endcode
		///This adds the levels "Easy", "Medium", "Hard" and "Very hard".
		void addStandardLevelRange(KGame::DifficultyLevel::StandardLevel from, KGame::DifficultyLevel::StandardLevel to);

		///@return a list of all difficulty levels, sorted by hardness
		QList<const KGame::DifficultyLevel*> levels() const;
		///@return the current difficulty level
		///
		///The current difficulty level will only be determined when this method
		///is called for the first time. This allows the application developer
		///to set up the difficulty levels before KGame::Difficulty retrieves
		///the last selected level from the configuration file.
		const KGame::DifficultyLevel* currentLevel() const;

		///KGame::Difficulty has optional protection against changing the
		///difficulty level while a game is running. If setRunning(true) has
		///been called, and select() is called to select a new difficulty level,
		///the user will be asked for confirmation.
		void setGameRunning(bool running);
	Q_SIGNALS:
		///Emitted when a new difficulty level has been selected.
		void changed(const KGame::DifficultyLevel* level);
		///Emitted after every call to select(), even when the user has rejected
		///the change. (This may be used by a difficulty level selection UI.)
		void selected(const KGame::DifficultyLevel* level);
	public Q_SLOTS:
		///Select a new difficulty level. The given level must already have been
		///added to this instance.
		void select(const KGame::DifficultyLevel* level);
	private:
		class Private;
		Private* const d;
};

} // namespace KGame

#endif // KGAME_DIFFICULTY_H
