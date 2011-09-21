/*  This file is part of the KDE project
    Copyright 2007 David Faure <faure@kde.org>
    Copyright 2010 Stefan Majewsky <majewsky@gmx.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef LIBKGAME_EXPORT_H
#define LIBKGAME_EXPORT_H

#include <kdemacros.h>

//export macro for libtagaro

#ifndef KGAME_EXPORT
#	if defined(MAKE_KGAME_LIB)
#		define KGAME_EXPORT KDE_EXPORT //building the library
#	else
#		define KGAME_EXPORT KDE_IMPORT //using the library
#	endif
#endif

#ifndef KGAME_EXPORT_DEPRECATED
#	define KGAME_EXPORT_DEPRECATED KDE_DEPRECATED KGAME_EXPORT
#endif

#endif // LIBKGAME_EXPORT_H
