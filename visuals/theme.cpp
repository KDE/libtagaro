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

#include "theme.h"

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>

struct Tagaro::Theme::Private
{
	QByteArray m_identifier;
	QHash<Tagaro::Theme::Role, QVariant> m_values;
	QHash<QByteArray, QVariant> m_customValues;

	Private(const QByteArray& identifier) : m_identifier(identifier) {}
};

Tagaro::Theme::Theme(const QByteArray& identifier)
	: d(new Private(identifier))
{
}

Tagaro::Theme::~Theme()
{
	delete d;
}

QVariant Tagaro::Theme::data(Tagaro::Theme::Role role, const QVariant& defaultValue) const
{
	return d->m_values.value(role, defaultValue);
}

QVariant Tagaro::Theme::data(const QByteArray& key, const QVariant& defaultValue) const
{
	return d->m_customValues.value(key, defaultValue);
}

void Tagaro::Theme::setData(Tagaro::Theme::Role role, const QVariant& value)
{
	d->m_values.insert(role, value);
}

void Tagaro::Theme::setData(const QByteArray& key, const QVariant& value)
{
	d->m_customValues.insert(key, value);
}

QByteArray Tagaro::Theme::identifier() const
{
	return d->m_identifier;
}

uint Tagaro::Theme::modificationTimestamp() const
{
	const QFileInfo svgFile(d->m_values.value(Tagaro::Theme::GraphicsFileRole).toString());
	const QFileInfo themeFile(d->m_values.value(Tagaro::Theme::ThemeFileRole).toString());
	uint timestamp = svgFile.lastModified().toTime_t();
	if (themeFile.exists())
		timestamp = qMax(timestamp, themeFile.lastModified().toTime_t());
	return timestamp;
}
