SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PSD_SOURCE_FILES
	"psd_reader/colorModeReader.cpp"
	"psd_reader/imageDataReader.cpp"
	"psd_reader/headerReader.cpp"
	"psd_reader/imageResourceReader.cpp"
	"psd_reader/layerAndMaskReader.cpp"
	"psd_reader/psdReader.cpp"
	)

set(PSD_HEADER_FILES
	"psd_reader/colorModeReader.h"
	"psd_reader/imageDataReader.h"
	"psd_reader/headerReader.h"
	"psd_reader/imageResourceReader.h"
	"psd_reader/layerAndMaskReader.h"
	"psd_reader/psdReader.h"
	"psd_reader/progress.h"
	)

set(ZLIB_LIBRARY_DIRECTORY ../lib)
set(ZLIB_INCLUDE ../include)
set(ZLIB_LIB zlibstatic)

ADD_LIBRARY(${TARGET_NAME_PSD_READER} STATIC
	${PSD_SOURCE_FILES}
	${PSD_HEADER_FILES}	
	)

INCLUDE_DIRECTORIES(../../${TARGET_NAME_UTIL}/src ${ZLIB_INCLUDE})
LINK_DIRECTORIES(${ZLIB_LIBRARY_DIRECTORY})

TARGET_LINK_LIBRARIES(${TARGET_NAME_PSD_READER} ${TARGET_NAME_UTIL} ${ZLIB_LIB})

SOURCE_GROUP("Header Files\\psd_reader" FILES  ${PSD_HEADER_FILES})
SOURCE_GROUP("Source Files\\psd_reader" FILES  ${PSD_SOURCE_FILES})

set_target_properties(${TARGET_NAME_PSD_READER} PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY  "../../build/lib"
		ARCHIVE_OUTPUT_NAME ${LIBRARY_NAME_PSD_READER}
		COMPILE_PDB_OUTPUT_DIRECTORY "../../build/lib"
		COMPILE_PDB_NAME ${LIBRARY_NAME_PSD_READER}
        PREFIX ""
		SUFFIX ".lib"
		)