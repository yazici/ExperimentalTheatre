#include "StdTeam.h"

#include "Designer/StdDesigner.h"
#include "ArtDirector/ArtDirectorServer.h"
#include "Choreographer/StdChoreographer.h"

namespace prop3
{
    StdTeam::StdTeam() :
        AbstractTeam(new StdDesigner,
                     new StdChoreographer())
    {

    }

    StdTeam::~StdTeam()
    {

    }
}
