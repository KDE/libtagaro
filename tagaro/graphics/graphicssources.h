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

#ifndef TAGARO_GRAPHICSSOURCES_H
#define TAGARO_GRAPHICSSOURCES_H

#include "graphicssource.h"

namespace Tagaro {

class QtSvgGraphicsSource : public Tagaro::GraphicsSource
{
	public:
		QtSvgGraphicsSource(const QString& path, const Tagaro::GraphicsSourceConfig& config);
		virtual ~QtSvgGraphicsSource();

		virtual uint lastModified() const;
		virtual QRectF elementBounds(const QString& element) const;
		virtual bool elementExists(const QString& element) const;
		virtual QImage elementImage(const QString& element, const QSize& size, bool timeConstraint) const;
	protected:
		virtual bool load();
	private:
		class Private;
		Private* const d;
};

class ColorGraphicsSource : public Tagaro::GraphicsSource
{
	public:
		ColorGraphicsSource(const Tagaro::GraphicsSourceConfig& config);
		virtual ~ColorGraphicsSource();

		virtual bool elementExists(const QString& element) const;
		virtual QImage elementImage(const QString& element, const QSize& size, bool timeConstraint) const;
	private:
		class Private;
		Private* const d;
};

class ImageGraphicsSource : public Tagaro::GraphicsSource
{
	public:
		ImageGraphicsSource(const QString& path, const Tagaro::GraphicsSourceConfig& config);
		virtual ~ImageGraphicsSource();

		virtual void addConfiguration(const QMap<QString, QString>& configuration);
		virtual bool load();

		virtual QRectF elementBounds(const QString& element) const;
		virtual bool elementExists(const QString& element) const;
		virtual QImage elementImage(const QString& element, const QSize& size, bool timeConstraint) const;
	private:
		class Private;
		Private* const d;
};

} //namespace Tagaro

#endif // TAGARO_GRAPHICSSOURCES_H
