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

#include "difficultyslider.h"
#include "../core/difficulty.h"

#include <QtGui/QLabel>
#include <QtGui/QSlider>
#include <KDE/KColorScheme>
#include <KDE/KColorUtils>

struct KGame::DifficultySlider::Private
{
	QSlider* m_slider;
	QLabel* m_label;

	KGame::DifficultySlider* q;
	KGame::Difficulty* m_difficulty;
	const KGame::DifficultyLevel* m_selectedLevel;

	Private(KGame::Difficulty* difficulty, KGame::DifficultySlider* q);
	void _t_sliderMoved(int index);
	void _t_valueChanged(int index);
	void display(int index, const KGame::DifficultyLevel* level);
};

KGame::DifficultySlider::DifficultySlider(KGame::Difficulty* difficulty, QWidget* parent)
	: QWidget(parent)
	, d(new Private(difficulty, this))
{
	select(difficulty->currentLevel());
}

KGame::DifficultySlider::Private::Private(KGame::Difficulty* difficulty, KGame::DifficultySlider* q)
	: m_slider(new QSlider(Qt::Horizontal, q))
	, m_label(new QLabel(q))
	, q(q)
	, m_difficulty(difficulty)
	, m_selectedLevel(0)
{
	m_slider->setMinimum(0);
	q->setSizePolicy(m_slider->sizePolicy());
	connect(m_slider, SIGNAL(sliderMoved(int)), q, SLOT(_t_sliderMoved(int)));
	connect(m_slider, SIGNAL(valueChanged(int)), q, SLOT(_t_valueChanged(int)));
	//adjust level font
	QFont font = m_label->font();
	font.setPointSize(1.3 * font.pointSize());
	font.setBold(true);
	m_label->setFont(font);
}

const KGame::DifficultyLevel* KGame::DifficultySlider::selectedLevel() const
{
	return d->m_selectedLevel;
}

void KGame::DifficultySlider::select(const KGame::DifficultyLevel* level)
{
	const int index = d->m_difficulty->levels().indexOf(level);
	if (index == -1 || level == d->m_selectedLevel)
	{
		return;
	}
	d->m_selectedLevel = level;
	d->display(index, level);
	emit selected(level);
}

void KGame::DifficultySlider::Private::display(int index, const KGame::DifficultyLevel* level)
{
	const QList<const KGame::DifficultyLevel*> levels = m_difficulty->levels();
	m_slider->setMaximum(levels.count() - 1);
	if (index != m_slider->value())
	{
		m_slider->setValue(index);
	}
	m_label->setText(level->title());
	//resizeEvent() implements layouting
	q->resizeEvent(0);
	//determine font color of label
	KColorScheme scheme(m_label->palette().currentColorGroup());
	const int minHardness = levels.front()->hardness();
	const int maxHardness = levels.back()->hardness();
	const int hardness = level->hardness();
	const qreal scale = qreal(hardness - minHardness) / (maxHardness - minHardness);
	QColor result;
	if (scale < 0.5)
	{
		const QColor c1 = scheme.foreground(KColorScheme::PositiveText).color();
		const QColor c2 = scheme.foreground(KColorScheme::NeutralText).color();
		result = KColorUtils::mix(c1, c2, scale / 0.5);
	}
	else
	{
		const QColor c1 = scheme.foreground(KColorScheme::NeutralText).color();
		const QColor c2 = scheme.foreground(KColorScheme::NegativeText).color();
		result = KColorUtils::mix(c1, c2, scale / 0.5 - 1);
	}
	QPalette p = m_label->palette();
	p.setColor(QPalette::WindowText, result);
	m_label->setPalette(p);
}

void KGame::DifficultySlider::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED(event)
	const QSize size = this->size();
	//position of slider
	QSize sliderSize(size.width(), d->m_slider->sizeHint().height());
	d->m_slider->setGeometry(QRect(QPoint(), sliderSize));
	//position of label
	const qreal scalePos = qreal(d->m_slider->value()) / d->m_slider->maximum();
	QRect labelRect(QPoint(), d->m_label->sizeHint());
	labelRect.moveCenter(QPoint(scalePos * size.width(), 0));
	labelRect.moveTop(sliderSize.height());
	if (labelRect.right() > size.width())
	{
		labelRect.moveRight(size.width());
	}
	if (labelRect.left() < 0)
	{
		labelRect.moveLeft(0);
	}
	d->m_label->setGeometry(labelRect);
}

void KGame::DifficultySlider::Private::_t_sliderMoved(int index)
{
	display(index, m_difficulty->levels().value(index));
}

void KGame::DifficultySlider::Private::_t_valueChanged(int index)
{
	q->select(m_difficulty->levels().value(index));
}

QSize KGame::DifficultySlider::minimumSizeHint() const
{
	const QSize s1 = d->m_slider->minimumSizeHint();
	const QSize s2 = d->m_label->minimumSizeHint();
	return QSize(qMax<int>(s1.width(), s2.width()), s1.height() + s2.height());
}

QSize KGame::DifficultySlider::sizeHint() const
{
	const QSize s1 = d->m_slider->sizeHint();
	const QSize s2 = d->m_label->sizeHint();
	return QSize(qMax<int>(s1.width(), s2.width()), s1.height() + s2.height());
}

#include "difficultyslider.moc"
