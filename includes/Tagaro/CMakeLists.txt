#NOTE: Use the autogen-includes.sh script to update this file.
install(FILES
	AudioScene
	Board
	FileThemeProvider
	GraphicsConfigDialog
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
	Sprite
	SpriteItem
	SpriteObjectItem
	StandardTheme
	StandardThemeProvider
	Theme
	ThemeProvider
DESTINATION ${INCLUDE_INSTALL_DIR}/Tagaro COMPONENT Devel)
