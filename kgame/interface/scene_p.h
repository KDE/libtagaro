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

#ifndef KGAME_SCENE_P_H
#define KGAME_SCENE_P_H

#include "scene.h"
#include "../graphics/spriteclient.h"

struct KGame::Scene::Private : public KGame::SpriteClient
{
	public:
		Private(KGame::Sprite* backgroundSprite, KGame::Scene* parent);

		//Returns whether sceneRect() was reset to mainView->rect().
		bool _k_resetSceneRect();
		void _k_updateSceneRect(const QRectF& rect);
		inline void updateRenderSize(const QSize& sceneSize);

		//interface to KGame::MessageOverlay
		void addMessageOverlay(KGame::MessageOverlay* overlay);
		void _k_moDestroyed(QObject* object);
		void _k_moTextChanged(const QString& text);
		void _k_moVisibleChanged(bool isVisible);

		KGame::Scene* m_parent;
		QGraphicsView* m_mainView;
		QSize m_renderSize;
		bool m_adjustingSceneRect;

		QList<KGame::MessageOverlay*> m_overlays;
		KGame::MessageOverlay* m_currentOverlay;
	protected:
		virtual void receivePixmap(const QPixmap& pixmap);
};

#endif // KGAME_SCENE_P_H
