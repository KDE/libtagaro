enable_testing()
project(libkgamevisuals)

kde4_no_enable_final(libkgamevisuals)

find_package(LibKDEGames REQUIRED) # TODO: still required because of KGameTheme

#################### compile ####################

set(kgamevisuals_LIB_SRCS
	rendering/kgvrendererclient.cpp
	rendering/kgvrenderer.cpp
	rendering/kgvspriteitem.cpp
	rendering/kgvspriteobjectitem.cpp
)

kde4_add_library(kgamevisuals SHARED ${kgamevisuals_LIB_SRCS})
target_link_libraries(kgamevisuals ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTSVG_LIBRARY} ${KDEGAMES_LIBRARY})
target_link_libraries(kgamevisuals LINK_INTERFACE_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDEGAMES_LIBRARY})

set_target_properties(kgamevisuals PROPERTIES
	VERSION 0.1.0
	SOVERSION 0
)

#################### install ####################

install(TARGETS kgamevisuals ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
	rendering/kgvrendererclient.h
	rendering/kgvrenderer.h
	rendering/kgvspriteitem.h
	rendering/kgvspriteobjectitem.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
