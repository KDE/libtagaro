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

#include "configdialog.h"
#include "themeprovider.h"
#include "themeselector_p.h"

#include <QtGui/QLayout>
#include <KDE/KConfigSkeleton>
#include <KDE/KGlobal>

struct Tagaro::ConfigDialog::Private
{
	QList<Tagaro::ThemeSelector*> m_selectors;
};

K_GLOBAL_STATIC(KConfigSkeleton, g_dummyConfig)

//We explicitly allow to construct Tagaro::ConfigDialog without a KConfigSkeleton
//(i.e. config == 0). Because KConfigDialog itself does not allow it, we give it
//a dummy object in this case. Note that g_dummyConfig is not created if it is
//not needed.
Tagaro::ConfigDialog::ConfigDialog(QWidget* parent, const QString& name, KConfigSkeleton* config)
	: KConfigDialog(parent, name, config ? config : (KConfigSkeleton*) g_dummyConfig)
	, d(new Private)
{
}

Tagaro::ConfigDialog::~ConfigDialog()
{
	delete d;
}

void Tagaro::ConfigDialog::addThemeSelector(Tagaro::ThemeProviderPtr provider, Tagaro::ConfigDialog::ThemeSelectorOptions options, const QString& itemName, const QString& iconName, const QString& header)
{
	Tagaro::ThemeSelector* selector = new Tagaro::ThemeSelector(provider, options);
	selector->layout()->setMargin(0); //for embedding in the dialog's layout
	addPage(selector, itemName, iconName, header);
	d->m_selectors << selector;
	connect(selector, SIGNAL(selectedIndexChanged()), SLOT(settingsChangedSlot()));
}

bool Tagaro::ConfigDialog::hasChanged()
{
	foreach (Tagaro::ThemeSelector* selector, d->m_selectors)
	{
		if (selector->selectedIndex() != selector->provider()->selectedIndex())
		{
			return true;
		}
	}
	return false;
}

bool Tagaro::ConfigDialog::isDefault()
{
	foreach (Tagaro::ThemeSelector* selector, d->m_selectors)
	{
		if (selector->selectedIndex() != 0)
		{
			return false;
		}
	}
	return true;
}

void Tagaro::ConfigDialog::updateSettings()
{
	foreach (Tagaro::ThemeSelector* selector, d->m_selectors)
	{
		selector->provider()->setSelectedIndex(selector->selectedIndex());
	}
}

void Tagaro::ConfigDialog::updateWidgets()
{
	foreach (Tagaro::ThemeSelector* selector, d->m_selectors)
	{
		selector->setSelectedIndex(selector->provider()->selectedIndex());
	}
}

void Tagaro::ConfigDialog::updateWidgetsDefault()
{
	foreach (Tagaro::ThemeSelector* selector, d->m_selectors)
	{
		selector->setSelectedIndex(0);
	}
}
