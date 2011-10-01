/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#ifndef KGAME_DIFFICULTYSLIDER_H
#define KGAME_DIFFICULTYSLIDER_H

#include <QtGui/QWidget>

#include <libkgame_export.h>

namespace KGame {

class Difficulty;
class DifficultyLevel;

/**
 * @class KGame::DifficultySlider difficultyslider.h <KGame/DifficultySlider>
 *
 * This slider widget allows the user to select a difficulty level. By default,
 * the selection is not applied to the KGame::Difficulty instance directly to
 * support the usage of the slider in dialogs. Connect selected() to select() of
 * KGame::Difficulty and this slider in both directions to get this behavior.
 */
class KGAME_EXPORT DifficultySlider : public QWidget
{
	Q_OBJECT
	public:
		///Creates a new difficulty slider using the difficulty levels in the
		///given KGame::Difficulty instance.
		DifficultySlider(KGame::Difficulty* difficulty, QWidget* parent = 0);

		///@return which level is currently selected on the slider
		const KGame::DifficultyLevel* selectedLevel() const;

		virtual QSize minimumSizeHint() const;
		virtual QSize sizeHint() const;
	public Q_SLOTS:
		///Selects a different difficulty level on this slider.
		void select(const KGame::DifficultyLevel* level);
	Q_SIGNALS:
		///Emitted when a difficulty level has been selected on this slider.
		void selected(const KGame::DifficultyLevel* level);
	protected:
		virtual void resizeEvent(QResizeEvent* event);
	private:
		class Private;
		Private* const d;
		Q_PRIVATE_SLOT(d, void _t_sliderMoved(int));
		Q_PRIVATE_SLOT(d, void _t_valueChanged(int));
};

} //namespace KGame

#endif // KGAME_DIFFICULTYSLIDER_H
