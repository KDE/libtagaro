enable_testing()
project(libtagaro)

kde4_no_enable_final(libtagaro)

#################### compile ####################

configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

set(tagaro_LIB_SRCS
	audio/audioscene-${TAGAROAUDIO_BACKEND}.cpp
	audio/sound-${TAGAROAUDIO_BACKEND}.cpp
	core/application.cpp
	graphics/declthemeprovider.cpp
	graphics/graphicsconfigdialog.cpp
	graphics/graphicsdelegate.cpp
	graphics/graphicssource.cpp
	graphics/graphicssources.cpp
	graphics/graphicssourceconfig.cpp
	graphics/sprite.cpp
	graphics/spriteclient.cpp
	graphics/spriteitem.cpp
	graphics/spriteobjectitem.cpp
	graphics/theme.cpp
	graphics/themeprovider.cpp
	graphics/themeselector.cpp
	interface/board.cpp
	interface/messageoverlay.cpp
	interface/scene.cpp
)
kde4_add_kcfg_files(tagaro_LIB_SRCS settings.kcfgc)

kde4_add_library(tagaro SHARED ${tagaro_LIB_SRCS})
target_link_libraries(tagaro ${KDE4_KDEUI_LIBS} ${QT_QTSVG_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} ${TAGAROAUDIO_LINKLIBS})
target_link_libraries(tagaro LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS} ${QT_QTDECLARATIVE_LIBRARY})

set_target_properties(tagaro PROPERTIES
	VERSION ${TAGARO_VERSION} SOVERSION ${TAGARO_SOVERSION}
	DEFINE_SYMBOL MAKE_TAGARO_LIB
)

#################### install ####################

install(TARGETS tagaro EXPORT TagaroLibraryDepends ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/settings.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kgame COMPONENT Devel)

install(FILES
	audio/audioscene.h
	audio/sound.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kgame/audio COMPONENT Devel)

install(FILES
	core/application.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kgame/core COMPONENT Devel)

install(FILES
	graphics/declthemeprovider.h
	graphics/graphicsconfigdialog.h
	graphics/graphicssource.h
	graphics/graphicssourceconfig.h
	graphics/sprite.h
	graphics/spriteclient.h
	graphics/spriteitem.h
	graphics/spriteobjectitem.h
	graphics/theme.h
	graphics/themeprovider.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kgame/graphics COMPONENT Devel)

install(FILES
	interface/board.h
	interface/messageoverlay.h
	interface/scene.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kgame/interface COMPONENT Devel)
