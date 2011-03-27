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

#include "game.h"

#include <KDE/KAboutData>
#include <KDE/KComponentData>
#include <KDE/KGlobal>
#include <KDE/KIcon>
#include <KDE/KLocale>
#include <KDE/KMainWindow>

struct Tagaro::Game::Private
{
	KComponentData m_cdata;
	bool m_active, m_paused;
	QString m_caption, m_windowTitle;
	//what gets passed through shell handshake
	KMainWindow* m_mainWindow;
	KIcon m_windowIcon;

	Private(const KAboutData& aboutData, const QVariantList& args);
};

Tagaro::Game::Private::Private(const KAboutData& aboutData, const QVariantList& args)
	: m_cdata(aboutData)
	, m_active(false)
	, m_paused(true)
{
	//check size and protocol version of initialization sequence
	Q_ASSERT_X(args.count() == 3, "Tagaro::Game", "Handshake with shell failed.");
	Q_ASSERT_X(args[0] == QVariant(1), "Tagaro::Game", "Handshake with shell failed.");
	//read handshake message
	m_mainWindow = qobject_cast<KMainWindow*>(args[1].value<QObject*>());
	Q_ASSERT(m_mainWindow);
	m_windowIcon = KIcon(args[2].toString());
}

Tagaro::Game::Game(const KAboutData& aboutData, QObject* parent, const QVariantList& args)
	: d(new Private(aboutData, args))
{
	QObject::setParent(parent);
}

Tagaro::Game::~Game()
{
	delete d;
}

bool Tagaro::Game::isActive() const
{
	return d->m_active;
}

bool Tagaro::Game::isPaused() const
{
	return d->m_paused;
}

QString Tagaro::Game::caption() const
{
	return d->m_windowTitle;
}

const KComponentData& Tagaro::Game::componentData() const
{
	return d->m_cdata;
}

void Tagaro::Game::setActive(bool active)
{
	if (d->m_active == active)
	{
		return;
	}
	if (!active)
	{
		setPaused(true);
	}
	d->m_active = active;
	if (active)
	{
		KGlobal::setActiveComponent(d->m_cdata);
		d->m_mainWindow->setWindowIcon(d->m_windowIcon);
		d->m_mainWindow->setWindowTitle(d->m_windowTitle);
	}
	activeEvent(active);
	emit activeChanged(active);
	if (!active)
	{
		setPaused(false);
	}
}

void Tagaro::Game::setPaused(bool paused)
{
	if (!d->m_active)
	{
		paused = true;
	}
	if (d->m_paused == paused)
	{
		return;
	}
	d->m_paused = paused;
	pauseEvent(paused);
	emit pausedChanged(paused);
}

void Tagaro::Game::setCaption(const QString& caption)
{
	if (d->m_caption == caption)
	{
		return;
	}
	d->m_caption = caption;
	const QString appName = d->m_cdata.aboutData()->programName();
	d->m_windowTitle = caption + i18nc("Document/application separator in titlebar", " – ") + appName;
	if (d->m_active)
	{
		d->m_mainWindow->setWindowTitle(d->m_windowTitle);
	}
}

void Tagaro::Game::activeEvent(bool active)
{
	Q_UNUSED(active) //virtual hook
}

void Tagaro::Game::pauseEvent(bool paused)
{
	Q_UNUSED(paused) //virtual hook
}

#include "game.moc"
