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

#include "graphicsdelegate_p.h"
#include "theme.h" //for role names

#include <QtGui/QAbstractItemView>
#include <QtGui/QApplication>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>
#include <KDE/KLocalizedString>

namespace Metrics
{
	const int Padding = 6;
	const QSize ThumbnailBaseSize(64, 64);
}

Tagaro::GraphicsDelegate::GraphicsDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
	QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent);
	if (view)
		view->setItemDelegate(this);
}

QRect Tagaro::GraphicsDelegate::thumbnailRect(const QRect& baseRect) const
{
	QRect thumbnailBaseRect(QPoint(Metrics::Padding + baseRect.left(), 0), Metrics::ThumbnailBaseSize);
	thumbnailBaseRect.moveCenter(QPoint(thumbnailBaseRect.center().x(), baseRect.center().y()));
	if (QApplication::isRightToLeft())
		thumbnailBaseRect.moveRight(baseRect.right() - Metrics::Padding);
	return thumbnailBaseRect;
}

void Tagaro::GraphicsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	const bool rtl = option.direction == Qt::RightToLeft;
	QRect baseRect = option.rect;
	//draw background
	QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);
	//draw thumbnail
	QRect thumbnailBaseRect = this->thumbnailRect(baseRect);
	const QPixmap thumbnail = index.data(Qt::DecorationRole).value<QPixmap>().scaled(Metrics::ThumbnailBaseSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QRect thumbnailRect(thumbnailBaseRect.topLeft(), thumbnail.size());
	thumbnailRect.translate( //center inside thumbnailBaseRect
		(thumbnailBaseRect.width() - thumbnailRect.width()) / 2,
		(thumbnailBaseRect.height() - thumbnailRect.height()) / 2
	);
	painter->drawPixmap(thumbnailRect.topLeft(), thumbnail);
	//find metrics: text
	QStringList texts; QList<QFont> fonts;
	{
		QString name = index.data(Qt::DisplayRole).toString();
		if (name.isEmpty())
			name = i18n("[No name]");
		texts << name;
		QFont theFont(painter->font()); theFont.setBold(true); fonts << theFont;
	}{
		QString comment = index.data(DescriptionRole).toString();
		if (!comment.isEmpty())
		{
			texts << comment;
			fonts << painter->font();
		}
	}{
		QString author = index.data(AuthorRole).toString();
		if (!author.isEmpty())
		{
			const QString authorString = ki18nc("Author attribution, e.g. \"by Jack\"", "by %1").subs(author).toString();
			texts << authorString;
			QFont theFont(painter->font()); theFont.setItalic(true); fonts << theFont;
		}
	}
	//TODO: display AuthorEmailRole
	QList<QRect> textRects; int totalTextHeight = 0;
	for (int i = 0; i < texts.count(); ++i)
	{
		QFontMetrics fm(fonts[i]);
		textRects << fm.boundingRect(texts[i]);
		textRects[i].setHeight(qMax(textRects[i].height(), fm.lineSpacing()));
		totalTextHeight += textRects[i].height();
	}
	QRect textBaseRect(baseRect);
	if (rtl)
	{
		textBaseRect.setRight(thumbnailBaseRect.left() - Metrics::Padding);
		textBaseRect.adjust(Metrics::Padding, Metrics::Padding, 0, -Metrics::Padding);
	}
	else
	{
		textBaseRect.setLeft(thumbnailBaseRect.right() + Metrics::Padding);
		textBaseRect.adjust(0, Metrics::Padding, -Metrics::Padding, -Metrics::Padding);
	}
	textBaseRect.setHeight(totalTextHeight);
	textBaseRect.moveTop(baseRect.top() + (baseRect.height() - textBaseRect.height()) / 2);
	//draw texts
	QRect currentTextRect(textBaseRect);
	painter->save();
	for (int i = 0; i < texts.count(); ++i)
	{
		painter->setFont(fonts[i]);
		const QRect& textRect = textRects[i];
		currentTextRect.setHeight(textRect.height());
		const QFontMetrics fm(fonts[i]);
		const QString text = fm.elidedText(texts[i], Qt::ElideRight, currentTextRect.width());
		painter->drawText(currentTextRect, Qt::AlignLeft | Qt::AlignVCenter, text);
		currentTextRect.moveTop(currentTextRect.bottom());
	}
	painter->restore();
}

QSize Tagaro::GraphicsDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option) Q_UNUSED(index)
	//TODO: take text size into account
	return QSize(400, Metrics::ThumbnailBaseSize.height() + 2 * Metrics::Padding);
}
