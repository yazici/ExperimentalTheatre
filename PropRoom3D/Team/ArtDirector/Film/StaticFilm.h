#ifndef PROPROOM3D_STATICFILM_H
#define PROPROOM3D_STATICFILM_H

#include "Film.h"


namespace prop3
{
    class PROP3D_EXPORT StaticFilm : public Film
    {
    public:
        StaticFilm();
        virtual ~StaticFilm();

        using Film::addSample;
        using Film::pixelSample;
        using Film::pixelPriority;

        virtual const std::vector<glm::vec3>& colorBuffer(ColorOutput colorOutput) override;

        virtual void backupAsReferenceShot() override;

        virtual bool saveReferenceShot(const std::string& name) const override;
        virtual bool loadReferenceShot(const std::string& name) override;
        virtual bool clearReferenceShot() override;

        virtual bool saveRawFilm(const std::string& name) const override;
        virtual bool loadRawFilm(const std::string& name) override;

        virtual double compileDivergence() const override;

        virtual bool needNewTiles() const override;
        virtual void tileCompleted(Tile& tile) override;
        virtual void rewindTiles() override;

    protected:
        virtual void resetFilmState() override;
        virtual void clearBuffers(const glm::dvec3& color) override;
        virtual void endTileReached() override;
        virtual double pixelDivergence(int index) const override;
        virtual double pixelPriority(int index) const override;
        virtual glm::dvec4 pixelSample(int index) const override;
        virtual void addSample(int index, const glm::dvec4& sample) override;
    };
}

#endif // PROPROOM3D_STATICFILM_H
