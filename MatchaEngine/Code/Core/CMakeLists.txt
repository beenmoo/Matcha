target_sources(${TARGET_NAME} PRIVATE
    Application.cpp
    Application.h
    Assert.h
    Debug.h
    EngineContext.cpp
    EngineContext.h
    EntryPoint.h
    Event.h
    Handle.h
    Input.cpp
    Input.h
    KeyCodes.h
    Logger.cpp
    Logger.h
    PlatformDetection.h
    Time.cpp
    Time.h
    Window.cpp
    Window.h
    SDL/SDLInput.cpp
    SDL/SDLInput.h
    SDL/SDLWindow.cpp
    SDL/SDLWindow.h)

if (BUILD_QT_BACKEND)
    target_sources(${TARGET_NAME} PRIVATE
        Qt/QtInput.cpp
        Qt/QtInput.h
        Qt/QtKeyCodeMap.cpp
        Qt/QtKeyCodeMap.h
        Qt/QtViewportWidget.cpp
        Qt/QtViewportWidget.h
        Qt/QtWindow.cpp
        Qt/QtWindow.h)

    target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Widgets Qt6::OpenGLWidgets)
    target_compile_definitions(${TARGET_NAME} PRIVATE MT_ENABLE_QT_BACKEND)
endif()