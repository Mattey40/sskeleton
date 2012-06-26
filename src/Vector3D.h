#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <cmath>
#include "Point3D.h"
#include "Tuple3D.h"

class Vector3D : public Tuple3D
{
    public:
        Vector3D(void);
        Vector3D(Tuple3D t1);
        Vector3D(Point3D p);
        Vector3D(double x, double y, double z);
        void cross(Vector3D v1, Vector3D v2);
        double dot(const Vector3D v1) const;
        double length(void);
        void normalise(void);
        double angle(Vector3D v1);

        inline static Vector3D invalid(void)
        {
            Vector3D v;
            v.valid = false;
            return v;
        }
};

#endif
