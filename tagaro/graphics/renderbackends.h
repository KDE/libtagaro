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

#ifndef TAGARO_RENDERBACKENDS_H
#define TAGARO_RENDERBACKENDS_H

#include "renderbackend.h"

namespace Tagaro {

class QtSvgRenderBackend : public Tagaro::RenderBackend
{
	public:
		QtSvgRenderBackend(const QString& path, const Tagaro::RenderBehavior& behavior);
		virtual ~QtSvgRenderBackend();

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

} //namespace Tagaro

#endif // TAGARO_RENDERBACKENDS_H
