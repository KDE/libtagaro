#NOTE: Use the ./autogen-includes.sh script to update this file.
install(FILES
	AudioScene
	Board
	ConfigDialog
	DesktopThemeProvider
	FileThemeProvider
	MessageOverlay
	ObjectPointer
	ObjectPool
	QSvgRendererModule
	Renderer
	RendererClient
	RendererModule
	Scene
	Settings
	Sound
	SpriteItem
	SpriteObjectItem
	Theme
	ThemeProvider
DESTINATION ${INCLUDE_INSTALL_DIR}/Tagaro COMPONENT Devel)
