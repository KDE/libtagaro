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

#include "messageoverlay.h"
#include "scene.h"
#include "scene_p.h"

#include <QtCore/QBasicTimer>
#include <QtCore/QTimerEvent>

struct KGame::MessageOverlay::Private
{
	QString m_text;
	int m_timeout;
	bool m_visible;
	QBasicTimer m_timer;

	Private() : m_timeout(-1), m_visible(false) {}
};

//The interesting part of this class is that it does not do anything with the
//GUI. It is just a data class. The actual MessageOverlay GUI is constructed by
//the scene as part of its drawForeground().

KGame::MessageOverlay::MessageOverlay(KGame::Scene* scene)
	: d(new Private)
{
	//WARNING: The scene requires that this overlay is not visible during the following call.
	scene->d->addMessageOverlay(this);
}

KGame::MessageOverlay::~MessageOverlay()
{
	delete d;
}

QString KGame::MessageOverlay::text() const
{
	return d->m_text;
}

void KGame::MessageOverlay::setText(const QString& text)
{
	if (d->m_text != text)
	{
		d->m_text = text;
		emit textChanged(text);
	}
}

int KGame::MessageOverlay::timeout() const
{
	return d->m_timeout;
}

void KGame::MessageOverlay::setTimeout(int timeout)
{
	if (timeout <= 0)
	{
		timeout = -1;
	}
	if (d->m_timeout != timeout)
	{
		d->m_timeout = timeout;
		//adjust timer
		if (d->m_visible)
		{
			d->m_timer.stop();
			if (d->m_timeout > 0)
			{
				d->m_timer.start(d->m_timeout, this);
			}
		}
	}
}

bool KGame::MessageOverlay::isVisible() const
{
	return d->m_visible;
}

void KGame::MessageOverlay::setVisible(bool visible)
{
	if (d->m_visible == visible)
	{
		return;
	}
	if ((d->m_visible = visible))
	{
		if (d->m_timeout > 0)
		{
			d->m_timer.start(d->m_timeout, this);
		}
	}
	else
	{
		d->m_timer.stop();
	}
	emit visibleChanged(visible);
}

void KGame::MessageOverlay::show()
{
	setVisible(true);
}

void KGame::MessageOverlay::hide()
{
	setVisible(false);
}

void KGame::MessageOverlay::timerEvent(QTimerEvent* event)
{
	if (event->timerId() != d->m_timer.timerId())
	{
		return QObject::timerEvent(event);
	}
	d->m_timer.stop(); //singleshot behavior
	d->m_visible = false;
	emit visibleChanged(false);
}

#include "messageoverlay.moc"
