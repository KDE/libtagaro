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

struct Tagaro::Sound::Private
{
	//TODO
};

Tagaro::Sound::Sound(const QString& file, QObject* parent)
	: QObject(parent)
	, d(new Private)
{
	//TODO
}

Tagaro::Sound::~Sound()
{
	delete d;
}

bool Tagaro::Sound::isValid() const
{
	//TODO
}

Tagaro::Sound::PlaybackType Tagaro::Sound::playbackType() const
{
	return Tagaro::Sound::AmbientPlayback;
}

void Tagaro::Sound::setPlaybackType(Tagaro::Sound::PlaybackType type)
{
	Q_UNUSED(type)
	static bool onlyOnce = true;
	if (onlyOnce)
	{
		onlyOnce = false;
		kDebug() << "Not supported by Phonon.";
	}
}

QPointF Tagaro::Sound::pos() const
{
	return QPointF(0.0, 0.0);
}

void Tagaro::Sound::setPos(const QPointF& pos)
{
	Q_UNUSED(pos)
	static bool onlyOnce = true;
	if (onlyOnce)
	{
		onlyOnce = false;
		kDebug() << "Not supported by Phonon.";
	}
}

qreal Tagaro::Sound::volume() const
{
	//TODO
}

void Tagaro::Sound::setVolume(qreal volume)
{
	//TODO
}

void Tagaro::Sound::start()
{
	//TODO
}

void Tagaro::Sound::start(const QPointF& pos)
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

void Tagaro::Sound::stop()
{
	//TODO
}

#include "sound.moc"
