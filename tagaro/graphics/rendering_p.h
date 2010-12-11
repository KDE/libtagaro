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

#ifndef TAGARO_RENDERING_P_H
#define TAGARO_RENDERING_P_H

#include <QtCore/QMetaType>
#include <QtCore/QRunnable>
#include <QtGui/QImage>

namespace Tagaro {

class RendererPrivate;
class RendererModule;

struct RenderJob
{
	Tagaro::RendererModule* module;
	QSize size;
	QString elementKey;
};

class RenderWorker : public QRunnable
{
	public:
		//API for synchronous rendering
		static QImage run(Tagaro::RenderJob* job);
		//API for asynchronous rendering (i.e. via QThreadPool)
		RenderWorker(Tagaro::RenderJob* job, Tagaro::RendererPrivate* d);
		virtual void run();
	private:
		Tagaro::RendererPrivate* const d;
		Tagaro::RenderJob* m_job;
};

} //namespace Tagaro

Q_DECLARE_METATYPE(Tagaro::RenderJob*)

#endif // TAGARO_RENDERING_P_H
