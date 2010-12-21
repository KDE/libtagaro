enable_testing()
project(libtagaro)

kde4_no_enable_final(libtagaro)

#################### compile ####################

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/graphics
)

set(tagaro_LIB_SRCS
	objectpool.cpp
	audio/audioscene.cpp
	audio/sound.cpp
	graphics/graphicsconfigdialog.cpp
	graphics/graphicsdelegate.cpp
	graphics/renderbackend.cpp
	graphics/rendererclient.cpp
	graphics/renderer.cpp
	graphics/renderermodule.cpp
	graphics/rendering.cpp
	graphics/sprite.cpp
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
target_link_libraries(tagaro ${KDE4_KDEUI_LIBS} ${QT_QTSVG_LIBRARY} ${QCA2_LIBRARIES} ${OPENAL_LIBRARY} ${SNDFILE_LIBRARIES})
target_link_libraries(tagaro LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS})

set_target_properties(tagaro PROPERTIES
	VERSION 0.1.0
	SOVERSION 0
	DEFINE_SYMBOL MAKE_TAGARO_LIB
)

#################### install ####################

install(TARGETS tagaro ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
	objectpool.h
	${CMAKE_CURRENT_BINARY_DIR}/settings.h
DESTINATION ${INCLUDE_INSTALL_DIR}/tagaro COMPONENT Devel)

install(FILES
	audio/audioscene.h
	audio/sound.h
DESTINATION ${INCLUDE_INSTALL_DIR}/tagaro/audio COMPONENT Devel)

install(FILES
	graphics/graphicsconfigdialog.h
	graphics/renderbackend.h
	graphics/rendererclient.h
	graphics/renderer.h
	graphics/sprite.h
	graphics/spriteitem.h
	graphics/spriteobjectitem.h
	graphics/theme.h
	graphics/themeprovider.h
DESTINATION ${INCLUDE_INSTALL_DIR}/tagaro/graphics COMPONENT Devel)

install(FILES
	interface/board.h
	interface/messageoverlay.h
	interface/scene.h
DESTINATION ${INCLUDE_INSTALL_DIR}/tagaro/interface COMPONENT Devel)
