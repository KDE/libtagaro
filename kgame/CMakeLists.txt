enable_testing()
project(libkgame)

kde4_no_enable_final(libkgame)

#################### compile ####################

configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

set(kgame_LIB_SRCS
	audio/audioscene-${KGAMEAUDIO_BACKEND}.cpp
	audio/sound-${KGAMEAUDIO_BACKEND}.cpp
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
kde4_add_kcfg_files(kgame_LIB_SRCS settings.kcfgc)

kde4_add_library(kgame SHARED ${kgame_LIB_SRCS})
target_link_libraries(kgame ${KDE4_KDEUI_LIBS} ${QT_QTSVG_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} ${KGAMEAUDIO_LINKLIBS})
target_link_libraries(kgame LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS} ${QT_QTDECLARATIVE_LIBRARY})

set_target_properties(kgame PROPERTIES
	VERSION ${KGAME_VERSION} SOVERSION ${KGAME_SOVERSION}
	DEFINE_SYMBOL MAKE_KGAME_LIB
)

#################### install ####################

install(TARGETS kgame EXPORT KGameLibraryDepends ${INSTALL_TARGETS_DEFAULT_ARGS})

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
