/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "kcmtagaro.h"

#include <KDE/KAboutData>
#include <KDE/KConfigDialogManager>
#include <KDE/KGenericFactory>

#include <Tagaro/Settings>
#include "ui_visuals.h"

typedef KGenericFactory<KCMTagaro, QWidget> KCMTagaroFactory;
K_EXPORT_COMPONENT_FACTORY(kcmtagaro, KCMTagaroFactory("kcmtagaro"))

struct KCMTagaro::Private
{
	Ui_Visuals m_visualsUi;
};

KCMTagaro::KCMTagaro(QWidget* parent, const QStringList& args)
	: KCModule(KCMTagaroFactory::componentData(), parent)
	, d(new Private)
{
	Q_UNUSED(args)
	KAboutData* about = new KAboutData(
		"kcmtagaro", "libtagaro", ki18nc("AboutData appname for KCM", "KDE games library configuration"),
		"0.1", ki18nc("AboutData description for KCM", "Global configuration for KDE games"),
		KAboutData::License_GPL, ki18n("Copyright 2010 Stefan Majewsky"));
	setAboutData(about);
	//setup configuration widgets
	d->m_visualsUi.setupUi(this);
	addConfig(Tagaro::Settings::self(), this);
}

KCMTagaro::~KCMTagaro()
{
	delete d;
}
