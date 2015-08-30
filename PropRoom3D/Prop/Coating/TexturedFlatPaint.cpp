#include "TexturedFlatPaint.h"

#include <GLM/gtc/random.hpp>

#include <CellarWorkbench/Image/Image.h>
#include <CellarWorkbench/Image/ImageBank.h>

#include "../Ray/Raycast.h"
#include "../Ray/RayUtils.h"
#include "../../StageSet/StageSetVisitor.h"


namespace prop3
{
    TexturedFlatPaint::TexturedFlatPaint(const std::string& texName,
                                 const glm::dvec3& defaultColor) :
        _texName(texName),
        _texture(cellar::getImageBank().getImage(texName)),
        _defaultColor(defaultColor)
    {
    }

    TexturedFlatPaint::~TexturedFlatPaint()
    {

    }

    void TexturedFlatPaint::indirectBrdf(
            std::vector<Raycast>& raycasts,
            const RayHitReport& report,
            const std::shared_ptr<Material>& leavedMaterial,
            const std::shared_ptr<Material>& enteredMaterial,
            unsigned int outRayCountHint) const
    {
        // Pigment diffuse reflection
        size_t preSize = raycasts.size();
        indirectDiffuseReflection(raycasts, report, leavedMaterial, outRayCountHint);
        size_t postSize = raycasts.size();

        glm::dvec3 color = _defaultColor;
        if(report.isTextured)
        {
            const glm::dvec3& texCoord = report.texCoord;
            int i = texCoord.s * _texture.width();
            int j = texCoord.t * _texture.height();
            unsigned char* pixel = _texture.pixel(
                glm::clamp(i, 0,  _texture.width()-1),
                glm::clamp(j, 0, _texture.height()-1));

            // Not blended with default color
            color.x = pixel[0] / 255.0;
            color.y = pixel[1] / 255.0;
            color.z = pixel[2] / 255.0;
        }

        for(size_t i=preSize; i<postSize; ++i)
        {
            raycasts[i].color *= color;
        }
    }

    glm::dvec3 TexturedFlatPaint::directBrdf(
            const RayHitReport& report,
            const glm::dvec3& outDirection,
            const std::shared_ptr<Material>& leavedMaterial,
            const std::shared_ptr<Material>& enteredMaterial) const
    {
        glm::dvec3 color = _defaultColor;
        if(report.isTextured)
        {
            const glm::dvec3& texCoord = report.texCoord;
            int i = texCoord.s * _texture.width();
            int j = texCoord.t * _texture.height();
            unsigned char* pixel = _texture.pixel(
                glm::clamp(i, 0,  _texture.width()-1),
                glm::clamp(j, 0, _texture.height()-1));

            // Not blended with default color
            color.x = pixel[0] / 255.0;
            color.y = pixel[1] / 255.0;
            color.z = pixel[2] / 255.0;
        }

        return color * directDiffuseReflection(report, outDirection);
    }

    void TexturedFlatPaint::accept(StageSetVisitor& visitor)
    {
        visitor.visit(*this);
    }
}
