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

#include "touchwindow.h"
#include "pullhandlewidget_p.h"
#include "slidelayout_p.h"

#include <QtGui/QStackedLayout>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>

namespace KGame {

struct TouchWindow::Private
{
	TouchWindow* q;
	KGame::SlideLayout* m_layout;

	QStackedLayout* m_firstDialogLayout;
	QVBoxLayout* m_firstPanelLayout;
	QStackedLayout* m_centralLayout;
	QToolBar* m_firstToolBar;

	QMap<KGame::PullHandleWidget*, QWidget*> m_firstPanelHandleMap;

	Private(TouchWindow* q);
	void _t_pullHandlePulled();
};

TouchWindow::TouchWindow(QWidget* parent, Qt::WindowFlags flags)
	: QWidget(parent, flags)
	, d(new Private(this))
{
}

TouchWindow::Private::Private(TouchWindow* q)
	: q(q)
	, m_layout(new SlideLayout(q))
	, m_firstDialogLayout(new QStackedLayout)
	, m_firstPanelLayout(new QVBoxLayout)
	, m_centralLayout(new QStackedLayout)
	, m_firstToolBar(new QToolBar)
{
	//populate main layout
	m_layout->addLayout(m_firstDialogLayout);
	m_layout->addFixation();
	m_layout->addLayout(m_firstPanelLayout);
	m_layout->addLayout(m_centralLayout);
	m_layout->addFixation();
	m_layout->setCurrentScrollTarget(1);
	//setup panel
	m_firstToolBar->setOrientation(Qt::Vertical);
	m_firstToolBar->setIconSize(QSize(32, 32));
	m_firstPanelLayout->addWidget(m_firstToolBar);
}

TouchWindow::~TouchWindow()
{
	delete d; //All contained widgets and layouts are deleted by the usual
	          //QWidget/QLayout cleanup.
}

void TouchWindow::addAction(QAction* action)
{
	d->m_firstToolBar->addAction(action);
}

void TouchWindow::addPullHandle(const QString& title, QWidget* widget)
{
	PullHandleWidget* w = new PullHandleWidget(
		title, PullHandleWidget::LeftEdge
	);
	connect(w, SIGNAL(pulled()), SLOT(_t_pullHandlePulled()));
	d->m_firstPanelLayout->addWidget(w, /*stretch=*/ 1);
	d->m_firstDialogLayout->addWidget(widget);
	d->m_firstPanelHandleMap.insert(w, widget);
}

void TouchWindow::Private::_t_pullHandlePulled()
{
	PullHandleWidget* sender = static_cast<PullHandleWidget*>(q->sender());
	QWidget* widget = m_firstPanelHandleMap.value(sender);
	if (widget)
	{
		m_firstDialogLayout->setCurrentWidget(widget);
		emit q->handlePulled(widget);
	}
}

QWidget* TouchWindow::centralWidget() const
{
	return d->m_centralLayout->widget(0);
}

void TouchWindow::setCentralWidget(QWidget* widget)
{
	delete d->m_centralLayout->widget(0);
	d->m_centralLayout->addWidget(widget);
}

} //namespace KGame
#include "touchwindow.moc"
