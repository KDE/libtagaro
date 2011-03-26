set(tagaroshell_SRCS
	main.cpp
)

kde4_add_executable(tagaroshell ${tagaroshell_SRCS})
target_link_libraries(tagaroshell tagaro)

install(TARGETS tagaroshell ${INSTALL_TARGETS_DEFAULT_ARGS})
