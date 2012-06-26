#ifndef SKELETON_H
#define SKELETON_H

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <map>
#include "Cache.h"
#include "Corner.h"
#include "DHash.h"
#include "Edge.h"
#include "Feature.h"
#include "LinkedHashContainer.h"
#include "Loop.h"
#include "ManyManyMap.h"
#include "MultiHashMap.h"
#include "Output.h"

class CollisionQ;
class EdgeCollision;
class Output;

class Skeleton
{
    public:
        IndirectLinkedHashSet<Corner *> liveCorners;
        IndirectLinkedHashSet<Edge *> liveEdges;
        CollisionQ *qu;
        double height;

        /* we store the triplets of faces we've already passed out to stop
         * repeats (insensitive to face order)
         */
        IndirectLinkedHashSet<EdgeCollision *> seen;

        /* output data */
        LoopL<Corner *> flatTop;
        Output *output;

        IndirectLinkedHashMap<Edge *, LinkedHashSet<Feature> > planFeatures;

        IndirectDHash<Corner *, Corner *> cornerMap; // contains lookup for results (new->old)
        IndirectManyManyMap<Corner *, Corner *> segmentMap; // contains lookup for results (old->new)

        Skeleton(void);
        Skeleton(LoopL<Edge> *input);
        Skeleton(LoopL<Corner> *corners);

        virtual ~Skeleton(void);

        void setupForEdges(LoopL<Edge> *input);
        void setup(LoopL<Corner> *input);
        void skeleton(void);
        LoopL<Corner> * capCopy(double height);

        void refindAllFaceEventsLater(void);
        void refindFaceEventsIfNeeded(void);
        void validate(void);

        static int horizontalComparator(Edge *o1, Edge *o2);
        static bool horizontalComparatorLT(Edge *o1, Edge *o2);

    private:
        /* lazy system for refinding all face events. True so we run it once at
         * start.
         */
        bool refindFaceEvents;
};

#endif
