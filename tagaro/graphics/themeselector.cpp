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

#include "themeselector_p.h"
#include "graphicsdelegate_p.h"
#include "theme.h"
#include "themeprovider.h"

#include <QtGui/QListView>
#include <QtGui/QPushButton>
#include <QtGui/QScrollBar>
#include <QtGui/QVBoxLayout>
#include <KDE/KLocale>

Tagaro::ThemeSelector::ThemeSelector(Tagaro::ThemeProvider* provider, Tagaro::ConfigDialog::ThemeSelectorOptions options)
	: m_provider(provider)
	, m_themeList(new QListView(this))
{
	//setup interface
	QVBoxLayout* layout = new QVBoxLayout;
	setLayout(layout);
	layout->addWidget(m_themeList);
	if (options & Tagaro::ConfigDialog::WithNewStuffDownload)
	{
		QPushButton* knsButton = new QPushButton(KIcon("get-hot-new-stuff"), i18n("Get New Themes..."), this);
		knsButton->setEnabled(false); //KNS not implemented yet
		layout->addWidget(knsButton);
		connect(knsButton, SIGNAL(clicked()), SLOT(openNewStuffDialog()));
	}
	//setup theme list
	m_themeList->setModel(provider->model());
	m_themeList->setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectedIndex(m_provider->selectedIndex());
	connect(m_themeList->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SIGNAL(selectedIndexChanged()));
	//setup appearance of theme list (minimum size = 4 items)
	Tagaro::GraphicsDelegate* delegate = new Tagaro::GraphicsDelegate(m_themeList);
	const QSize itemSizeHint = delegate->sizeHint(QStyleOptionViewItem(), QModelIndex());
	const QSize scrollBarSizeHint = m_themeList->verticalScrollBar()->sizeHint();
	m_themeList->setMinimumSize(itemSizeHint.width() + 2 * scrollBarSizeHint.width(), 4 * itemSizeHint.height());
}

Tagaro::ThemeProvider* Tagaro::ThemeSelector::provider() const
{
	return m_provider;
}

int Tagaro::ThemeSelector::selectedIndex() const
{
	const QModelIndex selectedIndex = m_themeList->selectionModel()->selectedIndexes().value(0);
	return selectedIndex.isValid() ? selectedIndex.row() : 0;
}

void Tagaro::ThemeSelector::setSelectedIndex(int index)
{
	m_themeList->selectionModel()->setCurrentIndex(m_themeList->model()->index(index, 0), QItemSelectionModel::ClearAndSelect);
}

void Tagaro::ThemeSelector::openNewStuffDialog()
{
	//TODO: implement KNS support
}

#include "themeselector_p.moc"
