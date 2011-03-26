/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#include "instantiable.h"

#include <KDE/KIcon>
#include <KDE/KMessageBox>
#include <KDE/KServiceTypeTrader>
#include <KDE/KToolInvocation>

//BEGIN TApp::Instantiable

TApp::Instantiable::Instantiable()
	: m_running(false)
	, m_widget(0)
{
}

TApp::Instantiable::~Instantiable()
{
	//NOTE: It's too late for close() because the vtable has been destroyed.
}

bool TApp::Instantiable::activate()
{
	if (!m_running)
	{
		m_running = createInstance(m_widget);
		if (!m_running)
			m_widget = 0;
	}
	if (m_running)
	{
		activateInstance(m_widget);
		return true;
	}
	else
		return false;
}

bool TApp::Instantiable::close()
{
	if (!m_running)
		return true;
	const bool success = deleteInstance(m_widget);
	m_running = !success;
	if (success)
		m_widget = 0;
	return success;
}

//END TApp::Instantiable
//BEGIN TApp::TagaroGamePlugin

/*static*/ void TApp::TagaroGamePlugin::loadInto(QStandardItemModel* model)
{
	KService::List services = KServiceTypeTrader::self()->query(QLatin1String("Tagaro/Game"));
	foreach (KService::Ptr service, services)
		model->appendRow(new TApp::TagaroGamePlugin(service));
}

TApp::TagaroGamePlugin::TagaroGamePlugin(KService::Ptr service)
	: m_service(service)
{
	setData(service->name(), Qt::DisplayRole);
	setData(service->genericName(), Qt::ToolTipRole);
	setData(KIcon(service->icon()), Qt::DecorationRole);
}

TApp::InstantiatorFlags TApp::TagaroGamePlugin::flags() const
{
	return 0;
}

bool TApp::TagaroGamePlugin::createInstance(QWidget*& widget)
{
	QString error;
	widget = m_service->createInstance<QWidget>(0, QVariantList(), &error);
	if (!widget)
		KMessageBox::detailedError(0, i18n("The game \"%1\" could not be launched.", m_service->name()), error);
	return (bool) widget;
}

bool TApp::TagaroGamePlugin::activateInstance(QWidget* widget)
{
	//TODO: when there is a manager for these widget, tell it to select the widget
	widget->show();
	widget->activateWindow();
	return true;
}

bool TApp::TagaroGamePlugin::deleteInstance(QWidget* widget)
{
	delete widget;
	return true;
}

//END TApp::TagaroGamePlugin
//BEGIN TApp::XdgAppPlugin

/*static*/ void TApp::XdgAppPlugin::loadInto(QStandardItemModel* model)
{
	KService::List services = KServiceTypeTrader::self()->query(
		QLatin1String("Application"),
		QLatin1String("'Game' in Categories and exist Exec and not ('tagaroshell' ~ Exec)")
	);
	foreach (KService::Ptr service, services)
		model->appendRow(new TApp::XdgAppPlugin(service));
}

TApp::XdgAppPlugin::XdgAppPlugin(KService::Ptr service)
	: m_service(service)
{
	setData(service->name(), Qt::DisplayRole);
	setData(service->genericName(), Qt::ToolTipRole);
	setData(KIcon(service->icon()), Qt::DecorationRole);
}

TApp::InstantiatorFlags TApp::XdgAppPlugin::flags() const
{
	return TApp::OutOfProcessInstance;
}

bool TApp::XdgAppPlugin::createInstance(QWidget*& widget)
{
	//TODO: Is it possible to get a QProcess instance out of KToolInvocation
	//      (or similar) to control the started application?
	widget = 0;
	KToolInvocation::startServiceByDesktopPath(m_service->entryPath());
	return true;
}

bool TApp::XdgAppPlugin::activateInstance(QWidget* widget)
{
	//dummy implementation (see todo item in createInstance())
	Q_UNUSED(widget)
	return true;
}

bool TApp::XdgAppPlugin::deleteInstance(QWidget* widget)
{
	//dummy implementation (see todo item in createInstance())
	Q_UNUSED(widget)
	return true;
}

//END TApp::XdgAppPlugin
