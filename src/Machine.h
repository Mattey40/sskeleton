#ifndef MACHINE_H
#define MACHINE_H

#include <vector>
#include "CollisionQ.h"
#include "DirectionHeightEvent.h"
#include "GC.h"
#include "HeightEvent.h"
#include "LinkedHashContainer.h"

/**
 * A machine controls the angle of its set of edges over time (height...)
 *
 * superclass of all machines
 *
 * idea is to add all directions before adding edges. When you add the first edge
 *
 * instead of just adding all height changes to the main event queue, this
 * machine only adds the next one. Reason here is that we might want to change
 * out minds as we build upwards.
 */
class Machine
{
    public:
        /* a machine will only ever have one pending event in the skeleton->qu,
         * others are stored here
         */
        std::vector<HeightEvent *> events;
        std::string description;

        /* Math.PI / 4---when an edge is added this is the angle it is given */
        double currentAngle;

        HeightEvent *heightEvent; // marked as transient? -Tom
        int currentDirection; // marked as transient? -Tom

        Machine(void);
        Machine(double initial);
        void addEdge(Edge *e, Skeleton *skel);
        void sortHeightEvents(void);
        void addHeightEvent(HeightEvent *dir);
        void findNextHeight(Skeleton *skel);
        std::vector<HeightEvent *> getDirections(void);

    protected:
        IndirectLinkedHashSet<Edge *> seenEdges;

    private:
        void init(double initial);
};

#endif
