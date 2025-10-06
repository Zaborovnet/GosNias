cmake_minimum_required(VERSION 3.16)

project(GosNias LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(GosNias main.cpp
    MessageStructures.h
    RestMessageSender.h RestMessageSender.cpp )

include(GNUInstallDirs)
install(TARGETS GosNias
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
