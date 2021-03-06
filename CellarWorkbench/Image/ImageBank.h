#ifndef CELLARWORKBENCH_IMAGEBANK_H
#define CELLARWORKBENCH_IMAGEBANK_H

#include <map>
#include <string>
#include <memory>
#include <stdexcept>

#include <CellarWorkbench/DesignPattern/Singleton.h>


#include "../libCellarWorkbench_global.h"


namespace cellar
{
    class Image;

    class CELLAR_EXPORT ImageBank : public cellar::Singleton<ImageBank>
    {
    private:
        friend class cellar::Singleton<ImageBank>;
        ImageBank();

    public:
        bool isInBank(
                const std::string& imgName) const;

        bool addImage(
                const std::string& imgName,
                const std::string& fileName);

        bool addImage(
                const std::string& imgName,
                const Image& image);

        Image& getImage(
                const std::string& imgName);

        std::shared_ptr<Image> getImagePtr(
                const std::string& imgName);

        bool deleteImage(
                const std::string& imgName);


    private:
        std::map< std::string, std::shared_ptr< Image > > _images;
    };

    CELLAR_EXPORT ImageBank& getImageBank();



    // IMPLEMENTATION //
    inline bool ImageBank::isInBank(
            const std::string& imgName) const
    {
        return _images.find(imgName) != _images.end();
    }
}

#endif // CELLARWORKBENCH_IMAGEBANK_H
