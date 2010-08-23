enable_testing()
project(libtagarovisuals)

kde4_no_enable_final(libtagarovisuals)

#################### compile ####################

set(tagarovisuals_LIB_SRCS
	board.cpp
	configdialog.cpp
	graphicsdelegate.cpp
	rendererclient.cpp
	renderer.cpp
	spriteitem.cpp
	spriteobjectitem.cpp
	theme.cpp
	themeprovider.cpp
	themeselector.cpp
	view.cpp
)
kde4_add_kcfg_files(tagarovisuals_LIB_SRCS settings.kcfgc)

kde4_add_library(tagarovisuals SHARED ${tagarovisuals_LIB_SRCS})
target_link_libraries(tagarovisuals ${KDE4_KDEUI_LIBS} ${QT_QTSVG_LIBRARY})
target_link_libraries(tagarovisuals LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS})

set_target_properties(tagarovisuals PROPERTIES
	VERSION 0.1.0
	SOVERSION 0
	DEFINE_SYMBOL MAKE_TAGAROVISUALS_LIB
)

#################### install ####################

install(TARGETS tagarovisuals ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
	board.h
	configdialog.h
	rendererclient.h
	renderer.h
	spriteitem.h
	spriteobjectitem.h
	theme.h
	themeprovider.h
	view.h
	${CMAKE_CURRENT_BINARY_DIR}/settings.h
DESTINATION ${INCLUDE_INSTALL_DIR}/tagaro COMPONENT Devel)
