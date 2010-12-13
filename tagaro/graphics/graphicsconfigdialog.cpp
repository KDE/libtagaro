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

#include "graphicsconfigdialog.h"
#include "themeprovider.h"
#include "themeselector_p.h"

#include <QtGui/QLayout>
#include <KDE/KLocale>

struct Tagaro::GraphicsConfigDialog::Private
{
	Tagaro::GraphicsConfigDialog* q;
	QList<Tagaro::ThemeSelector*> m_selectors;

	Private(Tagaro::GraphicsConfigDialog* q_) : q(q_) {}

	void _k_restoreDefault();
	void _k_selectionChanged();
};

Tagaro::GraphicsConfigDialog::GraphicsConfigDialog(const QString& title, QWidget* parent)
	: KPageDialog(parent)
	, d(new Private(this))
{
	setCaption(title.isEmpty() ? i18n("Configure Appearance") : title);
	setButtons(KDialog::Default | KDialog::Close);
	setFaceType(KPageDialog::Auto);
	enableButton(KDialog::Default, false); //nothing available yet to restore to default
	connect(this, SIGNAL(defaultClicked()), SLOT(_k_restoreDefault()));
}

Tagaro::GraphicsConfigDialog::~GraphicsConfigDialog()
{
	delete d;
}

void Tagaro::GraphicsConfigDialog::addThemeSelector(Tagaro::ThemeProvider* provider, const QString& itemName, const KIcon& icon, const QString& header)
{
	Tagaro::ThemeSelector* selector = new Tagaro::ThemeSelector(provider);
	//insert into dialog
	selector->layout()->setMargin(0); //for embedding in the dialog's layout
	KPageWidgetItem* page = addPage(selector, itemName);
	page->setIcon(icon);
	page->setHeader(header);
	//hook up to dialog
	d->m_selectors << selector;
	connect(provider, SIGNAL(selectedThemeChanged(const Tagaro::Theme*)), SLOT(_k_selectionChanged()));
	d->_k_selectionChanged(); //determine state of "Default" button
}

void Tagaro::GraphicsConfigDialog::Private::_k_restoreDefault()
{
	foreach (Tagaro::ThemeSelector* selector, m_selectors)
	{
		selector->provider()->setSelectedTheme(selector->provider()->defaultTheme());
	}
}

void Tagaro::GraphicsConfigDialog::Private::_k_selectionChanged()
{
	foreach (Tagaro::ThemeSelector* selector, m_selectors)
	{
		if (selector->provider()->selectedTheme() != selector->provider()->defaultTheme())
		{
			q->enableButton(KDialog::Default, true);
			return;
		}
	}
	q->enableButton(KDialog::Default, false);
}

#include "graphicsconfigdialog.moc"
