#include "StdCoating.h"

#include <GLM/gtc/random.hpp>
#include <GLM/gtc/matrix_transform.hpp>

#include "Ray/RayHitReport.h"
#include "Prop/Material/Material.h"


namespace prop3
{
    // The constant represents the fact that subsurface
    // scattering only returns half of the rays while the other
    // half is scattered further into material's depths...
    //const double INTERNAL_SCATTER_LOSS = 0.61803398876;
    const double INTERNAL_SCATTER_LOSS = 1.0;

    StdCoating::StdCoating()
    {

    }

    StdCoating::~StdCoating()
    {

    }

    void StdCoating::indirectBrdf(
        std::vector<Raycast>& raycasts,
        const RayHitReport& report,
        const Material& leavedMaterial,
        const Material& enteredMaterial) const
    {
        // Report's shorthands
        const glm::dvec3& pos = report.position;
        const glm::dvec3& tex = report.texCoord;
        const glm::dvec3& wallNormal = report.normal;
        const glm::dvec3& incident = report.incidentRay.direction;
        const glm::dvec3& reflectOrig = report.reflectionOrigin;
        const glm::dvec3& refractOrig = report.refractionOrigin;


        // StdCoating properties
        double rough = roughness(tex);
        double roughnessEntropy = glm::mix(
                Raycast::FULLY_SPECULAR_ENTROPY,
                Raycast::FULLY_DIFFUSIVE_ENTROPY,
                rough);

        glm::dvec4 paintFrag = paintColor(tex);
        glm::dvec3 pColor = glm::dvec3(paintFrag);
        double pOpa = paintFrag.a;


        // Entered material properties
        double lRIdx = leavedMaterial.refractiveIndex(pos);


        // Paint Reflection
        if(pOpa > 0.0)
        {
            double paintRIdx = paintRefractiveIndex(tex);

            glm::dvec3 diffuseColor = pColor * INTERNAL_SCATTER_LOSS;
            glm::dvec3 reflectColor = color::white;


            glm::dvec3 diffuseNormal = getMicrofacetNormal(
                    wallNormal,
                    incident,
                    1.0); // Fully diffusive

            glm::dvec3 reflectNormal = getMicrofacetNormal(
                    wallNormal,
                    incident,
                    rough);

            glm::dvec3 diffuseDirection =
                    glm::reflect(incident, diffuseNormal);

            glm::dvec3 reflectDirection =
                    glm::reflect(incident, reflectNormal);

            double reflectionRatio =
                    computeReflexionRatio(
                        lRIdx,
                        paintRIdx,
                        incident,
                        reflectNormal);

            double diffuseWeight = (1.0 - reflectionRatio) * pOpa;
            double reflectWeight = reflectionRatio * pOpa;

            glm::dvec3 attDiffuseColor = diffuseColor *
                    glm::dot(diffuseDirection, wallNormal);

            // Diffuse
            raycasts.push_back(Raycast(
                    Raycast::BACKDROP_DISTANCE,
                    diffuseWeight,
                    Raycast::FULLY_DIFFUSIVE_ENTROPY,
                    attDiffuseColor,
                    reflectOrig,
                    diffuseDirection));

            // Specular
            raycasts.push_back(Raycast(
                    Raycast::BACKDROP_DISTANCE,
                    reflectWeight,
                    roughnessEntropy,
                    reflectColor,
                    reflectOrig,
                    reflectDirection));
        }

        // Totally opaque paint
        if(pOpa >= 1.0)
            return;


        // Metal reflection
        double eMatWeight = (1 - pOpa);
        double eCond = enteredMaterial.conductivity(pos);
        glm::dvec3 eColor = enteredMaterial.color(pos);
        if(eCond > 0.0)
        {
            const glm::dvec3& metallicColor = eColor;
            double metallicWeight = eMatWeight * eCond;

            glm::dvec3 reflectNormal = getMicrofacetNormal(
                    wallNormal,
                    incident,
                    rough);

            glm::dvec3 reflectDir = glm::reflect(incident, reflectNormal);

            // Metallic
            raycasts.push_back(Raycast(
                    Raycast::BACKDROP_DISTANCE,
                    metallicWeight,
                    roughnessEntropy,
                    metallicColor,
                    reflectOrig,
                    reflectDir));
        }

        // Totally metallic
        if(eCond >= 1.0)
            return;


        // Dielectric scattering
        double eRIdx = enteredMaterial.refractiveIndex(pos);
        double eOpa = enteredMaterial.opacity(pos);

        double insulatorWeight = eMatWeight * (1 - eCond);


        // Total scattering of the light at the surface
        if(eOpa >= 1.0)
        {
            // This is a shortcut: Since light is almost totally absorbed
            // near the surface, we prevent the ray from entering the
            // material by averaging the effect through a diffuse reflexion.
            double eScat = enteredMaterial.scattering(pos);
            double eScatNorm = (1/(1/(1-eScat) - 1) + 1);
            glm::dvec3 diffuseBase = glm::pow(eColor, glm::dvec3(eScatNorm));
            diffuseBase *= INTERNAL_SCATTER_LOSS;
            glm::dvec3 reflectColor = color::white;


            glm::dvec3 diffuseNormal = getMicrofacetNormal(
                    wallNormal,
                    incident,
                    1.0); // Fully diffusive

            glm::dvec3 reflectNormal = getMicrofacetNormal(
                    wallNormal,
                    incident,
                    rough);

            glm::dvec3 diffuseDirection =
                    glm::reflect(incident, diffuseNormal);

            glm::dvec3 reflectDirection =
                    glm::reflect(incident, reflectNormal);

            double reflectionRatio =
                    computeReflexionRatio(
                        lRIdx,
                        eRIdx,
                        incident,
                        reflectNormal);


            double diffuseWeight = (1.0 - reflectionRatio) * insulatorWeight;
            double reflectWeight = reflectionRatio * insulatorWeight;

            glm::dvec3 diffuseColor = diffuseBase *
                   glm::dot(diffuseDirection, wallNormal);


            // Specular
            if(rough < 1.0)
            {
                raycasts.push_back(Raycast(
                        Raycast::BACKDROP_DISTANCE,
                        reflectWeight,
                        roughnessEntropy,
                        reflectColor,
                        reflectOrig,
                        reflectDirection));
            }
            else
            {
                diffuseWeight += reflectWeight;
                diffuseColor = glm::mix(diffuseColor,
                                    reflectColor, reflectionRatio);
            }

            // Diffuse
            raycasts.push_back(Raycast(
                    Raycast::BACKDROP_DISTANCE,
                    diffuseWeight,
                    Raycast::FULLY_DIFFUSIVE_ENTROPY,
                    diffuseColor,
                    reflectOrig,
                    diffuseDirection));
        }
        else
        {
            glm::dvec3 refractColor = color::white;
            glm::dvec3 reflectColor = color::white;


            glm::dvec3 reflectNormal = getMicrofacetNormal(
                    wallNormal,
                    incident,
                    rough);

            glm::dvec3 reflectDir = glm::reflect(incident, reflectNormal);

            glm::dvec3 refractDir =
                    computeRefraction(
                        lRIdx,
                        eRIdx,
                        incident,
                        reflectNormal);


            double reflectionRatio =
                    computeReflexionRatio(
                        lRIdx,
                        eRIdx,
                        incident,
                        reflectNormal);

            double refractWeight = (1.0 - reflectionRatio) * insulatorWeight;
            double reflectWeight = reflectionRatio * insulatorWeight;


            // Refraction
            if(glm::dot(refractDir, wallNormal) < 0.0)
            {
                raycasts.push_back(Raycast(
                        Raycast::BACKDROP_DISTANCE,
                        refractWeight,
                        roughnessEntropy,
                        refractColor,
                        refractOrig,
                        refractDir));
            }
            else
            {
                reflectWeight += refractWeight;
            }

            // Reflexion
            raycasts.push_back(Raycast(
                    Raycast::BACKDROP_DISTANCE,
                    reflectWeight,
                    roughnessEntropy,
                    reflectColor,
                    reflectOrig,
                    reflectDir));
        }
    }

    glm::dvec3 StdCoating::directBrdf(
            const RayHitReport& report,
            const glm::dvec3& outDirection,
            const Material& leavedMaterial,
            const Material& enteredMaterial) const
    {
        assert(false);
		return color::black;
    }

    glm::dvec3 StdCoating::albedo(const RayHitReport& report) const
    {
        glm::dvec4 paint = paintColor(report.texCoord);
        glm::dvec3 matCol = report.nextMaterial->color(report.position);
        return glm::mix(matCol, glm::dvec3(paint), paint.a);
    }

    inline glm::dvec3 StdCoating::getMicrofacetNormal(
            const glm::dvec3& wallNormal,
            const glm::dvec3& incidentDir,
            double rough)
    {
        if(rough <= 0.0)
            return wallNormal;

        glm::dvec3 diffuse = glm::sphericalRand(1.0);
        if(glm::dot(diffuse, wallNormal) < 0.0)
            diffuse = -diffuse;

        if(rough >= 1.0)
            return glm::normalize((diffuse - incidentDir) / 2.0);

        glm::dvec3 specular = glm::reflect(incidentDir, wallNormal);
        glm::dvec3 glossy = glm::mix(specular, diffuse, rough);

        glm::dvec3 normal = glm::normalize((glossy - incidentDir) / 2.0);
        return normal;
    }
}
