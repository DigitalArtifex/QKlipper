cmake_minimum_required(VERSION 3.16)

#set(PROJECT-NAME_VERSION)
project(QKlipper LANGUAGES CXX VERSION 1.0.2)

option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_STATIC_LIBS "Build static library" ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(USE_SYSTEM_PATH)
    set(INSTALL_PREFIX "/usr")
else()
    set(INSTALL_PREFIX ${CMAKE_PREFIX_PATH})
endif()

set(INSTALL_LIB_PATH "${INSTALL_PREFIX}/lib")
set(INSTALL_INCLUDE_PATH "${INSTALL_PREFIX}/include")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Network Quick)

add_custom_target(Configs SOURCES
    cmake/QKlipper.pc.in
    cmake/QKlipper.cmake.in
)

set(INSTALLED_NAME Qt${QT_VERSION_MAJOR}Klipper)

qt_add_resources(RESOURCES resources.qrc)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/QKlipper/*.h)

if(BUILD_SHARED_LIBS)
    add_library(QKlipper SHARED)

    target_link_libraries(QKlipper PRIVATE
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Quick
    )

    target_compile_definitions(QKlipper PUBLIC QKLIPPER_LIBRARY)

    target_include_directories(QKlipper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    target_sources(QKlipper PRIVATE ${SOURCES})
    target_sources(QKlipper PRIVATE ${RESOURCES})
    target_sources(QKlipper PRIVATE ${HEADERS})

    set_target_properties(QKlipper PROPERTIES
        VERSION ${CMAKE_PROJECT_VERSION}
        SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
        LIBRARY_OUTPUT_NAME ${INSTALLED_NAME}
    )

    install(TARGETS QKlipper DESTINATION "${INSTALL_LIB_PATH}")
endif()

if(BUILD_STATIC_LIBS)
    add_library(QKlipperStatic STATIC)

    target_link_libraries(QKlipperStatic PRIVATE
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Quick
    )

    target_compile_definitions(QKlipperStatic PUBLIC QKLIPPER_LIBRARY)

    target_include_directories(QKlipperStatic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    target_sources(QKlipperStatic PRIVATE ${SOURCES})
    target_sources(QKlipperStatic PRIVATE ${RESOURCES})
    target_sources(QKlipperStatic PRIVATE ${HEADERS})

    set_target_properties(QKlipperStatic PROPERTIES
        VERSION ${CMAKE_PROJECT_VERSION}
        SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
        OUTPUT_NAME ${INSTALLED_NAME}
    )

    install(TARGETS QKlipperStatic DESTINATION "${INSTALL_LIB_PATH}")
endif()

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/QKlipperConfigVersion.cmake
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/QKlipperConfigVersion.cmake
    DESTINATION "${INSTALL_LIB_PATH}/cmake/QKlipper"
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/QKlipper.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/QKlipperConfig.cmake"
    INSTALL_DESTINATION "${INSTALL_LIB_PATH}/cmake/QKlipper"
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/QKlipper.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/QKlipper.pc"
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/QKlipper.pc
    DESTINATION "${INSTALL_LIB_PATH}/pkgconfig"
)

install(
    TARGETS QKlipper
    EXPORT QKlipperTargets
    FILE_SET HEADERS
    DESTINATION "${INSTALL_INCLUDE_PATH}/QKlipper/"
)

install(
    EXPORT QKlipperTargets
    NAMESPACE QKlipper::
    DESTINATION "${INSTALL_LIB_PATH}/cmake/QKlipper"
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/QKlipper/ DESTINATION ${INSTALL_INCLUDE_PATH}/QKlipper)

#Config messages
message(STATUS "${INSTALLED_NAME}")
message(STATUS "Version: ${CMAKE_PROJECT_VERSION}")
message(STATUS "Qt Version: ${QT_VERSION}")

if(BUILD_SHARED_LIBS)
    message(STATUS "Building shared libs: Yes")
else()
    message(STATUS "Building shared libs: No")
endif()

if(BUILD_STATIC_LIBS)
    message(STATUS "Building static libs: Yes")
else()
    message(STATUS "Building static libs: No")
endif()

message(STATUS "Install Prefix: ${INSTALL_PREFIX}")
