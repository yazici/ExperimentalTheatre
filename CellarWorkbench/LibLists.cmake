# Qt
FIND_PACKAGE(Qt4 REQUIRED)
SET(QT_DONT_USE_QTGUI TRUE)
INCLUDE(${QT_USE_FILE})

# Global
SET(CELLAR_LIBRARIES
    ${QT_LIBRARIES})
SET(CELLAR_INCLUDE_DIR 
    ${CELLAR_INCLUDE_DIR})
