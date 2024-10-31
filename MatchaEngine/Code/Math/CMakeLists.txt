cmake_minimum_required(VERSION 3.15)

target_sources(${PROJECT_NAME} PRIVATE
    Matrix.h
    Quaternion.h
    Transform.cpp
    Transform.h
    Vector.h)