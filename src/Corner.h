#ifndef CORNER_H
#define CORNER_H

#include <boost/lexical_cast.hpp>
#include <iostream>
#include "Point3D.h"

class Edge;
class Skeleton;

class Corner: public Point3D
{
    public:
        Edge *nextL;
        Edge *prevL;
        Corner *nextC;
        Corner *prevC;

        Corner(double x, double y, double z);
        Corner(double x, double y);
        Corner(Point3D in);
        Point3D getLoc3(void);
        static void replace(Corner *old, Corner *neu, Skeleton *skel);
        bool operator==(const Corner &other) const;
};

size_t hash_value(Corner const& e);

#endif
