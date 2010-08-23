enable_testing()
project(libtagaro)

kde4_no_enable_final(libtagaro)

#################### compile ####################

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/graphics
)

set(tagaro_LIB_SRCS
	configdialog.cpp
	graphics/board.cpp
	graphics/graphicsdelegate.cpp
	graphics/rendererclient.cpp
	graphics/renderer.cpp
	graphics/spriteitem.cpp
	graphics/spriteobjectitem.cpp
	graphics/theme.cpp
	graphics/themeprovider.cpp
	graphics/themeselector.cpp
	graphics/view.cpp
)
kde4_add_kcfg_files(tagaro_LIB_SRCS settings.kcfgc)

kde4_add_library(tagaro SHARED ${tagaro_LIB_SRCS})
target_link_libraries(tagaro ${KDE4_KDEUI_LIBS} ${QT_QTSVG_LIBRARY})
target_link_libraries(tagaro LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS})

set_target_properties(tagaro PROPERTIES
	VERSION 0.1.0
	SOVERSION 0
	DEFINE_SYMBOL MAKE_TAGARO_LIB
)

#################### install ####################

install(TARGETS tagaro ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
	configdialog.h
	${CMAKE_CURRENT_BINARY_DIR}/settings.h
DESTINATION ${INCLUDE_INSTALL_DIR}/tagaro COMPONENT Devel)

install(FILES
	graphics/board.h
	graphics/rendererclient.h
	graphics/renderer.h
	graphics/spriteitem.h
	graphics/spriteobjectitem.h
	graphics/theme.h
	graphics/themeprovider.h
	graphics/view.h
DESTINATION ${INCLUDE_INSTALL_DIR}/tagaro/graphics COMPONENT Devel)
