#NOTE: Use the autogen-includes.sh script to update this file.
install(FILES
	AudioScene
	Board
	CachedProxyRenderBackend
	GraphicsConfigDialog
	MessageOverlay
	ObjectPointer
	ObjectPool
	RenderBackend
	RenderBehavior
	Renderer
	RendererClient
	Scene
	Settings
	SimpleThemeProvider
	Sound
	Sprite
	SpriteItem
	SpriteObjectItem
	StandardTheme
	StandardThemeProvider
	Theme
	ThemeProvider
DESTINATION ${INCLUDE_INSTALL_DIR}/Tagaro COMPONENT Devel)
