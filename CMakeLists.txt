cmake_minimum_required(VERSION 3.16)

project(ProyectoEstru2MariaRodriguez VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========== Qt 5/6 con Widgets + Multimedia + MultimediaWidgets ==========
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

# ========== Fuentes ==========
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    lobby.h lobby.cpp
    personaje.h personaje.cpp
    roulettewidget.h roulettewidget.cpp
    locksprite.h locksprite.cpp

    minigamearte.h minigamearte.cpp
    minijuegohistoria.h minijuegohistoria.cpp
    minijuegociencia.h minijuegociencia.cpp
    minijuegopolitica.h minijuegopolitica.cpp

    HeartBar.h
    Maze.h
    MazeLoader.h
    mazeloader.h mazeloader.cpp

    combatenivel3.h combatenivel3.cpp
    nivel3combate.h nivel3combate.cpp
    nivel1.h nivel1.cpp
    mapa.h mapa.cpp
    escuela.h escuela.cpp
    aula.h aula.cpp
    laboratorio.h laboratorio.cpp
)

# ========== Target ==========
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ProyectoEstru2MariaRodriguez
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ranking.h ranking.cpp
        gamesession.h gamesession.cpp
        rankingstore.h rankingstore.cpp
    )
else()
    if(ANDROID)
        add_library(ProyectoEstru2MariaRodriguez SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(ProyectoEstru2MariaRodriguez
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# ========== Enlaces (¡al target correcto!) ==========
target_link_libraries(ProyectoEstru2MariaRodriguez PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
)

# ========== Propiedades de bundle (opcionales) ==========
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ProyectoEstru2MariaRodriguez)
endif()
set_target_properties(ProyectoEstru2MariaRodriguez PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ProyectoEstru2MariaRodriguez
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ProyectoEstru2MariaRodriguez)
endif()
