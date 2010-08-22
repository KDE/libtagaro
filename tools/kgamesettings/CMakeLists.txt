project(kgamesettings)

set(kgamesettings_SRCS
	main.cpp
)
kde4_add_ui_files(kgamesettings_SRCS visuals.ui)

kde4_add_executable(kgamesettings ${kgamesettings_SRCS})
target_link_libraries(kgamesettings kgamevisuals)

install(TARGETS kgamesettings ${INSTALL_TARGETS_DEFAULT_ARGS})
