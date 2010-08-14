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

#include "kgvthemeselector_p.h"
#include "kgvgraphicsdelegate_p.h"
#include "kgvtheme.h"
#include "kgvthemeprovider.h"

#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <KDE/KLocale>

KgvThemeSelector::KgvThemeSelector(KgvThemeProvider* provider, KgvConfigDialog::ThemeSelectorOptions options)
	: m_provider(provider)
	, m_themeList(new QListWidget(this))
{
	connect(m_provider, SIGNAL(themesChanged(int, int)), SLOT(themesChanged(int, int)));
	connect(m_provider, SIGNAL(themesInserted(int, int)), SLOT(themesInserted(int, int)));
	connect(m_provider, SIGNAL(themesAboutToBeRemoved(int, int)), SLOT(themesRemoved(int, int)));
	//setup interface
	QVBoxLayout* layout = new QVBoxLayout;
	setLayout(layout);
	layout->addWidget(m_themeList);
	if (options & KgvConfigDialog::WithNewStuffDownload)
	{
		QPushButton* knsButton = new QPushButton(KIcon("get-hot-new-stuff"), i18n("Get New Themes..."), this);
		layout->addWidget(knsButton);
		connect(knsButton, SIGNAL(clicked()), SLOT(openNewStuffDialog()));
	}
	//setup theme list
	new KgvGraphicsDelegate(m_themeList);
	themesInserted(0, m_provider->themeCount() - 1);
	m_themeList->setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectedIndex(m_provider->selectedIndex());
	connect(m_themeList, SIGNAL(itemSelectionChanged()), SIGNAL(selectedIndexChanged()));
}

KgvThemeProvider* KgvThemeSelector::provider() const
{
	return m_provider;
}

int KgvThemeSelector::selectedIndex() const
{
	const QListWidgetItem* selectedItem = m_themeList->selectedItems().value(0);
	return selectedItem ? m_themeList->row(selectedItem) : 0;
}

void KgvThemeSelector::setSelectedIndex(int index)
{
	QListWidgetItem* selectedItem = m_themeList->item(index);
	m_themeList->setCurrentItem(selectedItem, QItemSelectionModel::SelectCurrent);
}

void KgvThemeSelector::themesInserted(int firstIndex, int lastIndex)
{
	//create empty items
	QStringList themeNames;
	for (int index = firstIndex; index <= lastIndex; ++index)
	{
		themeNames << QString();
	}
	m_themeList->insertItems(firstIndex, themeNames);
	//fill empty items with data
	themesChanged(firstIndex, lastIndex);
}

void KgvThemeSelector::themesChanged(int firstIndex, int lastIndex)
{
	for (int index = firstIndex; index <= lastIndex; ++index)
	{
		//get data from associated theme
		const KgvTheme* theme = m_provider->theme(index);
		const QString name = theme->data(KgvTheme::NameRole, QString()).toString();
		const QString description = theme->data(KgvTheme::DescriptionRole, QString()).toString();
		const QString author = theme->data(KgvTheme::AuthorRole, QString()).toString();
		const QString authorEmail = theme->data(KgvTheme::AuthorEmailRole, QString()).toString();
		const QPixmap preview = theme->data(KgvTheme::PreviewRole).value<QPixmap>();
		//update item with data
		QListWidgetItem* item = m_themeList->item(index);
		item->setData(Qt::DisplayRole, name);
		item->setData(KgvGraphicsDelegate::CommentRole, description);
		item->setData(KgvGraphicsDelegate::AuthorRole, author);
		item->setData(KgvGraphicsDelegate::AuthorEmailRole, authorEmail);
		item->setData(KgvGraphicsDelegate::ThumbnailRole, preview);
	}
}

void KgvThemeSelector::themesRemoved(int firstIndex, int lastIndex)
{
	for (int index = lastIndex; index >= firstIndex; --index)
	{
		delete m_themeList->takeItem(index);
	}
}

void KgvThemeSelector::openNewStuffDialog()
{
	//TODO
}

#include "kgvthemeselector_p.moc"
