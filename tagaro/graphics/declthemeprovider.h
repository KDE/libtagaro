/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>                     *
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

#ifndef TAGARO_DECLTHEMEPROVIDER_H
#define TAGARO_DECLTHEMEPROVIDER_H

#include <QtDeclarative/QDeclarativeImageProvider>

#include <libtagaro_export.h>

namespace Tagaro
{

class ThemeProvider;

/**
 * @class Tagaro::DeclarativeThemeProvider declthemeprovider.h <Tagaro/DeclarativeThemeProvider>
 *
 * This class makes Tagaro::ThemeProvider available to QML Image elements.
 * @code
 * Tagaro::SomeThemeProvider tp(...);
 * QDeclarativeEngine engine;
 * ...
 * engine.addImageProvider(QLatin1String("foo"), new Tagaro::DeclarativeThemeProvider(&tp);
 * @endcode
 * The last line should be executed before any QML files are loaded into the
 * engine. The engine takes ownership of the eclarativeThemeProvider, so ensure
 * that the Tagaro::ThemeProvider lives at least as long as the engine.
 *
 * After that, images can be fetched in QML with "image://foo/" URLs
 * (where "theme" was the first argument to addImageProvider).
 * @code
 * import Qt 4.7
 * Image {
 *     source: "image://foo/mySprite~4"
 * }
 * @endcode
 * This will display frame no. 4 of the sprite "mySprite". Omit the tilde
 * and number if the sprite is not animated.
 *
 * FIXME: images are not reloaded on theme change (cf. QTBUG-14900)
 * TODO: merge this into ThemeProvider?
 * TODO: need QML access to other properties of Sprite (at least frameCount)
 */
class TAGARO_EXPORT DeclarativeThemeProvider : public QDeclarativeImageProvider
{
	public:
		DeclarativeThemeProvider(const Tagaro::ThemeProvider* tp);
		~DeclarativeThemeProvider();

		virtual QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);
	private:
		class Private;
		Private* const d;
};

} // namespace Tagaro

#endif // TAGARO_DECLTHEMEPROVIDER_H
