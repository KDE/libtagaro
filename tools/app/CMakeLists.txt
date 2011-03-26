set(tagaroapp_SRCS
	applist.cpp
	instantiable.cpp
	main.cpp
)
set(tagaroapp_LIBS tagaro)

if (GLUON_FOUND)
	add_definitions(-DTAGAROAPP_USE_GLUON)
	include_directories(${GLUON_INCLUDES})
	set(tagaroapp_LIBS ${tagaroapp_LIBS} ${GLUON_LIBS})
endif (GLUON_FOUND)

kde4_add_executable(tagaroapp ${tagaroapp_SRCS})
target_link_libraries(tagaroapp ${tagaroapp_LIBS})

install(TARGETS tagaroapp ${INSTALL_TARGETS_DEFAULT_ARGS})
