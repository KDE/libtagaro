project(kcmkdegames)

set(kcmkdegames_SRCS
	kcmkdegames.cpp
)
kde4_add_ui_files(kcmkdegames_SRCS visuals.ui)

kde4_add_plugin(kcmkdegames ${kcmkdegames_SRCS})
target_link_libraries(kcmkdegames kgame)

install(TARGETS kcmkdegames DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcmkdegames.desktop DESTINATION ${SERVICES_INSTALL_DIR})
