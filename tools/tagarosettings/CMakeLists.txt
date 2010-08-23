project(tagarosettings)

set(tagarosettings_SRCS
	main.cpp
)
kde4_add_ui_files(tagarosettings_SRCS visuals.ui)

kde4_add_executable(tagarosettings ${tagarosettings_SRCS})
target_link_libraries(tagarosettings tagarovisuals)

install(TARGETS tagarosettings ${INSTALL_TARGETS_DEFAULT_ARGS})
