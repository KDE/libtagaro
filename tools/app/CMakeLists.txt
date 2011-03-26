set(tagaroapp_SRCS
	applist.cpp
	instantiable.cpp
	main.cpp
)

kde4_add_executable(tagaroapp ${tagaroapp_SRCS})
target_link_libraries(tagaroapp tagaro)

install(TARGETS tagaroapp ${INSTALL_TARGETS_DEFAULT_ARGS})
