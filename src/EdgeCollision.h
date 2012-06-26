#ifndef EDGECOLLISION_H
#define EDGECOLLISION_H

#include <boost/functional.hpp>
#include <stdexcept>
#include "Edge.h"
#include "HeightEvent.h"
#include "Point3D.h"
#include "Skeleton.h"

class HeightCollision;
class Skeleton;

class EdgeCollision: public HeightEvent
{
    public:
        Point3D loc;
        Edge *a, *b, *c;

        EdgeCollision(Point3D location, Edge *e1, Edge *e2, Edge *e3);

        bool operator==(const EdgeCollision &e) const;
        double getHeight(void) const;
        virtual bool process(Skeleton *skel);
        static void processConsecutive(Point3D *loc, Corner *a, Corner *b,
                Skeleton *skel);
        static void processJump(Point3D *loc, Corner *a, Corner *an, Corner *b,
                Skeleton *skel, HeightCollision *hc);
        static Corner * findCorner(Edge *in, Point3D collision, Skeleton *skel);
        friend std::ostream & operator<<(std::ostream &strm,
                const EdgeCollision &c);
};

size_t hash_value(EdgeCollision const& e);

#endif
