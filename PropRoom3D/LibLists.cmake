# Qt
FIND_PACKAGE(Qt4 REQUIRED)
SET(QT_DONT_USE_QTGUI TRUE)
INCLUDE(${QT_USE_FILE})


# Glew
SET(GLEW_LIBRARIES GLEW)
SET(GLEW_INCLUDE_PATH "")
IF(WIN32)
	FIND_LIBRARY(GLEW_LIBRARIES_TRY
		NAMES glew GLEW glew32 glew32mx
		PATHS $ENV{GLEW_ROOT}/bin $ENV{GLEW_ROOT}/lib)
	SET(GLEW_LIBRARIES ${GLEW_LIBRARIES_TRY})
	SET(GLEW_INCLUDE_PATH $ENV{GLEW_ROOT}/include)
ENDIF()

MESSAGE(STATUS "GLEW bin: ${GLEW_LIBRARIES}")
MESSAGE(STATUS "GLEW inc: ${GLEW_INCLUDE_PATH}")


# Global
SET(PROP3_LIBRARIES
    CellarWorkbench
	${QT_LIBRARIES}
    ${GLEW_LIBRARIES})
SET(PROP3_INCLUDE_DIR 
	${PROP3_INCLUDE_DIR} 
	${GLEW_INCLUDE_PATH})
