#include "Box.h"

#include <GLM/gtx/transform.hpp>

#include "../Coating/Coating.h"
#include "Ray/RayHitList.h"
#include "Ray/RayHitReport.h"
#include "Node/Visitor.h"


namespace prop3
{
    Box::Box(const glm::dvec3& minCorner, const glm::dvec3& maxCorner) :
        _center((minCorner + maxCorner) / 2.0),
        _minCorner(minCorner),
        _maxCorner(maxCorner),
        _dimensions(maxCorner - minCorner),
        _invDim(1.0 / _dimensions)
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

    void Box::accept(Visitor& visitor)
    {
        visitor.visit(*this);
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
		return 0.0;
    }

    void Box::raycast(const Raycast& ray, RayHitList& reports) const
    {
        glm::dvec3 t1 = (_minCorner - ray.origin) * ray.invDir;
        glm::dvec3 t2 = (_maxCorner - ray.origin) * ray.invDir;

        glm::dvec3 vtmin = glm::min(t1, t2);
        glm::dvec3 vtmax = glm::max(t1, t2);

        double tmin = glm::max(glm::max(vtmin.x, vtmin.y), vtmin.z);
        double tmax = glm::min(glm::min(vtmax.x, vtmax.y), vtmax.z);

        if(tmax > glm::max(tmin, 0.0))
        {
            if(0.0 < tmin && tmin < ray.limit)
            {
                glm::dvec3 pt = ray.origin + ray.direction * tmin;
                glm::dvec3 n = (pt - _center) * _invDim;
                glm::dvec3 absN = glm::abs(n);
                if(absN.x > absN.y)
                    if(absN.x > absN.z)
                        n = glm::dvec3(glm::sign(n.x), 0, 0);
                    else
                        n = glm::dvec3(0, 0, glm::sign(n.z));
                else
                    if(absN.y > absN.z)
                        n = glm::dvec3(0, glm::sign(n.y), 0);
                    else
                        n = glm::dvec3(0, 0, glm::sign(n.z));

                reports.add(tmin, pt, n,
                            RayHitReport::NO_TEXCOORD,
                            _coating.get(),
                            _innerMat.get(),
                            _outerMat.get());
            }

            if(0.0 < tmax && tmax < ray.limit)
            {
                glm::dvec3 pt = ray.origin + ray.direction * tmax;
                glm::dvec3 n = (pt - _center) * _invDim;
                glm::dvec3 absN = glm::abs(n);
                if(absN.x > absN.y)
                    if(absN.x > absN.z)
                        n = glm::dvec3(glm::sign(n.x), 0, 0);
                    else
                        n = glm::dvec3(0, 0, glm::sign(n.z));
                else
                    if(absN.y > absN.z)
                        n = glm::dvec3(0, glm::sign(n.y), 0);
                    else
                        n = glm::dvec3(0, 0, glm::sign(n.z));

                reports.add(tmax, pt, n,
                            RayHitReport::NO_TEXCOORD,
                            _coating.get(),
                            _innerMat.get(),
                            _outerMat.get());
            }
        }
    }

    bool Box::intersects(const Raycast& ray, RayHitList& reports) const
    {
        glm::dvec3 minDiff = _minCorner - ray.origin;
        glm::dvec3 maxDiff = _maxCorner - ray.origin;
        if(minDiff.x <= 0.0 && minDiff.y <= 0.0 && minDiff.z <= 0.0 &&
           maxDiff.x >= 0.0 && maxDiff.y >= 0.0 && maxDiff.z >= 0.0)
            return true;

        glm::dvec3 t1 = minDiff * ray.invDir;
        glm::dvec3 t2 = maxDiff * ray.invDir;

        glm::dvec3 vtmin = glm::min(t1, t2);
        glm::dvec3 vtmax = glm::max(t1, t2);

        double tmin = glm::max(glm::max(vtmin.x, vtmin.y), vtmin.z);
        double tmax = glm::min(glm::min(vtmax.x, vtmax.y), vtmax.z);

        if(tmax > glm::max(tmin, 0.0))
        {
            if(0.0 < tmin && tmin < ray.limit)
                return true;

            if(0.0 < tmax && tmax < ray.limit)
                return true;
        }

        return false;
    }

    void Box::transform(const Transform& transform)
    {
        _minCorner = glm::dvec3(transform.mat() * glm::dvec4(_minCorner, 1.0));
        _maxCorner = glm::dvec3(transform.mat() * glm::dvec4(_maxCorner, 1.0));
        _center = (_minCorner + _maxCorner) / 2.0;
        _dimensions = _maxCorner - _minCorner;
        _invDim = 1.0 / _dimensions;

        stampCurrentUpdate();
    }


    // Box Texture
    BoxTexture::BoxTexture(
            const glm::dvec3& minCorner,
            const glm::dvec3& maxCorner,
            const glm::dvec3& texOrigin,
            const glm::dvec3& texU,
            const glm::dvec3& texV) :
        Box(minCorner, maxCorner),
        _texOrigin(texOrigin),
        _texU(texU),
        _texV(texV),
        _texU2(glm::dot(_texU, _texU)),
        _texV2(glm::dot(_texV, _texV))
    {
        _texW = glm::normalize(glm::cross(texU, texV)) * glm::length(_texU);
        _texW2 = glm::dot(_texW, _texW);
    }

    void BoxTexture::transform(const Transform& transform)
    {
        _texOrigin = glm::dvec3(transform.mat() * glm::dvec4(_texOrigin, 1.0));
        _texU = glm::dvec3(transform.mat() * glm::dvec4(_texU, 0.0));
        _texV = glm::dvec3(transform.mat() * glm::dvec4(_texV, 0.0));
        _texW = glm::dvec3(transform.mat() * glm::dvec4(_texW, 0.0));
        _texU2 = glm::dot(_texU, _texU);
        _texV2 = glm::dot(_texV, _texV);
        _texW2 = glm::dot(_texW, _texW);

        Box::transform(transform);
    }


    // Box Side Texture
    BoxSideTexture::BoxSideTexture(
            const glm::dvec3& minCorner,
            const glm::dvec3& maxCorner,
            const glm::dvec3& texOrigin,
            const glm::dvec3& texU,
            const glm::dvec3& texV) :
        BoxTexture(minCorner, maxCorner, texOrigin, texU, texV)
    {

    }

    std::shared_ptr<Surface> BoxSideTexture::boxCorners(
            const glm::dvec3& minCorner,
            const glm::dvec3& maxCorner,
            const glm::dvec3& texOrigin,
            const glm::dvec3& texU,
            const glm::dvec3& texV)
    {
        return std::shared_ptr<Surface>(
            new BoxSideTexture(
                minCorner,
                maxCorner,
                texOrigin,
                texU,
                texV));
    }

    std::shared_ptr<Surface> BoxSideTexture::boxPosDims(
            const glm::dvec3& center,
            const glm::dvec3& dimensions,
            const glm::dvec3& texOrigin,
            const glm::dvec3& texU,
            const glm::dvec3& texV)
    {
        glm::dvec3 minCorner = center - dimensions / 2.0;
        glm::dvec3 maxCorner = center + dimensions / 2.0;
        return std::shared_ptr<Surface>(
            new BoxSideTexture(
                minCorner,
                maxCorner,
                texOrigin,
                texU,
                texV));
    }

    void BoxSideTexture::accept(Visitor& visitor)
    {
        visitor.visit(*this);
    }

    void BoxSideTexture::raycast(const Raycast& ray, RayHitList& reports) const
    {
        RayHitReport* last = reports.head;
        Box::raycast(ray, reports);
        RayHitReport* node = reports.head;

        while(node != last)
        {
            RayHitReport& r = *node;

            if(glm::dot(r.normal, texW()) >
                    RayHitReport::EPSILON_LENGTH)
            {
                glm::dvec3 dist = r.position - texOrigin();
                r.texCoord.s = glm::dot(dist, texU()) / texU2();
                r.texCoord.t = glm::dot(dist, texV()) / texV2();
            }

            node = node->_next;
        }
    }


    // Box Band Texture
    BoxBandTexture::BoxBandTexture(
            const glm::dvec3& minCorner,
            const glm::dvec3& maxCorner,
            const glm::dvec3& texOrigin,
            const glm::dvec3& texU,
            const glm::dvec3& texV) :
        BoxTexture(minCorner, maxCorner, texOrigin, texU, texV)
    {

    }

    std::shared_ptr<Surface> BoxBandTexture::boxCorners(
            const glm::dvec3& minCorner,
            const glm::dvec3& maxCorner,
            const glm::dvec3& texOrigin,
            const glm::dvec3& texU,
            const glm::dvec3& texV)
    {
        return std::shared_ptr<Surface>(
            new BoxBandTexture(
                minCorner,
                maxCorner,
                texOrigin,
                texU,
                texV));
    }

    std::shared_ptr<Surface> BoxBandTexture::boxPosDims(
            const glm::dvec3& center,
            const glm::dvec3& dimensions,
            const glm::dvec3& texOrigin,
            const glm::dvec3& texU,
            const glm::dvec3& texV)
    {
        glm::dvec3 minCorner = center - dimensions / 2.0;
        glm::dvec3 maxCorner = center + dimensions / 2.0;
        return std::shared_ptr<Surface>(
            new BoxBandTexture(
                minCorner,
                maxCorner,
                texOrigin,
                texU,
                texV));
    }

    void BoxBandTexture::accept(Visitor& visitor)
    {
        visitor.visit(*this);
    }

    void BoxBandTexture::raycast(const Raycast& ray, RayHitList& reports) const
    {
        RayHitReport* last = reports.head;
        Box::raycast(ray, reports);
        RayHitReport* node = reports.head;

        while(node != last)
        {
            RayHitReport& r = *node;

            double dotW = glm::dot(r.normal, texW());

            if(dotW > RayHitReport::EPSILON_LENGTH)
            {
                glm::dvec3 dist = r.position - texOrigin();
                r.texCoord.s = glm::dot(dist, texU()) / texU2();
                r.texCoord.t = glm::dot(dist, texV()) / texV2();
            }
            else if(dotW < -RayHitReport::EPSILON_LENGTH)
            {
                glm::dvec3 dist = r.position - texOrigin();
                r.texCoord.s = -glm::dot(dist, texU()) / texU2();
                r.texCoord.t = glm::dot(dist, texV()) / texV2();
            }
            else
            {
                double dotU = glm::dot(r.normal, texU());
                if(dotU > RayHitReport::EPSILON_LENGTH)
                {
                    glm::dvec3 dist = r.position - texOrigin();
                    r.texCoord.s = -glm::dot(dist, texW()) / texW2();
                    r.texCoord.t = glm::dot(dist, texV()) / texV2();
                }
                else if(dotU < -RayHitReport::EPSILON_LENGTH)
                {
                    glm::dvec3 dist = r.position - texOrigin();
                    r.texCoord.s = glm::dot(dist, texW()) / texW2();
                    r.texCoord.t = glm::dot(dist, texV()) / texV2();
                }
            }

            node = node->_next;
        }
    }
}
