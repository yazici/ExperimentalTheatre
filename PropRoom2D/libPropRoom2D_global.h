#ifndef PROPROOM2D_LIBPROPROOM2D_GLOBAL_H
#define PROPROOM2D_LIBPROPROOM2D_GLOBAL_H

#include <QtCore/qglobal.h>
#include <DataStructure/Vector.h>
#include <DataStructure/Matrix.h>
#include <Geometry/Segment2D.h>

#if defined(PROPROOM2D_LIBRARY)
#  define PROP2D_EXPORT Q_DECL_EXPORT
#else
#  define PROP2D_EXPORT Q_DECL_IMPORT
#endif


namespace prop2
{
    // Units precision
    typedef float real;


    // Shapes attributes
    typedef unsigned int PropId;
    namespace PropType
    {
    enum Enum {CIRCLE,
               POLYGON,
               TEXT_HUD,
               IMAGE_HUD,
               NB_TYPES};
    }

    namespace BodyType
    {
    enum Enum {STATIC,
               CINEMATIC,
               DYNAMIC,
               NB_TYPES};
    }

    namespace HorizontalAnchor
    {
    enum Enum {LEFT,
               RIGHT,
               NB_ANCHORS};
    }

    namespace VerticalAnchor
    {
    enum Enum {BOTTOM,
               TOP,
               NB_ANCHORS};
    }


    // Vectors and matrices
    typedef cellar::Vector<2, real> Vec2r;
    typedef cellar::Vector<3, real> Vec3r;
    typedef cellar::Vector<4, real> Vec4r;
    typedef cellar::Matrix3x3<real> Mat3r;
    typedef cellar::Matrix4x4<real> Mat4r;

    // Line
    typedef cellar::Segment2D<real> Segment2Dr;
}

#endif // PROPROOM2D_LIBPROPROOM2D_GLOBAL_H
