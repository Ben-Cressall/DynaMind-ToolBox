cmake_minimum_required(VERSION 2.8)
project(DynaMind-ToolBox)

IF(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	message(STATUS "Using Visual C compilers")
	set(CMAKE_CXX_FLAGS "/EHsc /bigobj")
	ADD_DEFINITIONS(-D_VARIADIC_MAX=9 -DCGAL_CFG_NO_CPP0X_ISFINITE)
ENDIF()

OPTION(DYNAMIND_UPDATE "update before compile" OFF)
OPTION(WITH_INSTALLER "create a windows installer (NSIS)" OFF)
OPTION(WITH_INSTALLER_GDAL "create a windows installer (NSIS)" OFF)
OPTION(WITH_UNIT_TESTS "build unit tests" ON)
OPTION(WITH_PLUGIN_GDALMODULE "if with GDALMODULES plugin" ON)
OPTION(WITH_PLUGIN_GDALDRAINAGE  "if with GDALDRAINAGE plugin" ON)
OPTION(WITH_STANDALONE_EXE "if with standalon executeable" ON)
OPTION(WITH_GUI "if with GUI" OFF)
OPTION(WITH_EXTENSIONS "if with Extensions plugin" ON)
OPTION(WITH_PLUGIN_PERFORMANCE_ASSESSMENT "if with performance assessment plugin" ON)
OPTION(WITH_DOC "Create Docu" OFF)
OPTION(PYTHON_EMBEDDING "Embedding Python" ON)
OPTION(USEQT5 "Use Qt5" ON)
OPTION(QGL_FROM_SOURCE "Build QGLViewer from source" OFF)


set(QT_MAJOR_VERSION 4)
if(USEQT5)
    set(QT_MAJOR_VERSION 5)
endif()

 #IF(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        	SET (CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
 #ENDIF()

find_package(Git REQUIRED)

set(TOOLBOX DynaMind)

if(WITH_PLUGIN_BASICMODULES)
    set(TOOLBOX ${TOOLBOX} DynaMind-BasicModules)
endif()

if(WITH_GUI)
    set(TOOLBOX ${TOOLBOX} DynaMind-Gui)
endif(WITH_GUI)

if(WITH_EXTENSIONS)
    set(TOOLBOX ${TOOLBOX} DynaMind-Extensions)
endif(WITH_EXTENSIONS)

if(WITH_PLUGIN_DYNAVIBE)
    set(TOOLBOX ${TOOLBOX} DynaVIBe)
endif()

if(WITH_PLUGIN_DYNALP)
    set(TOOLBOX ${TOOLBOX} DynaMind-DynAlp)
endif()

if(WITH_PLUGIN_GDALMODULE)
    set(TOOLBOX ${TOOLBOX} DynaMind-GDALModules)
endif()

if(WITH_PLUGIN_PERFORMANCE_ASSESSMENT)
    set(TOOLBOX ${TOOLBOX} DynaMind-Performance-Assessment)
endif()

if(WITH_STANDALONE_EXE)
    set(TOOLBOX ${TOOLBOX} DynaMind-Executable)
endif()

file(MAKE_DIRECTORY ${DynaMind-ToolBox_BINARY_DIR}/output)

foreach (tool ${TOOLBOX})
    if(EXISTS ${CMAKE_SOURCE_DIR}/${tool}/src)
        message(STATUS "${tool} submodule already exists")
    else (EXISTS ${CMAKE_SOURCE_DIR}/${tool}/src)
        message(STATUS "preparing ${tool} submodule for you")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive ${tool}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    endif (EXISTS ${CMAKE_SOURCE_DIR}/${tool}/src)

    if(DYNAMIND_UPDATE)
            execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --recursive ${tool}
                            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    endif(DYNAMIND_UPDATE)
endforeach(tool ${TOOLBOX}) 

SET(DYNAMIND_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/DynaMind/src/core) 
SET(DYNAMIND_INCLUDE_DIR ${DYNAMIND_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/DynaMind/src/DMToolboxes) 
SET(DYNAMIND_INCLUDE_DIR ${DYNAMIND_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/DynaMind/src/python-swig) 
SET(DYNAMIND_EXTENSIONS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/DynaMind-Extensions/src) 
SET(DYNAMIND_TOOLBOX_BUILD_DIR ${DynaMind-ToolBox_BINARY_DIR}/output)


foreach (tool ${TOOLBOX})
    add_subdirectory(${tool})
endforeach(${tool}) 

add_subdirectory(Data)

if(WITH_DOC)
    add_subdirectory(./doc/)
endif()

if(NOT USEQT5 AND WITH_INSTALLER)
    INCLUDE(DynaMind-ToolBoxCPack_with_Python.txt)
endif()
if(NOT USEQT5 AND WITH_INSTALLER_GDAL)
	INCLUDE(DynaMind-ToolBoxCPack_with_Python_sfcgal.txt)
endif()
