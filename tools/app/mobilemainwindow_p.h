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

#ifndef TAPP_MOBILEMAINWINDOW_P_H
#define TAPP_MOBILEMAINWINDOW_P_H

#include "instantiable.h"

#include <QtGui/QBoxLayout>
#include <QtGui/QToolButton>
#include <KDE/KAction>
#include <KDE/KStandardAction>
#include <Tagaro/Game>

Q_DECLARE_METATYPE(Tagaro::Game*)

namespace TApp
{
	class Button : public QToolButton
	{
		Q_OBJECT
		public:
			Button(KAction* act, QWidget* parent = 0)
				: QToolButton(parent)
			{
				setDefaultAction(act);
				setIconSize(QSize(48, 48));
			}
		public Q_SLOTS:
			void setDown(bool down) { QToolButton::setDown(down); }
	};

	class ButtonList : public QWidget
	{
		Q_OBJECT
		public:
			ButtonList(QWidget* parent = 0);
		public Q_SLOTS:
			void createButton(Tagaro::Game* game);
		Q_SIGNALS:
			void selected(Tagaro::Game* game);
			void showNewDialog();
		private Q_SLOTS:
			void gameSelected();
		private:
			QVBoxLayout* m_layout;
	};
}

TApp::ButtonList::ButtonList(QWidget* parent)
	: QWidget(parent)
	, m_layout(new QVBoxLayout(this))
{
	qRegisterMetaType<Tagaro::Game*>();
	m_layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	//"New" button
	KAction* act = KStandardAction::openNew(this, SIGNAL(showNewDialog()), this);
	TApp::Button* btn = new TApp::Button(act, this);
	m_layout->addWidget(btn);
}

void TApp::ButtonList::createButton(Tagaro::Game* game)
{
	TApp::Instantiable* inst = TApp::Instantiable::forWidget(game);
	KAction* act = new KAction(inst->data(Qt::DisplayRole).value<QString>(), this);
	act->setIcon(inst->data(Qt::DecorationRole).value<QIcon>());
	TApp::Button* btn = new TApp::Button(act, this);
	m_layout->addWidget(btn);
	btn->setProperty("_t_game", QVariant::fromValue(game));
	connect(game, SIGNAL(activeChanged(bool)), btn, SLOT(setDown(bool)));
	connect(btn, SIGNAL(clicked()), SLOT(gameSelected()));
	connect(game, SIGNAL(destroyed(QObject*)), btn, SLOT(deleteLater()));
	act->QObject::setParent(btn);
}

void TApp::ButtonList::gameSelected()
{
	Tagaro::Game* game = sender()->property("_t_game").value<Tagaro::Game*>();
	if (game)
		emit selected(game);
}

#endif // TAPP_MOBILEMAINWINDOW_P_H
