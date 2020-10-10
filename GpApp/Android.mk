LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := GpApp

SDL_PATH := ../SDL2

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../GpCommon	\
	$(LOCAL_PATH)/../Common	\
	$(LOCAL_PATH)/../PortabilityLayer

LOCAL_CFLAGS := -DGP_DEBUG_CONFIG=0

# Add your application source files here...
LOCAL_SRC_FILES := \
	About.cpp	\
	AnimCursor.cpp	\
	AppleEvents.cpp	\
	Banner.cpp	\
	ColorUtils.cpp	\
	Coordinates.cpp	\
	DialogUtils.cpp	\
	DynamicMaps.cpp	\
	Dynamics.cpp	\
	Dynamics2.cpp	\
	Dynamics3.cpp	\
	Environ.cpp	\
	Events.cpp	\
	FileError.cpp	\
	GameOver.cpp	\
	GpAppInterface.cpp	\
	Grease.cpp	\
	HighScores.cpp	\
	House.cpp	\
	HouseInfo.cpp	\
	HouseIO.cpp	\
	HouseLegal.cpp	\
	Input.cpp	\
	Interactions.cpp	\
	InterfaceInit.cpp	\
	Link.cpp	\
	Main.cpp	\
	MainWindow.cpp	\
	Map.cpp	\
	Marquee.cpp	\
	Menu.cpp	\
	Modes.cpp	\
	Music.cpp	\
	ObjectAdd.cpp	\
	ObjectDraw.cpp	\
	ObjectDraw2.cpp	\
	ObjectDrawAll.cpp	\
	ObjectEdit.cpp	\
	ObjectInfo.cpp	\
	ObjectRects.cpp	\
	Objects.cpp	\
	Play.cpp	\
	Player.cpp	\
	Prefs.cpp	\
	RectUtils.cpp	\
	Render.cpp	\
	Room.cpp	\
	RoomGraphics.cpp	\
	RoomInfo.cpp	\
	RubberBands.cpp	\
	SavedGames.cpp	\
	Scoreboard.cpp	\
	Scrap.cpp	\
	SelectHouse.cpp	\
	Settings.cpp	\
	Sound.cpp	\
	SoundSync_Cpp11.cpp	\
	StringUtils.cpp	\
	StructuresInit.cpp	\
	StructuresInit2.cpp	\
	Tools.cpp	\
	Transit.cpp	\
	Transitions.cpp	\
	Triggers.cpp	\
	Trip.cpp	\
	Utilities.cpp	\
	WindowUtils.cpp

LOCAL_STATIC_LIBRARIES := PortabilityLayer

include $(BUILD_SHARED_LIBRARY)
