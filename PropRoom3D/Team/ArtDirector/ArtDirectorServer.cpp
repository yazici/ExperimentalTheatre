#include "ArtDirectorServer.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include <CellarWorkbench/Camera/Camera.h>
#include <CellarWorkbench/Misc/Log.h>

#include "CpuRaytracerEngine.h"
#include "Film/AbstractFilm.h"
#include "GlPostProdUnit.h"
#include "Node/StageSet.h"


namespace prop3
{
    ArtDirectorServer::ArtDirectorServer() :
        _colorBufferTexId(0),
        _postProdUnit(new GlPostProdUnit())
    {
#ifdef NDEBUG
        _localRaytracer.reset(new CpuRaytracerEngine(8));
#else
        _localRaytracer.reset(new CpuRaytracerEngine(1));
#endif
    }

    ArtDirectorServer::~ArtDirectorServer()
    {
        glDeleteTextures(1, &_colorBufferTexId);
        _colorBufferTexId = 0;
    }

    void ArtDirectorServer::setup(const std::shared_ptr<StageSet>& stageSet)
    {
        // Color texture
        glGenTextures(1, &_colorBufferTexId);
        glBindTexture(GL_TEXTURE_2D, _colorBufferTexId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        clearColorTexture();

        _stageSet = stageSet;

        double divergenceThreshold = -1.0;
        RaytracerState::DraftParams draftParams;
        draftParams.sizeRatio = 4;
        draftParams.levelCount = 1;
        draftParams.frameCountPerLevel = 1;
        draftParams.fastDraftEnabled = true;

        _localRaytracer->setup(divergenceThreshold, draftParams);

        if(_postProdUnit)
        {
            _postProdUnit->setColorBufferTexId(_colorBufferTexId);
            _postProdUnit->setup();
        }

        camera()->refresh();
    }

    void ArtDirectorServer::update(double dt)
    {
        if(_stageSet->stageSetChanged())
        {
            clearColorTexture();
        }

        _localRaytracer->update(_stageSet);

        // TODO wbussiere 2015-05-01 : retreive client frames
        //_localRaytracer->pourFramesIn();
    }

    void ArtDirectorServer::draw(double dt)
    {
        if(_localRaytracer->isUpdated())
        {
            sendBuffersToGpu();
            // Let raytracer manage its drafts
            _localRaytracer->onUpdateConsumed();
        }

        _postProdUnit->execute();
    }

    void ArtDirectorServer::reset()
    {
        _localRaytracer->reset();
        clearColorTexture();

        // TODO wbussiere 2015-05-01 : reset clients
    }

    void ArtDirectorServer::notify(cellar::CameraMsg &msg)
    {
        clearColorTexture();
        if(msg.change == cellar::CameraMsg::EChange::VIEWPORT)
        {
            const glm::ivec2& viewport = msg.camera.viewport();
            _localRaytracer->resize(viewport.x, viewport.y);
        }
        else if(msg.change == cellar::CameraMsg::EChange::PROJECTION)
        {
            _localRaytracer->updateProjection(msg.camera.projectionMatrix());
        }
        else if(msg.change == cellar::CameraMsg::EChange::VIEW)
        {
            _localRaytracer->updateView(msg.camera.viewMatrix());
        }


        // TODO wbussiere 2015-05-01 : notify clients
    }

    std::shared_ptr<GlPostProdUnit> ArtDirectorServer::postProdUnit() const
    {
        return _postProdUnit;
    }

    std::shared_ptr<RaytracerState> ArtDirectorServer::raytracerState() const
    {
        return _localRaytracer->raytracerState();
    }

    void ArtDirectorServer::sendBuffersToGpu()
    {
        const AbstractFilm& film = _localRaytracer->film();
        glm::ivec2 viewportSize = film.resolution();

        std::vector<glm::vec3> colorBuffer;
        film.getColorBuffer(colorBuffer);

        // Send image to GPU
        glBindTexture(GL_TEXTURE_2D, _colorBufferTexId);
        glTexImage2D(GL_TEXTURE_2D,         0,  GL_RGB32F,
                     viewportSize.x,        viewportSize.y,
                     0, GL_RGB, GL_FLOAT,   colorBuffer.data());


        // Output image stats
        unsigned int sampleCount = raytracerState()->sampleCount();

        std::stringstream ss;
        ss << /*"Frame " <<*/ sampleCount;

        if(!raytracerState()->isDrafting())
        {
            //* Human readable
            float renderTime = raytracerState()->renderTime();
            float secPerFrame = renderTime / sampleCount;
            float divergence = raytracerState()->divergence();

            ss << "\t(";
            ss.precision(3);
            ss << std::fixed << std::setw(10) << renderTime << " sec";
            ss << ", ";
            ss.precision(3);
            ss << std::fixed << std::setw(10)  << secPerFrame << " sec/frame";
            ss << ", ";
            ss.precision(3);
            ss << std::scientific << std::setw(10) << divergence << " div";
            ss << ")";
            //*/

            /* CVS File
            float renderTime = _localRaytracer->renderTime();
            float secPerFrame = renderTime / sampleCount;
            float divergence = _localRaytracer->divergenceValue();
            float imgVariance = _localRaytracer->imageVariance();

            ss << ", ";
            ss.precision(3);
            ss << std::fixed << std::setw(10) << renderTime << ", ";
            ss.precision(3);
            ss << std::fixed << std::setw(10)  << secPerFrame << ", ";
            ss.precision(3);
            ss << std::scientific << std::setw(10) << divergence << ", ";
            ss.precision(3);
            ss << std::scientific << std::setw(10) << imgVariance << ";";
            //*/
        }
        else
        {
            ss << " [Draft]";
        }

        cellar::getLog().postMessage(new cellar::Message(
            'I', false, ss.str(), "CpuRaytracerServer"));


        if(!raytracerState()->isDrafting() && sampleCount == 8)
        {
            //exit(0);
        }
    }

    void ArtDirectorServer::clearColorTexture()
    {
        const float black[] = {0, 0, 0};

        glBindTexture(GL_TEXTURE_2D, _colorBufferTexId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, black);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
