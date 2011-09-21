/***************************************************************************
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

#include "sound.h"

#include <KDE/KDebug>

struct KGame::Sound::Private
{
	//TODO
};

KGame::Sound::Sound(const QString& file, QObject* parent)
	: QObject(parent)
	, d(new Private)
{
	//TODO
}

KGame::Sound::~Sound()
{
	delete d;
}

bool KGame::Sound::isValid() const
{
	//TODO
}

KGame::Sound::PlaybackType KGame::Sound::playbackType() const
{
	return KGame::Sound::AmbientPlayback;
}

void KGame::Sound::setPlaybackType(KGame::Sound::PlaybackType type)
{
	Q_UNUSED(type)
	static bool onlyOnce = true;
	if (onlyOnce)
	{
		onlyOnce = false;
		kDebug() << "Not supported by Phonon.";
	}
}

QPointF KGame::Sound::pos() const
{
	return QPointF(0.0, 0.0);
}

void KGame::Sound::setPos(const QPointF& pos)
{
	Q_UNUSED(pos)
	static bool onlyOnce = true;
	if (onlyOnce)
	{
		onlyOnce = false;
		kDebug() << "Not supported by Phonon.";
	}
}

qreal KGame::Sound::volume() const
{
	//TODO
}

void KGame::Sound::setVolume(qreal volume)
{
	//TODO
}

void KGame::Sound::start()
{
	//TODO
}

void KGame::Sound::start(const QPointF& pos)
{
	Q_UNUSED(pos)
	static bool onlyOnce = true;
	if (onlyOnce)
	{
		onlyOnce = false;
		kDebug() << "Positional playback not supported by Phonon.";
	}
	//ignore parameter
	start();
}

void KGame::Sound::stop()
{
	//TODO
}

#include "sound.moc"
