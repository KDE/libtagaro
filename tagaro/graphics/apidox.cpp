/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>                     *
 *   Copyright 2011 Jeffrey Kelling <kelling.jeffrey@ages-skripte.org>     *
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

//This file is not compiled. It only contains general API documentation.

/**
@page tagarographics TagaroGraphics overview

Applications with 2D graphics usually build their interface from sprites. For
example, a card game may define 52 sprites which represent the single cards.
Sprites may also be animated, like the avatar in a jump'n'run game. In this
case, the animated sprite usually consists of multiple images ("frames") which
are shown in a defined order and speed.

Many games have exchangable themes, that is: sets of sprites with identical
meaning, but a distinct visual appearance. For example, a card game may offer
multiple carddeck themes. Each theme must contain 52 sprites which represent
the single cards.

TagaroGraphics is a rendering framework that makes it easy to manage themes
and sprites. It also understands animated sprites consisting of multiple frames.

@section basic-workflow Basic workflow

Themes are created and managed by a ThemeProvider, e.g. StandardThemeProvider
locates theme description files in the KStandardDirs. The ThemeProvider keeps
track of the selected theme, i.e. the theme which is used to actually render
pixmaps. The theme provider also creates KGame::Sprite instances, which
represent single sprites.

The sprites are theme-independent, but tied to one theme provider. This is
because all themes in one theme provider shall provide the same
(application-defined) set of sprites. Some applications have multiple sets of
sprites for which themes can be selected independently (e.g. foreground and
background elements). In this case, multiple theme providers need to be
instantiated, which manage the corresponding themes.

KGame::Sprite provides a synchronous interface (KGame::Sprite::pixmap), but
you will usually use KGame::SpriteClient instances. This approach has multiple
advantages:

	@li Sprite clients work in a fire-and-forget manner. You set them up once,
	and they will fetch their pixmaps themselves, and update them automatically
	when the theme provider selects another theme.

	@li Sprite clients fetch pixmaps asynchronously, which allows Tagaro to use
	multi-threaded rendering for complex graphics sources (e.g. big SVG files).

	@li KGame::SpriteClient subclasses for QGraphicsView are already
	available. They work similar to QGraphicsPixmapItem or QGraphicsSvgItem.
	The only difference is that you have to specify a render size: Sprites are
	usually size-independent, so each client must define the size of the pixmap
	which it wants to display. You usually want this to match the screen size
	of the client to improve painting speed. Calculating the render size can be
	automated by using KGame::Board from TagaroInterface.

@section theme-structure Structure of a theme

Formally speaking, a theme is an entity consisting of some metadata (name,
author, etc.) plus one one or multiple graphics sources (e.g. graphics files).
Each graphics source contains a set of graphical elements. See @ref
theme-file-format for the standard representation of themes as files.

Both sprites and elements of graphics sources are identified by QString keys.
Therefore, there is an ambiguity when there are multiple graphics sources in
one theme: If a sprite "foo" is requested, which graphics source shall be used?
To answer this question, KGame::Theme maintains a mapping table which maps
sprite keys to graphics sources. By using regular expressions, the sprite keys
can also be rewritten systematically before they are used as element keys.

For example, KDiamond defines sprites "kdiamond-red", "kdiamond-green",
"kdiamond-blue" etc. for the different diamonds. A simple debug theme might
want to use the plain color source as the graphics source for these sprites.
(See @ref graphicssources for more information on sources.) This source has
elements "red", "green" and "blue", so the sprite key is different from the
right element key, and a regular expression has to be used. Using a standard
theme definition file, this is accomplished by:

@code
[Sources][default]
SourceType=color

[Mappings][default]
1-sprite=kdiamond-(.*)
1-element=%1
@endcode

See below for the definition of this format.

@section theme-file-format Format of theme definition files

Standard theme definition files are standalone KConfig files. They must contain
a group called "Tagaro Theme" ("KGameTheme" is also allowed for backwards
compatibility) which contains general metadata for this theme.

	@li Name
	@li Description (optional)
	@li Author
	@li AuthorEmail (optional)
	@li Preview (optional)

The string in preview is interpreted as path to a raster image that contains
the preview. Because of backwards compatibility to KGameTheme, an additional
key called "FileName" is recognized, which instantiates the default graphics
source. See below for details.

[Beware: What follows is a lengthy explanation of all the bells and whistles
that KGame::Theme offers for themes with multiple graphics sources. If your
theme has only one SVG file, go on to the last sentence of this section.]

The file may also contain a group "Sources" which defines the graphics sources
of this theme. Each graphics source corresponds to one group below the
"Sources" group. The group's key -- the source identifier -- is currently only
used inside this file, though it may later be used by Tagaro. (It's also
exposed in the API as the argument to KGame::Theme::source.)

Each source group must contain a key "SourceType". The value specifies the type
of graphics source. The general format is "type:location", where either type or
location may be omitted. (The colon is then omitted, too.) Omitting the type is
identical to using the type "auto", and triggers automatic type recognition.
The meaning of the location depends on the graphics source. See
@ref graphicssources for details.

The file may also contain a group "Mappings" below which may be a group for
each of the source groups (with the same name). This group defines the mappings
which are served by this graphics source. Each mapping is represented as two
key-value pairs. The first is "N-sprite" (where N is an arbitrary positive
integer); the value is a regular expression. The second is "N-element" (where N
is obviously the same integer); the value is the corresponding element key.

The mapping applies to all sprite keys which match the regular expression. The
element key of the graphics source is constructed by replacing %0, %1, %2 etc.
in the given element key by the regex's captures.

As a full example, the following KDiamond theme uses the image "foo.jpg" for
the window background (sprite "kdiamond-background") and rectangles with plain
color filling for the diamonds (sprites "kdiamond-red", "kdiamond-green" etc.):

@code
[Tagaro Theme]
Name=Debug Theme
Description=No complicated vector drawings or animations.
Author=John Doe
AuthorEmail=john.doe@example.org

[Sources][bgimage]
SourceType=foo.jpg

[Sources][color]
SourceType=color

[Mappings][bgimage]
1-sprite=kdiamond-background
1-element=full # The image source defines the element key "full".

[Mappings][color]
1-sprite=kdiamond-(.*)
1-element=%1 # e.g. element "red" for sprite "kdiamond-red"
@endcode

Most themes will only use one graphics source, e.g. a SVG file which can
contain arbitrarily many arbitrarily complex elements. They will also not need
to use the mapping table because the source file has been made specifically for
the application, using the sprite keys as element keys. KGame::Theme has
various shortcuts for this usecase:

	@li If the mapping table is empty (or no mappings apply), the element key
	    is set equal to the sprite key and the default graphics source with the
		identifier "default" is used. This is like writing by-hand the
		following mapping rule:

@code
[Mappings][default]
1-sprite=(.*)
1-element=%1
@endcode

	@li If the "Tagaro Theme"/"KGameTheme" group of the theme definition file
	    contains the key "FileName", its value is used to instantiate the
		"default" graphics source. Writing "FileName=foo" in this group is
		therefore equivalent to:

@code
[Sources][default]
SourceType=foo
@endcode

So if you have a single SVG file for your theme, just write "FileName=foo.svg"
in the "Tagaro Theme" or "KGameTheme" group and you're done.

@section graphicssources Graphics sources

This section describes the available graphics sources.

The "svg" source renders named elements from an SVG file. To instantiate, use
the type "svg"; the automatic type recognition understands the standard file
suffixes ".svg" and ".svgz". Examples:

@code
SourceType=svg:path/to/file.with.weird.ext
SourceType=auto:foo.svg
SourceType=bar.svgz
@endcode

The "image" source renders raster images. To instantiate, use the type "image";
the automatic type recognition understands all image file suffixes known to
QImageReader. Examples:

@code
SourceType=image:path/to/file.with.weird.ext
SourceType=auto:foo.jpg
SourceType=bar.png
@endcode

The "image" source defines an element "full" which corresponds to the full
image, but one can define more elements corresponding to parts of the image.
To do so, add entries to the config group defining the source (i.e. the one
which contains the "SourceType" key). The key is the new element key, the
value is the geometry of the image part in X11 syntax: "WxH+X+Y", where W
and H are width and height of the part, and X and Y are the coordinates of
the top-left corner of the parts. Attention: Nothing is optional. Example:
(This splits a 200x200 image into four equal-sized parts.)

@code
topleft=50x50+0+0
topright=50x50+50+0
bottomleft=50x50+0+50
bottomright=50x50+50+50
@endcode

The "ccsvg" source works like the "svg" source. It is able to replace a given
color key (default is #ff8989) by a color given as processingInstruction. The
implementation performs a simple text replacement on the SVG document, so it is
not rock solid. It is not covered by the automatic type recognition.

@code
SourceType=ccsvg:path/to/file.svg
ColorKey=#ffffff
@endcode

*/
