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

#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QScrollBar>
#include <QtGui/QVBoxLayout>
#include <KDE/KLocale>

Tagaro::ThemeSelector::ThemeSelector(Tagaro::ThemeProvider* provider, Tagaro::ConfigDialog::ThemeSelectorOptions options)
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
	if (options & Tagaro::ConfigDialog::WithNewStuffDownload)
	{
		QPushButton* knsButton = new QPushButton(KIcon("get-hot-new-stuff"), i18n("Get New Themes..."), this);
		layout->addWidget(knsButton);
		connect(knsButton, SIGNAL(clicked()), SLOT(openNewStuffDialog()));
	}
	//setup theme list
	themesInserted(0, m_provider->themeCount() - 1);
	m_themeList->setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectedIndex(m_provider->selectedIndex());
	connect(m_themeList, SIGNAL(itemSelectionChanged()), SIGNAL(selectedIndexChanged()));
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
	const QListWidgetItem* selectedItem = m_themeList->selectedItems().value(0);
	return selectedItem ? m_themeList->row(selectedItem) : 0;
}

void Tagaro::ThemeSelector::setSelectedIndex(int index)
{
	QListWidgetItem* selectedItem = m_themeList->item(index);
	m_themeList->setCurrentItem(selectedItem, QItemSelectionModel::SelectCurrent);
}

void Tagaro::ThemeSelector::themesInserted(int firstIndex, int lastIndex)
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

void Tagaro::ThemeSelector::themesChanged(int firstIndex, int lastIndex)
{
	for (int index = firstIndex; index <= lastIndex; ++index)
	{
		//get data from associated theme
		const Tagaro::Theme* theme = m_provider->theme(index);
		const QString name = theme->data(Tagaro::Theme::NameRole, QString()).toString();
		const QString description = theme->data(Tagaro::Theme::DescriptionRole, QString()).toString();
		const QString author = theme->data(Tagaro::Theme::AuthorRole, QString()).toString();
		const QString authorEmail = theme->data(Tagaro::Theme::AuthorEmailRole, QString()).toString();
		const QPixmap preview = theme->data(Tagaro::Theme::PreviewRole).value<QPixmap>();
		//update item with data
		QListWidgetItem* item = m_themeList->item(index);
		item->setData(Qt::DisplayRole, name);
		item->setData(Tagaro::GraphicsDelegate::CommentRole, description);
		item->setData(Tagaro::GraphicsDelegate::AuthorRole, author);
		item->setData(Tagaro::GraphicsDelegate::AuthorEmailRole, authorEmail);
		item->setData(Tagaro::GraphicsDelegate::ThumbnailRole, preview);
	}
}

void Tagaro::ThemeSelector::themesRemoved(int firstIndex, int lastIndex)
{
	for (int index = lastIndex; index >= firstIndex; --index)
	{
		delete m_themeList->takeItem(index);
	}
}

void Tagaro::ThemeSelector::openNewStuffDialog()
{
	//TODO
}

#include "themeselector_p.moc"
