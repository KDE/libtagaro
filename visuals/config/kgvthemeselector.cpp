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
#include "kgvtheme.h"
#include "kgvthemeprovider.h"
#include "ui_kgvthemeselector.h"

#include <QtGui/QPainter>

KgvThemeSelector::KgvThemeSelector(KgvThemeProvider* provider, KgvConfigDialog::ThemeSelectorOptions options)
	: m_provider(provider)
	, m_ui(new Ui_KgvThemeSelectorBase)
{
	connect(m_provider, SIGNAL(themesChanged(int, int)), SLOT(themesChanged(int, int)));
	connect(m_provider, SIGNAL(themesInserted(int, int)), SLOT(themesInserted(int, int)));
	connect(m_provider, SIGNAL(themesAboutToBeRemoved(int, int)), SLOT(themesRemoved(int, int)));
	//setup interface
	m_ui->setupUi(this);
	m_ui->getNewButton->setIcon(KIcon("get-hot-new-stuff"));
	m_ui->getNewButton->setVisible(options & KgvConfigDialog::WithNewStuffDownload);
	connect(m_ui->getNewButton, SIGNAL(clicked()), SLOT(openNewStuffDialog()));
	//setup theme list
	themesInserted(0, m_provider->themeCount() - 1);
	connect(m_ui->themeList, SIGNAL(itemSelectionChanged()), SLOT(themeSelectionChanged()));
	m_ui->themeList->setSelectionMode(QAbstractItemView::SingleSelection);
	QListWidgetItem* selectedItem = m_ui->themeList->item(m_provider->selectedIndex());
	m_ui->themeList->setCurrentItem(selectedItem, QItemSelectionModel::SelectCurrent);
}

KgvThemeSelector::~KgvThemeSelector()
{
	delete m_ui;
}

void KgvThemeSelector::themesInserted(int firstIndex, int lastIndex)
{
	QStringList themeNames;
	for (int index = firstIndex; index <= lastIndex; ++index)
	{
		themeNames << m_provider->theme(index)->data(KgvTheme::NameRole, QString()).toString();
	}
	m_ui->themeList->insertItems(firstIndex, themeNames);
}

void KgvThemeSelector::themesChanged(int firstIndex, int lastIndex)
{
	for (int index = firstIndex; index <= lastIndex; ++index)
	{
		QListWidgetItem* item = m_ui->themeList->item(index);
		item->setText(m_provider->theme(index)->data(KgvTheme::NameRole, QString()).toString());
	}
}

void KgvThemeSelector::themesRemoved(int firstIndex, int lastIndex)
{
	for (int index = lastIndex; index >= firstIndex; --index)
	{
		delete m_ui->themeList->takeItem(index);
	}
}

void KgvThemeSelector::themeSelectionChanged()
{
	const QListWidgetItem* selectedItem = m_ui->themeList->selectedItems().value(0);
	if (!selectedItem)
	{
		return;
	}
	const KgvTheme* theme = m_provider->theme(m_ui->themeList->row(selectedItem));
	//show detailed information about theme
	m_ui->themeDescription->setText(theme->data(KgvTheme::DescriptionRole).toString());
	m_ui->themeAuthor->setText(theme->data(KgvTheme::AuthorRole).toString());
	QString authorEmail = theme->data(KgvTheme::AuthorEmailRole).toString();
	if (!authorEmail.isEmpty())
	{
		authorEmail = QString::fromLatin1("<a href=\"mailto:%1\">%1</a>").arg(authorEmail);
	}
	m_ui->themeContact->setText(authorEmail);
	//show preview if available
	QPixmap preview = theme->data(KgvTheme::PreviewRole).value<QPixmap>();
	QPixmap thumbnail(m_ui->themePreview->minimumSize());
	thumbnail.fill(Qt::transparent);
	if (!preview.isNull())
	{
		//center preview in the thumbnail pixmap (this ensures that the pixmap passed to the label is always of the same size)
		preview = preview.scaled(m_ui->themePreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		const QSize sizeDelta = thumbnail.size() - preview.size();
		QPainter painter(&thumbnail);
		painter.drawPixmap(sizeDelta.width() / 2, sizeDelta.height() / 2, preview);
		painter.end();
	}
	m_ui->themePreview->setPixmap(thumbnail);
}

void KgvThemeSelector::openNewStuffDialog()
{
	//TODO
}

#include "kgvthemeselector_p.moc"
