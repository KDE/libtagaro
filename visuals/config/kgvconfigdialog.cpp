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

#include "kgvconfigdialog.h"
#include "kgvthemeselector_p.h"

#include <QtGui/QLayout>

struct KgvConfigDialog::Private
{
	QList<KgvThemeSelector*> m_selectors;
};

KgvConfigDialog::KgvConfigDialog(QWidget* parent, const QString& name, KConfigSkeleton* config)
	: KConfigDialog(parent, name, config)
	, d(new Private)
{
}

KgvConfigDialog::~KgvConfigDialog()
{
	delete d;
}

void KgvConfigDialog::addThemeSelector(KgvThemeProvider* provider, KgvConfigDialog::ThemeSelectorOptions options, const QString& itemName, const QString& iconName, const QString& header)
{
	KgvThemeSelector* selector = new KgvThemeSelector(provider, options);
	selector->layout()->setMargin(0); //for embedding in the dialog's layout
	addPage(selector, itemName, iconName, header);
}

bool KgvConfigDialog::hasChanged()
{
	//TODO
}

bool KgvConfigDialog::isDefault()
{
	//TODO
}

void KgvConfigDialog::updateSettings()
{
	//TODO
}

void KgvConfigDialog::updateWidgets()
{
	//TODO
}

void KgvConfigDialog::updateWidgetsDefault()
{
	//TODO
}
