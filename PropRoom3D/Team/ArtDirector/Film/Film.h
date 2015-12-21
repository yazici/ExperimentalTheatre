#ifndef PROPROOM3D_FILM_H
#define PROPROOM3D_FILM_H

#include <vector>
#include <memory>
#include <condition_variable>

#include <GLM/glm.hpp>

#include "Tile.h"


namespace prop3
{
    class PROP3D_EXPORT Film
    {
    public:
        Film();
        virtual ~Film();

        int frameWidth() const;
        int frameHeight() const;
        glm::ivec2 frameResolution() const;
        void resizeFrame(int frameWidth, int frameHeight);
        virtual void resizeFrame(const glm::ivec2& resolution);

        int tilesWidth() const;
        int tilesHeight() const;
        glm::ivec2 tilesResolution() const;
        void resizeTiles(int tilesWidth, int tilesHeight);
        virtual void resizeTiles(const glm::ivec2& resolution);

        bool newTileCompleted();
        bool newFrameCompleted();


        const std::vector<glm::vec3>& colorBuffer() const;

        virtual void clear(const glm::dvec3& color = glm::dvec3(0),
                           bool hardReset = false) = 0;

        virtual double compileDivergence() const = 0;

        glm::dvec4 sample(int i, int j) const;
        glm::dvec4 sample(const glm::ivec2& position) const;

        void addSample(int i, int j, const glm::dvec4& sample);
        void addSample(const glm::ivec2& position, const glm::dvec4& sample);

        void setColor(int i, int j, const glm::dvec3& color);
        void setColor(const glm::ivec2& position, const glm::dvec3& color);

        double pixelVariance(int i, int j) const;
        double pixelVariance(const glm::ivec2& position) const;

        virtual void mergeFilm(const Film& film);

        virtual std::shared_ptr<Tile> nextTile();
        virtual std::shared_ptr<Tile> endTile();

        void waitFrameCompletion();
        virtual void tileCompleted(Tile& tile) = 0;


    protected:
        virtual void endTileReached() = 0;
        virtual glm::dvec4 sample(int index) const = 0;
        virtual double pixelVariance(int index) const = 0;
        virtual void setColor(int index, const glm::dvec3& color) = 0;
        virtual void addSample(int index, const glm::dvec4& sample) = 0;

        virtual void buildTiles();


        size_t _framePassCount;
        glm::ivec2 _frameResolution;
        std::vector<glm::vec3> _colorBuffer;

        std::mutex _cvMutex;
        std::condition_variable _cv;

        int _nextTileId;
        std::mutex _tilesMutex;
        glm::ivec2 _tilesResolution;
        double _varianceThreshold;
        std::shared_ptr<Tile> _endTile;
        std::vector<std::shared_ptr<Tile>> _tiles;

        bool _newTileCompleted;
        bool _newFrameCompleted;
    };



    // IMPLEMENTATION //
    inline int Film::frameWidth() const
    {
        return _frameResolution.x;
    }

    inline int Film::frameHeight() const
    {
        return _frameResolution.y;
    }

    inline glm::ivec2 Film::frameResolution() const
    {
        return _frameResolution;
    }

    inline void Film::resizeFrame(int width, int height)
    {
        resizeFrame(glm::ivec2(width, height));
    }

    inline int Film::tilesWidth() const
    {
        return _tilesResolution.x;
    }

    inline int Film::tilesHeight() const
    {
        return _tilesResolution.y;
    }

    inline glm::ivec2 Film::tilesResolution() const
    {
        return _tilesResolution;
    }

    inline void Film::resizeTiles(int width, int height)
    {
        resizeTiles(glm::ivec2(width, height));
    }

    inline const std::vector<glm::vec3>& Film::colorBuffer() const
    {
        return _colorBuffer;
    }

    inline glm::dvec4 Film::sample(int i, int j) const
    {
        int index = i + j * _frameResolution.x;
        return sample(index);
    }

    inline glm::dvec4 Film::sample(const glm::ivec2& position) const
    {
        return sample(position.x, position.y);
    }

    inline void Film::addSample(
            int i, int j, const glm::dvec4& sample)
    {
        int index = i + j * _frameResolution.x;
        addSample(index, sample);
    }

    inline void Film::addSample(
            const glm::ivec2& position,
            const glm::dvec4& sample)
    {
        addSample(position.x, position.y, sample);
    }

    inline void Film::setColor(
            int i, int j, const glm::dvec3& color)
    {
        int index = i + j * _frameResolution.x;
        setColor(index, color);
    }

    inline void Film::setColor(
            const glm::ivec2& position,
            const glm::dvec3& color)
    {
        setColor(position.x, position.y, color);
    }

    inline double Film::pixelVariance(int i, int j) const
    {
        int index = i + j * _frameResolution.x;
        return pixelVariance(index);
    }

    inline double Film::pixelVariance(const glm::ivec2& position) const
    {
        return pixelVariance(position.x, position.y);
    }
}

#endif // PROPROOM3D_FILM_H
