# Qt
FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui REQUIRED)
SET(CMAKE_AUTOMOC ON)


# Global
SET(PROP2_LIBRARIES
    ${CELLAR_PROJECT}
    ${MEDIA_PROJECT})
SET(PROP2_INCLUDE_DIR
    ${CELLAR_SRC_DIR}
    ${MEDIA_SRC_DIR}
    ${PROP2_SRC_DIR}
    ${GL3_SRC_DIR})
SET(PROP2_QT_MODULES
    Core
    Gui)
