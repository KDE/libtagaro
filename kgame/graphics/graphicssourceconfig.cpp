/***************************************************************************
 *   Copyright 2010-2011 Stefan Majewsky <majewsky@gmx.net>                *
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

//BEGIN KGame::GraphicsSourceConfig

#include "graphicssourceconfig.h"

struct KGame::GraphicsSourceConfig::Private
{
	int m_cacheSize, m_frameBaseIndex;
	QString m_frameSuffix;

	Private();
};

KGame::GraphicsSourceConfig::Private::Private()
	: m_cacheSize(3) //in megabytes
	, m_frameBaseIndex(0)
	, m_frameSuffix(QLatin1String("_%1"))
{
}

KGame::GraphicsSourceConfig::GraphicsSourceConfig()
	: d(new Private)
{
}

KGame::GraphicsSourceConfig::GraphicsSourceConfig(const KGame::GraphicsSourceConfig& other)
	: d(new Private(*other.d))
{
}

KGame::GraphicsSourceConfig& KGame::GraphicsSourceConfig::operator=(const KGame::GraphicsSourceConfig& other)
{
	*d = *other.d;
	return *this;
}

KGame::GraphicsSourceConfig::~GraphicsSourceConfig()
{
	delete d;
}

int KGame::GraphicsSourceConfig::cacheSize() const
{
	return d->m_cacheSize;
}

void KGame::GraphicsSourceConfig::setCacheSize(int cacheSize)
{
	d->m_cacheSize = cacheSize;
}

int KGame::GraphicsSourceConfig::frameBaseIndex() const
{
	return d->m_frameBaseIndex;
}

void KGame::GraphicsSourceConfig::setFrameBaseIndex(int frameBaseIndex)
{
	d->m_frameBaseIndex = frameBaseIndex;
}

QString KGame::GraphicsSourceConfig::frameSuffix() const
{
	return d->m_frameSuffix;
}

void KGame::GraphicsSourceConfig::setFrameSuffix(const QString& suffix)
{
	d->m_frameSuffix = suffix.contains(QLatin1String("%1")) ? suffix : QLatin1String("_%1");
}

//END KGame::GraphicsSourceConfig
