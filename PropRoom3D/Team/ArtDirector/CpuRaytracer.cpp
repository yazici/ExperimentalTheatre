#include "CpuRaytracer.h"

#include <list>
#include <sstream>
#include <iostream>
#include <algorithm>

#include <CellarWorkbench/Misc/Log.h>

#include "../../Prop/Prop.h"
#include "../../Prop/Costume/Costume.h"
#include "../../Prop/Volume/Raycast.h"
#include "CpuRaytracerWorker.h"


namespace prop3
{
    const unsigned int CpuRaytracer::DEFAULT_WORKER_COUNT = 4;

    CpuRaytracer::CpuRaytracer() :
        _sampleCount(0),
        _draftLevel(0),
        _draftLevelCount(0),
        _draftThreadBatchPerLevel(0),
        _draftViewportSize(1, 1),
        _fastDraftEnabled(false),
        _fastDraftDone(false),
        _isUpdated(false),
        _viewportSize(1, 1),
        _workersInterrupted(false)
    {
        // hardware_concurrency is defined as a hint on the number of cores
        unsigned int coreCount =
            std::thread::hardware_concurrency();

        if(coreCount < 1)
        {
            coreCount = DEFAULT_WORKER_COUNT;
        }

        _workerObjects.resize(coreCount);

        init();
    }

    CpuRaytracer::CpuRaytracer(unsigned int  workerCount) :
        _sampleCount(0),
        _draftLevel(0),
        _draftLevelCount(0),
        _draftThreadBatchPerLevel(0),
        _draftViewportSize(1, 1),
        _fastDraftEnabled(false),
        _isUpdated(false),
        _viewportSize(1, 1),
        _workersInterrupted(false),
        _workerObjects(workerCount)
    {
        init();
    }


    void CpuRaytracer::init()
    {
        size_t workerCount = _workerObjects.size();

        std::stringstream ss;
        ss << "Using " << workerCount << " raytracer workers to render scene";
        cellar::getLog().postMessage(new cellar::Message(
            'I', false, ss.str(), "CpuRaytracer"));

        for(size_t i=0; i < workerCount; ++i)
        {
            _workerObjects[i].reset(new CpuRaytracerWorker());
            _workerThreads.push_back(std::thread(
                CpuRaytracerWorker::launchWorker,
                _workerObjects[i]));
        }

        bufferHardReset();
    }

    CpuRaytracer::~CpuRaytracer()
    {
        for(auto& w : _workerObjects)
        {
            w->terminate();
        }
        for(std::thread& t : _workerThreads)
        {
            t.join();
        }
    }

    void CpuRaytracer::reset()
    {
        _props.clear();

        for(auto& w : _workerObjects)
        {
            w->stop();
            w->setProps(_props);
        }

        bufferSoftReset();
    }

    bool CpuRaytracer::isDrafter() const
    {
        return _draftLevelCount != 0 && _draftThreadBatchPerLevel != 0;
    }

    bool CpuRaytracer::isDrafting() const
    {
        return isDrafter() && _draftLevel < _draftLevelCount;
    }

    void CpuRaytracer::setDraftParams(
            int levelCount,
            int levelSizeRatio,
            int threadBatchPerLevel)
    {
        if(isDrafting())
            abortDrafting();

        _draftLevelCount = levelCount;
        _draftLevelSizeRatio = levelSizeRatio;
        _draftThreadBatchPerLevel = threadBatchPerLevel;

        if(isDrafter())
            restartDrafting();
    }

    void CpuRaytracer::enableFastDraft(bool enable)
    {
        _fastDraftEnabled = enable;
    }

    void CpuRaytracer::gatherWorkerFrames()
    {
        if(_props.empty())
            return;


        if(_fastDraftEnabled)
        {
            if(!_fastDraftDone)
            {
                performNonStochasticSyncronousDraf();
                _fastDraftDone = true;
                return;
            }
        }


        if(_workersInterrupted)
        {
            _workersInterrupted = false;
            for(auto& w : _workerObjects)
            {
                if(!w->isRunning())
                {
                    w->start();
                }
            }
        }


        for(auto& w : _workerObjects)
        {
            while(w->completedFrameCount() != 0)
            {
                const float* pixels = w->readNextFrame();
                incorporateFrames(pixels, 1);
                w->popReadFrame();
            }
        }
    }

    void CpuRaytracer::pourFramesIn(
            const std::vector<float>& colorBuffer,
            unsigned int sampleCount)
    {
        abortDrafting();
        incorporateFrames(colorBuffer.data(), sampleCount);
    }

    void CpuRaytracer::pourFramesOut(
            std::vector<float>& colorBuffer,
            unsigned int& sampleCount)
    {
        sampleCount = _sampleCount;
        _sampleCount = 0;

        colorBuffer.resize(_colorBuffer.size());
        std::copy(_colorBuffer.begin(), _colorBuffer.end(), colorBuffer.begin());

        _isUpdated = false;
    }

    bool CpuRaytracer::isUpdated()
    {
        return _isUpdated;
    }

    bool CpuRaytracer::onUpdateConsumed()
    {
        _isUpdated = false;

        if(isDrafting())
        {
            if(_sampleCount >= _draftThreadBatchPerLevel * _workerThreads.size())
            {
                nextDraftSize();
            }
        }
    }

    float CpuRaytracer::convergenceValue() const
    {
        return _convergenceValue;
    }

    unsigned int CpuRaytracer::sampleCount() const
    {
        return _sampleCount;
    }

    const glm::ivec2& CpuRaytracer::viewportSize() const
    {
        if(isDrafting())
            return _draftViewportSize;
        return _viewportSize;
    }

    const std::vector<float>& CpuRaytracer::colorBuffer() const
    {
        return _colorBuffer;
    }

    void CpuRaytracer::resize(int width, int height)
    {
        _viewportSize = glm::ivec2(width, height);
        _colorBuffer.resize(width * height * 3);

        if(!isDrafting())
        {
            for(auto& w : _workerObjects)
            {
                w->stop();
                w->updateViewport(
                    _viewportSize,
                    glm::ivec2(0, 0),
                    _viewportSize);
            }
        }

        _workersInterrupted = true;
        bufferSoftReset();
    }

    void CpuRaytracer::updateView(const glm::dmat4& view)
    {
        for(auto& w : _workerObjects)
        {
            w->stop();
            w->updateView(view);
        }

        _workersInterrupted = true;
        bufferSoftReset();
    }

    void CpuRaytracer::updateProjection(const glm::dmat4& proj)
    {
        for(auto& w : _workerObjects)
        {
            w->stop();
            w->updateProjection(proj);
        }

        _workersInterrupted = true;
        bufferSoftReset();
    }

    void CpuRaytracer::manageProp(const std::shared_ptr<Prop>& prop)
    {
        _props.push_back(prop);

        for(auto& w : _workerObjects)
        {
            w->stop();
            w->setProps(_props);
        }

        _workersInterrupted = true;
        bufferSoftReset();
    }

    void CpuRaytracer::unmanageProp(const std::shared_ptr<Prop>& prop)
    {
        std::remove_if(_props.begin(), _props.end(),
            [&prop](const std::shared_ptr<Prop>& p) {
                return p == prop;
        });

        for(auto& w : _workerObjects)
        {
            w->stop();
            w->setProps(_props);
        }

        _workersInterrupted = true;
        bufferSoftReset();
    }

    unsigned int CpuRaytracer::propCount() const
    {
        return _props.size();
    }

    void CpuRaytracer::nextDraftSize()
    {
        if(!isDrafting())
            return;

        ++_draftLevel;
        _sampleCount = 0;
        _convergenceValue = 0;

        if(_draftLevel < _draftLevelCount)
        {
            int ratioPower = (_draftLevelCount - (_draftLevel+1));
            int ratio = glm::pow(2, ratioPower) * _draftLevelSizeRatio;

            _draftViewportSize = _viewportSize / glm::ivec2(ratio);
            _draftViewportSize = glm::max(_draftViewportSize, glm::ivec2(1));
        }
        else
        {
            _draftViewportSize = _viewportSize;
        }

        // Update worker buffers' size
        for(auto& w : _workerObjects)
        {
            w->updateViewport(
                _draftViewportSize,
                glm::ivec2(0, 0),
                _draftViewportSize);
        }
    }

    void CpuRaytracer::abortDrafting()
    {
        if(!isDrafting())
            return;

        _fastDraftDone = true;
        _draftLevel = _draftLevelCount-1;
        nextDraftSize();
    }

    void CpuRaytracer::restartDrafting()
    {
        if(!isDrafter())
            return;

        _fastDraftDone = false;
        _draftLevel = -1;
        nextDraftSize();
    }

    void CpuRaytracer::bufferSoftReset()
    {
        _sampleCount = 0;
        restartDrafting();
    }

    void CpuRaytracer::bufferHardReset()
    {
        _sampleCount = 0;
        restartDrafting();

        _colorBuffer.resize(_viewportSize.x * _viewportSize.y * 3);
        std::fill(_colorBuffer.begin(), _colorBuffer.end(), 0.0f);
    }

    void CpuRaytracer::incorporateFrames(
        const float* colorBuffer,
        unsigned int sampleCount)
    {
        _convergenceValue = 0.0;
        _sampleCount += sampleCount;
        float alpha = sampleCount / (float) _sampleCount;

        const glm::ivec2& viewport = viewportSize();
        int cc = viewport.x * viewport.y * 3;
        for(int i=0; i < cc; ++i)
        {
            float lastValue = _colorBuffer[i];
            float newValue = _colorBuffer[i] =
                glm::mix(
                    _colorBuffer[i],
                    colorBuffer[i],
                    alpha);

            float meanShift = glm::abs(newValue - lastValue);
            _convergenceValue += (meanShift * meanShift);
        }

        _convergenceValue = glm::sqrt(_convergenceValue) * sampleCount;
        _convergenceValue = 1.0f / (1.0f + _convergenceValue);
        _isUpdated = true;
    }

    void CpuRaytracer::performNonStochasticSyncronousDraf()
    {
        typedef decltype(_workerObjects.front()) WorkerPtr;
        typedef std::pair<glm::ivec2, WorkerPtr> DrafterType;
        std::list<DrafterType> drafters;

        int currOriginY = 0;
        unsigned int workerIdx = 0;
        unsigned int workerCount = _workerObjects.size();
        const glm::ivec2& viewport = viewportSize();
        for(auto& w : _workerObjects)
        {
            glm::ivec2 orig = glm::ivec2(0, currOriginY);
            int nextOriginY = (viewport.y * (++workerIdx)) / workerCount;
            int viewportHeight = nextOriginY - currOriginY;
            glm::ivec2 size(viewport.x, viewportHeight);

            w->usePixelJittering(false);
            w->useStochasticTracing(false);
            w->updateViewport(viewport, orig, size);
            w->start(true);

            glm::ivec2 viewtParam(currOriginY, viewportHeight);
            drafters.push_back(DrafterType(viewtParam, w));
            currOriginY = nextOriginY;
        }

        while(!drafters.empty())
        {
            auto it = drafters.begin();
            while(it != drafters.end())
            {
                WorkerPtr& worker = it->second;
                if(worker->completedFrameCount() != 0)
                {
                    const glm::ivec2 viewParams = it->first;
                    const float* pixels = worker->readNextFrame();
                    int baseIdx = viewParams.x * viewport.x * 3;
                    int topIdx = (viewParams.x + viewParams.y) * viewport.x * 3;

                    for(int i=baseIdx; i<topIdx; ++i)
                    {
                        _colorBuffer[i] = pixels[i - baseIdx];
                    }

                    worker->popReadFrame();
                    worker->usePixelJittering(true);
                    worker->useStochasticTracing(true);
                    it = drafters.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

        // Set this draft level as complete
        _sampleCount = _draftThreadBatchPerLevel * workerCount;

        // Notify user that a new frame is ready
        _isUpdated = true;
    }
}
