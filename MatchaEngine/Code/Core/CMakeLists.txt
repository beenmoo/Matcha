cmake_minimum_required(VERSION 3.15)

target_sources(${PROJECT_NAME} PRIVATE
    Application.cpp
    Application.h
    Assert.h
    Context.cpp
    Context.h
    Core.h
    EntryPoint.h
    Input.cpp
    Input.h
    KeyCodes.h
    Logger.cpp
    Logger.h
    PlatformDetection.h
    Time.cpp
    Time.h
    Window.cpp
    Window.h)