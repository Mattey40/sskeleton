#ifndef COSITEDCOLLISION_H
#define COSITEDCOLLISION_H

#ifdef __GNUC__
#include <execinfo.h>
#endif

#include <vector>
#include "Chain.h"
#include "ConsecutivePairs.h"
#include "ConsecutiveTriples.h"
#include "EdgeCollision.h"
#include "LinkedHashContainer.h"
#include "Point3D.h"

class HeightCollision;
class Skeleton;

/**
 * A bunch of faces that collide at one point
 */
class CoSitedCollision
{
    public:
        IndirectLinkedHashSet<EdgeCollision *> edges;
        Point3D loc;

        std::vector<Chain> chains;

        CoSitedCollision(Point3D loc, EdgeCollision *ec,
                HeightCollision *parent);
        void add(EdgeCollision *ec);
        bool findChains(Skeleton *skel);
        void validateChains(Skeleton *skel);
        bool processChains(Skeleton *skel);
        static Chain buildChain2(Corner *start,
                IndirectLinkedHashSet<Corner *> *input);
        friend std::ostream & operator<<(std::ostream &strm,
                const CoSitedCollision &c);

    private:
        HeightCollision *parent;

        bool hasAdjacent(Corner *a, Corner *b, Corner *c);
        bool moreOneSmashEdge(void);
};

#endif
