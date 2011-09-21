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

#include "sound.h"
#include "openalruntime_p.h"

#include <sndfile.hh> //TODO: use Phonon instead of libsndfile for decoding
#include <KDE/KDebug>

struct KGame::Sound::Private
{
	KGame::Sound::PlaybackType m_type;
	qreal m_volume;
	QPointF m_pos;

	bool m_valid;
	ALuint m_buffer;

	Private() : m_type(KGame::Sound::AmbientPlayback), m_volume(1.0), m_valid(false), m_buffer(AL_NONE) {}
};

//BEGIN KGame::Sound

KGame::Sound::Sound(const QString& file, QObject* parent)
	: QObject(parent)
	, d(new Private)
{
	//open sound file
	SndfileHandle handle(file.toUtf8());
	if (handle.error())
	{
		kDebug() << "Failed to load sound file. Error message from libsndfile follows.";
		kDebug() << handle.strError();
		return;
	}
	const int channelCount = handle.channels();
	const int sampleCount = channelCount * handle.frames();
	const int sampleRate = handle.samplerate();
	//load data from sound file
	QVector<ALshort> samples(sampleCount);
	if (handle.read(samples.data(), sampleCount) < sampleCount)
	{
		kDebug() << "Failed to read sound file" << file;
		kDebug() << "File ended unexpectedly.";
		return;
	}
	//determine file format from number of channels
	ALenum format;
	switch (channelCount)
	{
		case 1:
			format = AL_FORMAT_MONO16;
			break;
		case 2:
			format = AL_FORMAT_STEREO16;
			break;
		default:
			kDebug() << "Failed to read sound file" << file;
			kDebug() << "More than two channels are not supported.";
			return;
	}
	//make sure OpenAL is initialized; clear OpenAL error storage
	KGame::OpenALRuntime::instance();
	int error; alGetError();
	//create OpenAL buffer
	alGenBuffers(1, &d->m_buffer);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		kDebug() << "Failed to create OpenAL buffer: Error code" << error;
		return;
	}
	alBufferData(d->m_buffer, format, samples.data(), sampleCount * sizeof(ALshort), sampleRate);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		kDebug() << "Failed to fill OpenAL buffer: Error code" << error;
		alDeleteBuffers(1, &d->m_buffer);
		return;
	}
	//loading finished
	d->m_valid = true;
}

KGame::Sound::~Sound()
{
	if (d->m_valid)
	{
		stop();
		KGame::OpenALRuntime::instance()->m_soundsEvents.remove(this);
		alDeleteBuffers(1, &d->m_buffer);
	}
	delete d;
}

bool KGame::Sound::isValid() const
{
	return d->m_valid;
}

KGame::Sound::PlaybackType KGame::Sound::playbackType() const
{
	return d->m_type;
}

void KGame::Sound::setPlaybackType(KGame::Sound::PlaybackType type)
{
	d->m_type = type;
}

QPointF KGame::Sound::pos() const
{
	return d->m_pos;
}

void KGame::Sound::setPos(const QPointF& pos)
{
	d->m_pos = pos;
}

qreal KGame::Sound::volume() const
{
	return d->m_volume;
}

void KGame::Sound::setVolume(qreal volume)
{
	d->m_volume = volume;
}

void KGame::Sound::start()
{
	if (d->m_valid)
	{
		new KGame::PlaybackEvent(this, d->m_pos);
	}
}

void KGame::Sound::start(const QPointF& pos)
{
	if (d->m_valid)
	{
		new KGame::PlaybackEvent(this, pos);
	}
}

void KGame::Sound::stop()
{
	qDeleteAll(KGame::OpenALRuntime::instance()->m_soundsEvents.take(this));
}

//END KGame::Sound
//BEGIN KGame::PlaybackEvent

KGame::PlaybackEvent::PlaybackEvent(KGame::Sound* sound, const QPointF& pos)
	: m_valid(false)
{
	//make sure OpenAL is initialized
	KGame::OpenALRuntime* runtime = KGame::OpenALRuntime::instance();
	//clear OpenAL error storage
	int error; alGetError();
	//create source for playback
	alGenSources(1, &m_source);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		kDebug() << "Failed to create OpenAL source: Error code" << error;
		return;
	}
	//store in OpenALRuntime
	runtime->m_soundsEvents[sound] << this;
	m_valid = true;
	//connect to sound (buffer)
	alSource3f(m_source, AL_POSITION, pos.x(), pos.y(), 0);
	alSourcef(m_source, AL_PITCH, 1.5); //TODO: debug
	alSourcef(m_source, AL_GAIN, sound->volume());
	alSourcei(m_source, AL_BUFFER, sound->d->m_buffer);
	const KGame::Sound::PlaybackType type = sound->playbackType();
	alSourcef(m_source, AL_ROLLOFF_FACTOR, type == KGame::Sound::AmbientPlayback ? 0.0 : 1.0);
	alSourcei(m_source, AL_SOURCE_RELATIVE, type == KGame::Sound::RelativePlayback ? AL_TRUE : AL_FALSE);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		kDebug() << "Failed to setup OpenAL source: Error code" << error;
		return;
	}
	//start playback
	alSourcePlay(m_source);
}

KGame::PlaybackEvent::~PlaybackEvent()
{
	if (m_valid)
	{
		alSourceStop(m_source);
		alDeleteSources(1, &m_source);
	}
}

bool KGame::PlaybackEvent::isRunning() const
{
	ALint state;
	alGetSourcei(m_source, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

//END KGame::PlaybackEvent

#include "sound.moc"
