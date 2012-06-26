#ifndef POINT_3D
#define POINT_3D

#include "Tuple3D.h"

class Point3D : public Tuple3D
{
    public:
        Point3D(void);
        Point3D(double x, double y, double z);
        Point3D(Tuple3D t1);
        double distance(Point3D p1);

        inline static Point3D invalid(void)
        {
            Point3D p;
            p.valid = false;
            return p;
        }
};

#endif
