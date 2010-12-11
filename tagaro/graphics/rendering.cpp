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

#include "rendering_p.h"
#include "renderer_p.h"
#include "renderermodule.h"

#include <QtGui/QColor>
#include <QtGui/QPainter>
#include <KDE/KGlobal>

//BEGIN Tagaro::RenderWorker

static const uint transparentRgba = QColor(Qt::transparent).rgba();

QImage Tagaro::RenderWorker::run(Tagaro::RenderJob* job)
{
	QImage image(job->size, QImage::Format_ARGB32_Premultiplied);
	image.fill(transparentRgba);
	//do rendering
	QPainter painter(&image);
	job->module->render(&painter, job->elementKey);
	painter.end();
	return image;
}

Tagaro::RenderWorker::RenderWorker(Tagaro::RenderJob* job, Tagaro::RendererPrivate* d)
	: d(d)
	, m_job(job)
{
}

void Tagaro::RenderWorker::run()
{
	QMetaObject::invokeMethod(d, "jobFinished",
		Q_ARG(Tagaro::RenderJob*, m_job),
		Q_ARG(QImage, run(m_job)),
		Q_ARG(bool, false) //asynchronous
	);
}

//END Tagaro::RenderWorker

#include "rendering_p.moc"
