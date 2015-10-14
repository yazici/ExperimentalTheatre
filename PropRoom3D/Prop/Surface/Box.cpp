#include "Box.h"

#include <GLM/gtx/transform.hpp>

#include "../Coating/Coating.h"
#include "Ray/RayHitList.h"
#include "Ray/RayHitReport.h"
#include "../../StageSet/StageSetVisitor.h"


namespace prop3
{
    Box::Box(const glm::dvec3& minCorner, const glm::dvec3& maxCorner) :
        _center((minCorner + maxCorner) / 2.0),
        _dimensions(maxCorner - minCorner),
        _minCorner(minCorner),
        _maxCorner(maxCorner)
    {

    }

    std::shared_ptr<Surface> Box::boxCorners(
            const glm::dvec3& minCorner,
            const glm::dvec3& maxCorner)
    {
        return std::shared_ptr<Surface>(new Box(minCorner, maxCorner));
    }

    std::shared_ptr<Surface> Box::boxPosDims(
            const glm::dvec3& center,
            const glm::dvec3& dimensions)
    {
        glm::dvec3 minCorner = center - dimensions / 2.0;
        glm::dvec3 maxCorner = center + dimensions / 2.0;
        return std::shared_ptr<Surface>(new Box(minCorner, maxCorner));
    }

    void Box::transform(const Transform& transform)
    {
        _minCorner = glm::dvec3(transform.mat() * glm::dvec4(_minCorner, 1.0));
        _maxCorner = glm::dvec3(transform.mat() * glm::dvec4(_maxCorner, 1.0));
        _center = (_minCorner + _maxCorner) / 2.0;
        _dimensions = _maxCorner - _minCorner;

        stampCurrentUpdate();
    }

    EPointPosition Box::isIn(const glm::dvec3& point) const
    {
        if(glm::all(glm::lessThanEqual(_minCorner,  point)) &&
           glm::all(glm::lessThanEqual(point, _maxCorner)))
        {
            if(glm::any(glm::equal(_minCorner,  point)) ||
               glm::any(glm::equal(point, _maxCorner)))
            {
                return EPointPosition::ON;
            }
            else
            {
                return EPointPosition::IN;
            }
        }
        else
        {
            return EPointPosition::OUT;
        }
    }

    double Box::signedDistance(const glm::dvec3& point) const
    {
        assert(false);
    }

    void Box::raycast(const Raycast& ray, RayHitList& reports) const
    {
        glm::dvec3 t1 = (_minCorner - ray.origin) / ray.direction;
        glm::dvec3 t2 = (_maxCorner - ray.origin) / ray.direction;

        glm::dvec3 vtmin = glm::min(t1, t2);
        glm::dvec3 vtmax = glm::max(t1, t2);

        double tmin = glm::max(glm::max(vtmin.x, vtmin.y), vtmin.z);
        double tmax = glm::min(glm::min(vtmax.x, vtmax.y), vtmax.z);

        if(tmax > glm::max(tmin, 0.0))
        {
            if(0.0 < tmin && tmin < ray.limit)
            {
                glm::dvec3 pt = ray.origin + ray.direction * tmin;
                glm::dvec3 n = (pt - _center) / _dimensions;
                glm::dvec3 absN = glm::abs(n);
                if(absN.x > absN.y)
                    if(absN.x > absN.z)
                        n = glm::dvec3(n.x, 0, 0);
                    else
                        n = glm::dvec3(0, 0, n.z);
                else
                    if(absN.y > absN.z)
                        n = glm::dvec3(0, n.y, 0);
                    else
                        n = glm::dvec3(0, 0, n.z);
                n = glm::normalize(n);

                reports.add(tmin, ray, pt, n,
                            RayHitReport::NO_TEXCOORD,
                            _coating.get());
            }

            if(0.0 < tmax && tmax < ray.limit)
            {
                glm::dvec3 pt = ray.origin + ray.direction * tmax;
                glm::dvec3 n = (pt - _center) / _dimensions;
                glm::dvec3 absN = glm::abs(n);
                if(absN.x > absN.y)
                    if(absN.x > absN.z)
                        n = glm::dvec3(n.x, 0, 0);
                    else
                        n = glm::dvec3(0, 0, n.z);
                else
                    if(absN.y > absN.z)
                        n = glm::dvec3(0, n.y, 0);
                    else
                        n = glm::dvec3(0, 0, n.z);
                n = glm::normalize(n);

                reports.add(tmax, ray, pt, n,
                            RayHitReport::NO_TEXCOORD,
                            _coating.get());
            }
        }
    }

    bool Box::intersects(const Raycast& ray, RayHitList& reports) const
    {
        glm::dvec3 t1 = (_minCorner - ray.origin) / ray.direction;
        glm::dvec3 t2 = (_maxCorner - ray.origin) / ray.direction;

        glm::dvec3 vtmin = glm::min(t1, t2);
        glm::dvec3 vtmax = glm::max(t1, t2);

        double tmin = glm::max(glm::max(vtmin.x, vtmin.y), vtmin.z);
        double tmax = glm::min(glm::min(vtmax.x, vtmax.y), vtmax.z);

        return tmax > glm::max(tmin, 0.0);
    }

    void Box::accept(StageSetVisitor& visitor)
    {
        visitor.visit(*this);
    }


    // Box Texture
    BoxTexture::BoxTexture(
            const glm::dvec3& minCorner,
            const glm::dvec3& maxCorner,
            const glm::dvec3& texOrigin,
            const glm::dvec3& texU,
            const glm::dvec3& texV,
            bool texMainSideOnly) :
        Box(minCorner, maxCorner),
        _texOrigin(texOrigin),
        _texU(texU),
        _texV(texV),
        _texMainSideOnly(texMainSideOnly)
    {

    }

    std::shared_ptr<Surface> BoxTexture::boxCorners(
            const glm::dvec3& minCorner,
            const glm::dvec3& maxCorner,
            const glm::dvec3& texOrigin,
            const glm::dvec3& texU,
            const glm::dvec3& texV,
            bool texMainSideOnly)
    {
        return std::shared_ptr<Surface>(
            new BoxTexture(
                minCorner,
                maxCorner,
                texOrigin,
                texU,
                texV,
                texMainSideOnly));
    }

    std::shared_ptr<Surface> BoxTexture::boxPosDims(
            const glm::dvec3& center,
            const glm::dvec3& dimensions,
            const glm::dvec3& texOrigin,
            const glm::dvec3& texU,
            const glm::dvec3& texV,
            bool texMainSideOnly)
    {
        glm::dvec3 minCorner = center - dimensions / 2.0;
        glm::dvec3 maxCorner = center + dimensions / 2.0;
        return std::shared_ptr<Surface>(
            new BoxTexture(
                minCorner,
                maxCorner,
                texOrigin,
                texU,
                texV,
                texMainSideOnly));
    }

    void BoxTexture::accept(StageSetVisitor& visitor)
    {
        visitor.visit(*this);
    }

    void BoxTexture::transform(const Transform& transform)
    {
        _texOrigin = glm::dvec3(transform.mat() * glm::dvec4(_texOrigin, 1.0));
        _texU = glm::dvec3(transform.mat() * glm::dvec4(_texU, 0.0));
        _texV = glm::dvec3(transform.mat() * glm::dvec4(_texV, 0.0));

        Box::transform(transform);
    }

    void BoxTexture::raycast(const Raycast& ray, RayHitList& reports) const
    {
        RayHitReport* last = reports.head;
        Box::raycast(ray, reports);
        RayHitReport* first = reports.head;

        while(first != last)
        {
            RayHitReport& r = *first;

            if(!_texMainSideOnly ||
               glm::dot(r.normal, glm::cross(_texU, _texV)) >
                    RayHitReport::EPSILON_LENGTH)
            {
                glm::dvec3 dist = r.position - _texOrigin;
                r.texCoord.s = glm::dot(dist, _texU) / glm::dot(_texU, _texU);
                r.texCoord.t = glm::dot(dist, _texV) / glm::dot(_texV, _texV);
            }

            first = first->_next;
        }
    }
}
