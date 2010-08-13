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

#include "kgvtheme.h"

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>

struct KgvTheme::Private
{
	QByteArray m_identifier;
	QHash<KgvTheme::Role, QVariant> m_values;
	QHash<QByteArray, QVariant> m_customValues;

	Private(const QByteArray& identifier) : m_identifier(identifier) {}
};

KgvTheme::KgvTheme(const QByteArray& identifier)
	: d(new Private(identifier))
{
}

KgvTheme::~KgvTheme()
{
	delete d;
}

QVariant KgvTheme::data(KgvTheme::Role role, const QVariant& defaultValue) const
{
	return d->m_values.value(role, defaultValue);
}

QVariant KgvTheme::data(const QByteArray& key, const QVariant& defaultValue) const
{
	return d->m_customValues.value(key, defaultValue);
}

void KgvTheme::setData(KgvTheme::Role role, const QVariant& value)
{
	d->m_values.insert(role, value);
}

void KgvTheme::setData(const QByteArray& key, const QVariant& value)
{
	d->m_customValues.insert(key, value);
}

QByteArray KgvTheme::identifier() const
{
	return d->m_identifier;
}

uint KgvTheme::modificationTimestamp() const
{
	const QFileInfo svgFile(d->m_values.value(KgvTheme::GraphicsFileRole).toString());
	const QFileInfo themeFile(d->m_values.value(KgvTheme::ThemeFileRole).toString());
	uint timestamp = svgFile.lastModified().toTime_t();
	if (themeFile.exists())
		timestamp = qMax(timestamp, themeFile.lastModified().toTime_t());
	return timestamp;
}
