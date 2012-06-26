#ifndef EDGE_H
#define EDGE_H

#include <boost/math/special_functions/fpclassify.hpp>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include "Corner.h"
#include "GC.h"
#include "LinearForm3D.h"
#include "LinkedHashContainer.h"
#include "Point3D.h"
#include "Vector3D.h"

class Corner;
class Machine;

class Edge
{
    public:
        Corner *start;
        Corner *end;

        /* orthogonal vector pointing uphill */
        Vector3D uphill;
        LinearForm3D linearForm;

        /* corners that currently reference this edge in prevL or nextL */
        IndirectLinkedHashSet<Corner *> currentCorners;

        Machine *machine;

        Edge(Corner *start, Corner *end, double angle);
        Edge(Corner *start, Corner *end);
        Edge(Point3D start, Point3D end, double angle);
        void calculateUphill(void);
        void calculateLinearForm(void);
        Vector3D direction(void);
        bool isCollisionNearHoriz(Edge *other);
        bool sameDirectedLine(Edge *nextL);
        double getAngle(void);
        Vector3D getPlaneNormal(void);
        void setAngle(double angle);
        bool operator==(const Edge &other) const;
        bool operator!=(const Edge &other) const;
        friend std::ostream & operator<<(std::ostream &strm, const Edge &e);

        static Tuple3D collide(Corner *a, double height);

    private:
        /* 0 is straight up, positive/-ve is inwards/outwards, absolute value
         * must be less than pi/2
         */
        double angle;
};

size_t hash_value(const Edge &s);

#endif
