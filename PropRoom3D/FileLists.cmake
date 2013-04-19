## Headers ##
# All the header files #
SET(PROP3_HEADERS
    ${PROP3_SRC_DIR}/AbstractProp.h
    ${PROP3_SRC_DIR}/libPropRoom3D_global.h)


## Sources ##
# All the source files #
SET(PROP3_SOURCES
    ${PROP3_SRC_DIR}/AbstractProp.cpp)


## Global ##
SET(PROP3_CONFIG_FILES
    ${PROP3_SRC_DIR}/CMakeLists.txt
    ${PROP3_SRC_DIR}/FileLists.cmake
    ${PROP3_SRC_DIR}/LibLists.cmake)
	
SET(PROP3_SRC_FILES
    ${PROP3_HEADERS}
    ${PROP3_SOURCES}
	${PROP3_CONFIG_FILES})

	
SET(PROP3_INCLUDE_DIR
    ${PROP3_SRC_DIR})
SET(PROP3_INCLUDE_DIR
    ${PROP3_INCLUDE_DIR}
    PARENT_SCOPE)


## Source groups ##
SOURCE_GROUP("Header Files" FILES ${PROP3_HEADERS_FILTER})

SOURCE_GROUP("Source Files" FILES ${PROP3_SOURCES_FILTER})

SOURCE_GROUP("Config" FILES ${PROP3_CONFIG_FILES})