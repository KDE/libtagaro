project(kcmtagaro)

set(kcmtagaro_SRCS
	kcmtagaro.cpp
)
kde4_add_ui_files(kcmtagaro_SRCS visuals.ui)

kde4_add_plugin(kcmtagaro ${kcmtagaro_SRCS})
target_link_libraries(kcmtagaro tagaro)

install(TARGETS kcmtagaro DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcmtagaro.desktop DESTINATION ${SERVICES_INSTALL_DIR})
