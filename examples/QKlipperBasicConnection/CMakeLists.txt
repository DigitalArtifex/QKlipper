cmake_minimum_required(VERSION 3.19)
project(QKlipperCmake LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network Quick)
find_package(QKlipper 1.0.1 REQUIRED)

qt_standard_project_setup()

qt_add_executable(QKlipperCmake
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(QKlipperCmake
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Network
        Qt::Quick
        ${QKlipper_LIBRARIES}
)

include(GNUInstallDirs)

install(TARGETS QKlipperCmake
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET QKlipperCmake
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})
