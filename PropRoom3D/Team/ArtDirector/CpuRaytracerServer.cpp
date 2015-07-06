#include "CpuRaytracerServer.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include <CellarWorkbench/Camera/Camera.h>
#include <CellarWorkbench/Misc/Log.h>

#include "CpuRaytracer.h"
#include "GlPostProdUnit.h"


namespace prop3
{
    CpuRaytracerServer::CpuRaytracerServer() :
        _colorBufferTexId(0),
        _localRaytracer(new CpuRaytracer()),
        _postProdUnit(new GlPostProdUnit())
    {
    }

    CpuRaytracerServer::~CpuRaytracerServer()
    {
        glDeleteTextures(1, &_colorBufferTexId);
        _colorBufferTexId = 0;
    }

    void CpuRaytracerServer::setup()
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

        _localRaytracer->enableFastDraft(true);
        _localRaytracer->setDraftParams(2, 4, 1);

        if(_postProdUnit)
        {
            _postProdUnit->setColorBufferTexId(_colorBufferTexId);
            _postProdUnit->setup();
        }
    }

    void CpuRaytracerServer::reset()
    {
        _localRaytracer->reset();
        clearColorTexture();

        // TODO wbussiere 2015-05-01 : reset clients
    }

    void CpuRaytracerServer::draw(double dt)
    {
        if(_localRaytracer->isUpdated())
        {
            sendBuffersToGpu();
            // Let raytracer manage its drafts
            _localRaytracer->onUpdateConsumed();
        }

        _postProdUnit->execute();
    }

    void CpuRaytracerServer::update(double dt)
    {
        _localRaytracer->gatherWorkerFrames();

        // TODO wbussiere 2015-05-01 : retreive client frames
        //_localRaytracer->pourFramesIn();
    }

    void CpuRaytracerServer::notify(cellar::CameraMsg &msg)
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

    void CpuRaytracerServer::manageProp(const std::shared_ptr<Prop>& prop)
    {
        clearColorTexture();
        _localRaytracer->manageProp(prop);

        // TODO wbussiere 2015-05-01 : notify clients
    }

    void CpuRaytracerServer::unmanageProp(const std::shared_ptr<Prop>& prop)
    {
        clearColorTexture();
        _localRaytracer->unmanageProp(prop);

        // TODO wbussiere 2015-05-01 : notify clients
    }

    std::shared_ptr<GlPostProdUnit> CpuRaytracerServer::postProdUnit() const
    {
        return _postProdUnit;
    }

    void CpuRaytracerServer::sendBuffersToGpu()
    {
        const glm::ivec2& viewportSize = _localRaytracer->viewportSize();
        const std::vector<float>& colorBuffer = _localRaytracer->colorBuffer();


        // Send image to GPU
        glBindTexture(GL_TEXTURE_2D, _colorBufferTexId);
        glTexImage2D(GL_TEXTURE_2D,         0,  GL_RGB,
                     viewportSize.x,        viewportSize.y,
                     0, GL_RGB, GL_FLOAT,   colorBuffer.data());


        // Output image stats
        unsigned int sampleCount = _localRaytracer->sampleCount();

        std::stringstream ss;
        ss << /*"Frame " <<*/ sampleCount;

        if(!_localRaytracer->isDrafting())
        {
            //* Human readable
            float renderTime = _localRaytracer->renderTime();
            float secPerFrame = renderTime / sampleCount;
            float divergence = _localRaytracer->divergenceValue();

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


        if(!_localRaytracer->isDrafting() && sampleCount == 8)
        {
            //exit(0);
        }
    }

    void CpuRaytracerServer::clearColorTexture()
    {
        const float black[] = {0, 0, 0};

        glBindTexture(GL_TEXTURE_2D, _colorBufferTexId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, black);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
