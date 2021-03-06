#ifndef PROPROOM3D_BACKDROP_H
#define PROPROOM3D_BACKDROP_H

#include <GLM/glm.hpp>

#include <PropRoom3D/Node/Node.h>


namespace prop3
{
    class Raycast;
    class LightCast;


    class PROP3D_EXPORT Backdrop : public Node
    {
    protected:
        Backdrop();

    public:
        virtual ~Backdrop();

        virtual double distance(const Raycast& ray) const = 0;

        virtual glm::dvec4 raycast(const Raycast& ray) const =0;

        virtual void fireOn(
                std::vector<LightCast>& lightCasts,
                const glm::dvec3& pos,
                unsigned int count) const = 0;
    };
}

#endif // PROPROOM3D_BACKDROP_H
