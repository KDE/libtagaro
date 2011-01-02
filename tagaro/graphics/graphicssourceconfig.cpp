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

//BEGIN Tagaro::GraphicsSourceConfig

#include "graphicssourceconfig.h"

struct Tagaro::GraphicsSourceConfig::Private
{
	int m_cacheSize, m_frameBaseIndex;
	QString m_frameSuffix;

	Private();
};

Tagaro::GraphicsSourceConfig::Private::Private()
	: m_cacheSize(3) //in megabytes
	, m_frameBaseIndex(0)
	, m_frameSuffix(QLatin1String("_%1"))
{
}

Tagaro::GraphicsSourceConfig::GraphicsSourceConfig()
	: d(new Private)
{
}

Tagaro::GraphicsSourceConfig::GraphicsSourceConfig(const Tagaro::GraphicsSourceConfig& other)
	: d(new Private(*other.d))
{
}

Tagaro::GraphicsSourceConfig& Tagaro::GraphicsSourceConfig::operator=(const Tagaro::GraphicsSourceConfig& other)
{
	*d = *other.d;
	return *this;
}

Tagaro::GraphicsSourceConfig::~GraphicsSourceConfig()
{
	delete d;
}

int Tagaro::GraphicsSourceConfig::cacheSize() const
{
	return d->m_cacheSize;
}

void Tagaro::GraphicsSourceConfig::setCacheSize(int cacheSize)
{
	d->m_cacheSize = cacheSize;
}

int Tagaro::GraphicsSourceConfig::frameBaseIndex() const
{
	return d->m_frameBaseIndex;
}

void Tagaro::GraphicsSourceConfig::setFrameBaseIndex(int frameBaseIndex)
{
	d->m_frameBaseIndex = frameBaseIndex;
}

QString Tagaro::GraphicsSourceConfig::frameSuffix() const
{
	return d->m_frameSuffix;
}

void Tagaro::GraphicsSourceConfig::setFrameSuffix(const QString& suffix)
{
	d->m_frameSuffix = suffix.contains(QLatin1String("%1")) ? suffix : QLatin1String("_%1");
}

//END Tagaro::GraphicsSourceConfig
