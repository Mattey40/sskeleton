#ifndef COLLISIONQ_H
#define COLLISIONQ_H

#include <queue>
#include <stdexcept>
#include "EdgeCollision.h"
#include "HeightCollision.h"
#include "HeightEvent.h"

class Skeleton;

class CollisionQ
{
    public:
        typedef std::priority_queue<EdgeCollision *, vector<EdgeCollision *>,
                HeightEvent::PtrComparator> FaceEventQueue;
        typedef std::priority_queue<HeightEvent *, vector<HeightEvent *>,
                HeightEvent::PtrComparator> MiscEventQueue;

        Skeleton *skel;

        CollisionQ(Skeleton *skel);
        HeightEvent * poll(void);
        void add(HeightEvent *he);
        void addCorner(Corner *toAdd, HeightCollision *postProcess);
        void dump(void);
        void clearFaceEvents(void);
        void clearOtherEvents(void);
        void holdRemoves(void);
        void resumeRemoves(void);

    private:
        /* Collisions between edges */
        FaceEventQueue faceEvents;

        /* Other control events (gradient changes...) */
        MiscEventQueue miscEvents;

        HeightCollision *currentCoHeighted;
        bool _holdRemoves;
        std::vector<Corner *> removes;

        HeightEvent * nextEvent(void);
        void removeCorner(Corner *toAdd);
        void cornerEdgeCollision(Corner *corner, Edge *edge);
};

#endif
