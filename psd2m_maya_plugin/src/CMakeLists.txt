SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(ZLIBSTATIC_NAME zlibstatic)

# -------------------------------------
# Maya 
# -------------------------------------
FIND_PACKAGE(MAYA REQUIRED)

# -------------------------------------
# QT setup
# -------------------------------------
set(QT_DIR ${MAYA_LOCATION}/bin)
set(QT_CONF_PATH ${QT_DIR}/qt.conf)
set(QT_QMAKE_EXECUTABLE ${QT_DIR}/qmake.exe)
set(QT_MOC_EXECUTABLE ${QT_DIR}/moc)
set(QT_INCLUDE_DIR ${MAYA_INCLUDE_DIR})

if(${MAYA_VERSION} STREQUAL 2016)
	FIND_PACKAGE(Qt4 REQUIRED)
	
	INCLUDE(${QT_USE_FILE})
else()
	set(Qt5_DIR ${MAYA_LOCATION}/lib/cmake/qt5)
	FIND_PACKAGE(Qt5 COMPONENTS Core Widgets REQUIRED)
endif()
include_directories(${QT_INCLUDE_DIR})

# -------------------------------------
# Content declaration
# -------------------------------------
SET(GENERATOR_MAYA_SOURCE_FILES
	"maya_mesh/mayaMeshConvertor.cpp"
	"maya_mesh/meshGeneratorController.cpp"
	"maya_mesh/editorComponentGenerator.cpp")

SET(GENERATOR_MAYA_HEADER_FILES
	"maya_mesh/mayaMeshConvertor.h"
	"maya_mesh/meshGeneratorController.h"
	"maya_mesh/editorComponentGenerator.h")

SET(TEXTURE_EXPORTER_SOURCE_FILES
	"texture_exporter/lodepng.cpp"
	"texture_exporter/textureExporter.cpp")

SET(TEXTURE_EXPORTER_HEADER_FILES
	"texture_exporter/lodepng.h"
	"texture_exporter/textureExporter.h")

SET(QT_UI_SOURCE_FILES
	"interface/toolWidget.cpp")

SET(QT_UI_HEADER_FILES
	"interface/toolWidget.h")

SET(QT_SOURCE_FILES
	"pluginController.cpp"
	"mainWindowCmd.cpp"
	"parameters.cpp"
	"qtProgress.cpp")

SET(QT_HEADER_FILES
	"IControllerUpdate.h"
	"mayaUtils.h"
	"pluginController.h"
	"mainWindowCmd.h"
	"parameters.h"
	"qtProgress.h")

SET(UI_FILES
	"interface/toolWidget.ui")

SET(JSON_SOURCE_FILES
	"json/JSON.cpp"
	"json/JSONValue.cpp")

SET(JSON_HEADER_FILES
	"json/JSON.h"
	"json/JSONValue.h")
	
if(${MAYA_VERSION} STREQUAL 2016)
	QT4_WRAP_CPP(HEADERS_MOC ${QT_HEADER_FILES} ${QT_UI_HEADER_FILES})
	QT4_WRAP_UI(ToolWidget_UI ${UI_FILES})
else()
	QT5_WRAP_CPP(HEADERS_MOC ${QT_HEADER_FILES} ${QT_UI_HEADER_FILES})
	QT5_WRAP_UI(ToolWidget_UI ${UI_FILES})
endif()

SET(TOOL_UI ${ToolWidget_UI})

# -------------------------------------
# new in cmake 3.10 set the generated file to ignore automoc, in 3.9 file generated are automaticaly ignored.
# -------------------------------------
SET_PROPERTY(SOURCE ui_ToolWidget.h PROPERTY SKIP_AUTOMOC ON)

ADD_DEFINITIONS(${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${MAYA_INCLUDE_DIR} ../../${TARGET_NAME_PSD_READER}/src ../../${TARGET_NAME_MESH_GENERATOR}/src ../../${TARGET_NAME_UTIL}/src)
LINK_DIRECTORIES(${MAYA_LIBRARY_DIR} ../../${TARGET_NAME_PSD_READER}/lib)
ADD_LIBRARY(${TARGET_NAME_PLUGIN} SHARED
	${TEXTURE_EXPORTER_SOURCE_FILES}
	${GENERATOR_MAYA_SOURCE_FILES}
	${QT_SOURCE_FILES}
	${QT_UI_SOURCE_FILES}
	${TEXTURE_EXPORTER_HEADER_FILES}
	${GENERATOR_MAYA_HEADER_FILES}
	${QT_HEADER_FILES}
	${QT_UI_HEADER_FILES}
	${JSON_SOURCE_FILES}
	${JSON_HEADER_FILES}
	${TOOL_UI}
	)

if(${MAYA_VERSION} STREQUAL 2016)
	TARGET_LINK_LIBRARIES(${TARGET_NAME_PLUGIN} 
	${MAYA_LIBRARIES} 
	${QT_LIBRARIES} 
	${TARGET_NAME_PSD_READER} 
	${TARGET_NAME_MESH_GENERATOR} 
	${TARGET_NAME_UTIL} 
	${ZLIBSTATIC_NAME}
	)
else()
	TARGET_LINK_LIBRARIES(${TARGET_NAME_PLUGIN} 
	${MAYA_LIBRARIES} 
	${TARGET_NAME_PSD_READER} 
	${TARGET_NAME_MESH_GENERATOR} 
	${TARGET_NAME_UTIL} 
	${ZLIBSTATIC_NAME}
	)
	QT5_USE_MODULES(${TARGET_NAME_PLUGIN} Widgets)
endif()

# -------------------------------------
# Group for Visual Studio
# -------------------------------------
SOURCE_GROUP("Header Files\\mesh_maya_generator" FILES  ${GENERATOR_MAYA_HEADER_FILES})
SOURCE_GROUP("Source Files\\mesh_maya_generator" FILES  ${GENERATOR_MAYA_SOURCE_FILES})
SOURCE_GROUP("Header Files\\texture_exporter" FILES  ${TEXTURE_EXPORTER_HEADER_FILES})
SOURCE_GROUP("Source Files\\texture_exporter" FILES  ${TEXTURE_EXPORTER_SOURCE_FILES})
SOURCE_GROUP("Header Files\\interface" FILES  ${QT_UI_HEADER_FILES})
SOURCE_GROUP("Source Files\\interface" FILES  ${QT_UI_SOURCE_FILES})
SOURCE_GROUP("Header Files\\json" FILES  ${JSON_HEADER_FILES})
SOURCE_GROUP("Source Files\\json" FILES  ${JSON_SOURCE_FILES})
SOURCE_GROUP("Forms" FILES  ${UI_FILES})

# -------------------------------------
# Maya project properties
# -------------------------------------
MAYA_PLUGIN(${TARGET_NAME_PLUGIN} ${PROJECT_VERSION_PLUGIN})