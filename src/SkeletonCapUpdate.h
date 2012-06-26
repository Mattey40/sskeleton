#ifndef SKELETONCAPUPDATE_H
#define SKELETONCAPUPDATE_H

#include <vector>
#include "Corner.h"
#include "DHash.h"
#include "Edge.h"
#include "Line3D.h"
#include "Loop.h"
#include "Machine.h"
#include "SetCorrespondence.h"

class Skeleton;

/**
 * Arbitrary topology update of the corners/edges on the sweep plane
 */
class SkeletonCapUpdate
{
    public:
        class Segment
        {
            public:
                Corner *corner;
                bool top;
                bool start;

                Segment(void);
                Segment(Corner *corner, bool top, bool start);
        };

        class EdgeInfo
        {
            public:
                Edge *base;
                std::vector<Segment> segs;

                EdgeInfo(void);
                EdgeInfo(Edge *base);
                void addTopSeg(Corner *c);
                void addBottomSeg(Corner *c);
                std::vector<Segment> sort(void);
        };

        SkeletonCapUpdate(Skeleton *skel);
        LoopL<Corner> * getCap(double height);
        IndirectDHash<Corner *, Corner *> getOldBaseLookup(void);
        void update(LoopL<Corner> *newPlan,
                IndirectSetCorrespondence<Corner *, Corner *> nOSegments,
                IndirectDHash<Corner *, Corner *> nOCorner);

    private:
        class LineProjectionComparator
        {
            public:
                LineProjectionComparator(Point3D start, Point3D end);
                bool operator()(Segment o1, Segment o2) const;

            private:
                Line3D line;
        };

        Skeleton *skel;
        double height;
        IndirectDHash<Corner *, Corner *> oBCorner;

        LoopL<Corner> *oldCorners;
        IndirectLinkedHashMap<Edge *, EdgeInfo> edgeInfo;

        /* given in update */
        IndirectDHash<Corner *, Corner *> nOCorner;
        IndirectSetCorrespondence<Corner *, Corner *> nOSegments;

        void collectCorners(Corner *neu,
                IndirectLinkedHashSet<Corner *> *toDelete);
};

#endif
