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

#include "applist.h"
#include "instantiable.h"

#include <KDE/KGlobal>

//BEGIN TApp::AppListModel

namespace TApp
{
	class AppListModel : public QStandardItemModel
	{
		public:
			AppListModel();
			virtual ~AppListModel();
	};
}

TApp::AppListModel::AppListModel()
{
	TApp::TagaroGamePlugin::loadInto(this);
#ifdef TAGAROAPP_USE_GLUON
	TApp::GluonGameFile::loadInto(this);
#endif
	TApp::XdgAppPlugin::loadInto(this);
}

TApp::AppListModel::~AppListModel()
{
	const int count = rowCount();
	for (int i = 0; i < count; ++i)
	{
		TApp::Instantiable* inst = dynamic_cast<TApp::Instantiable*>(item(i));
		if (inst)
			inst->close();
	}
}

//END TApp::AppListModel
//BEGIN TApp::AppListView

K_GLOBAL_STATIC(TApp::AppListModel, g_model)

TApp::AppListView::AppListView(QWidget* parent)
	: QListView(parent)
{
	qRegisterMetaType<TApp::Instantiable*>();
	setModel(g_model);
	setViewMode(QListView::IconMode);
	setMovement(QListView::Snap);
	setResizeMode(QListView::Adjust);
	setGridSize(QSize(80, 80));
	connect(this, SIGNAL(activated(QModelIndex)), SLOT(handleActivated(QModelIndex)));
}

void TApp::AppListView::handleActivated(const QModelIndex& index)
{
	QStandardItem* item = g_model->itemFromIndex(index);
	TApp::Instantiable* inst = dynamic_cast<TApp::Instantiable*>(item);
	if (inst)
		emit selected(inst);
}

//END TApp::AppListView

#include "applist.moc"
