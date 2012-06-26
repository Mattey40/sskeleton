#ifndef HEIGHTCOLLISION_H
#define HEIGHTCOLLISION_H

#include <vector>
#include "CollisionQ.h"
#include "Corner.h"
#include "CoSitedCollision.h"
#include "EdgeCollision.h"
#include "HeightEvent.h"
#include "LinkedHashContainer.h"
#include "Skeleton.h"

class EdgeCollision;

/**
 * A bunch of faces that collide at the same height
 */
class HeightCollision: public HeightEvent
{
    public:
        HeightCollision(void);
        HeightCollision(std::vector<EdgeCollision *> coHeighted);
        double getHeight(void) const;
        virtual bool process(Skeleton *skel);
        void processHoriz(Skeleton *skel);
        void newHoriz(Corner *toAdd);
        friend std::ostream & operator<<(std::ostream &strm,
                const HeightCollision &h);

    private:
        double height;
        std::vector<EdgeCollision *> coHeighted;
        IndirectLinkedHashSet<Corner *> newHorizSet;
};

#endif
