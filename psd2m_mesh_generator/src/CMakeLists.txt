SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

set(GENERATOR_SOURCE_FILES
	"mesh_generator/dataMesh.cpp"
	)

set(GENERATOR_HEADER_FILES
	"mesh_generator/dataMesh.h"
	)
	
set(LINEAR_SOURCE_FILES
	"mesh_generator/linear_mesh/bezierCurve.cpp"
	"mesh_generator/linear_mesh/boundingBox.cpp"
	"mesh_generator/linear_mesh/linearMesh.cpp"
	)
	
set(LINEAR_HEADER_FILES
	"mesh_generator/linear_mesh/bezierCurve.h"
	"mesh_generator/linear_mesh/boundingBox.h"
	"mesh_generator/linear_mesh/linearMesh.h"
	)
	
set(CURVE_SOURCE_FILES
	"mesh_generator/curve_mesh/bezier.cpp"
	"mesh_generator/curve_mesh/curve.cpp"
	"mesh_generator/curve_mesh/curveNode.cpp"
	"mesh_generator/curve_mesh/curveData.cpp"
	"mesh_generator/curve_mesh/curveMeshGenerator.cpp"
	)
	
set(CURVE_HEADER_FILES
	"mesh_generator/curve_mesh/bezier.h"
	"mesh_generator/curve_mesh/curve.h"
	"mesh_generator/curve_mesh/curveNode.h"
	"mesh_generator/curve_mesh/curveData.h"
	"mesh_generator/curve_mesh/curveMeshGenerator.h"
	)
	
set(INFLUENCE_SOURCE_FILES
	"mesh_generator/influence_mesh/influenceMesh.cpp"
	"mesh_generator/influence_mesh/meshFace.cpp"
	)
	
set(INFLUENCE_HEADER_FILES
	"mesh_generator/influence_mesh/influenceMesh.h"
	"mesh_generator/influence_mesh/meshFace.h"
	)
	
INCLUDE_DIRECTORIES(../../${TARGET_NAME_UTIL}/src)



ADD_LIBRARY(${TARGET_NAME_MESH_GENERATOR} STATIC
	${GENERATOR_SOURCE_FILES}
	${GENERATOR_HEADER_FILES}
	${LINEAR_SOURCE_FILES}
	${LINEAR_HEADER_FILES}
	${CURVE_SOURCE_FILES}
	${CURVE_HEADER_FILES}
	${INFLUENCE_SOURCE_FILES}
	${INFLUENCE_HEADER_FILES}
	)

TARGET_LINK_LIBRARIES(${TARGET_NAME_MESH_GENERATOR} ${TARGET_NAME_UTIL})
	
SOURCE_GROUP("Header Files\\mesh_generator" FILES  ${GENERATOR_HEADER_FILES})
SOURCE_GROUP("Source Files\\mesh_generator" FILES  ${GENERATOR_SOURCE_FILES})
SOURCE_GROUP("Header Files\\mesh_generator\\linear_mesh" FILES  ${LINEAR_HEADER_FILES})
SOURCE_GROUP("Source Files\\mesh_generator\\linear_mesh" FILES  ${LINEAR_SOURCE_FILES})
SOURCE_GROUP("Header Files\\mesh_generator\\curve_mesh" FILES  ${CURVE_HEADER_FILES})
SOURCE_GROUP("Source Files\\mesh_generator\\curve_mesh" FILES  ${CURVE_SOURCE_FILES})
SOURCE_GROUP("Header Files\\mesh_generator\\influence_mesh" FILES  ${INFLUENCE_HEADER_FILES})
SOURCE_GROUP("Source Files\\mesh_generator\\influence_mesh" FILES  ${INFLUENCE_SOURCE_FILES})

SET_TARGET_PROPERTIES(${TARGET_NAME_MESH_GENERATOR} PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY  "../../build/lib"
		ARCHIVE_OUTPUT_NAME ${LIBRARY_NAME_MESH_GENERATOR}
		COMPILE_PDB_OUTPUT_DIRECTORY "../../build/lib"
		COMPILE_PDB_NAME ${LIBRARY_NAME_MESH_GENERATOR}
        PREFIX ""
		SUFFIX ".lib"
		)