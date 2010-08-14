/***************************************************************************
 *   Copyright 2009-2010 Stefan Majewsky <majewsky@gmx.net>                *
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

#ifndef KGVGRAPHICSDELEGATE_P_H
#define KGVGRAPHICSDELEGATE_P_H

#include <QtGui/QStyledItemDelegate>

//This delegate can be used in list views which allow to select graphical components (such as themes).
class KgvGraphicsDelegate : public QStyledItemDelegate
{
	public:
		enum SpecialRole
		{
			CommentRole = Qt::UserRole + 1, ///< For additional descriptions beyond the name in Qt::DisplayRole.
			ThumbnailRole,
			AuthorRole,
			AuthorEmailRole,
			UserRole ///< Users of KgvGraphicsDelegate can define their own roles above this index.
		};

		KgvGraphicsDelegate(QObject* parent = 0);
		virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
		virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
		QRect thumbnailRect(const QRect& baseRect) const;
};

#endif // KGVGRAPHICSDELEGATE_P_H
