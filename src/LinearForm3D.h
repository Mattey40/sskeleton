#ifndef LINEARFORM3D_H
#define LINEARFORM3D_H

#include <boost/math/special_functions/fpclassify.hpp>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <jama/jama_lu.h>
#include <jama/jama_svd.h>
#include <stdexcept>
#include "Line3D.h"
#include "LineOnPlane.h"
#include "Point3D.h"
#include "Vector3D.h"

class LinearForm3D
{
    public:
        double A; // x
        double B; // y
        double C; // z
        double D; // offset

        LinearForm3D(void);
        LinearForm3D(double A, double B, double C, double D);
        LinearForm3D(Vector3D normal, Tuple3D offset);
        Line3D collide(LinearForm3D other);
        Tuple3D collide(LinearForm3D b, LinearForm3D c);
        Point3D collide(Tuple3D rayOrigin, Tuple3D rayDirection);
        Point3D collide(Tuple3D rayOrigin, Tuple3D rayDirection,
                const double *distance);
        Vector3D collideToVector(LinearForm3D other);
        Vector3D createNormalVector(void);
        double pointDistance(Tuple3D point);
        bool operator==(const LinearForm3D &other) const;
        Vector3D normal(void);
        bool hasNaN(void);
};

#endif
