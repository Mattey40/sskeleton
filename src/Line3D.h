#ifndef LINE3D_H
#define LINE3D_H

#include "Point3D.h"
#include "Vector3D.h"

class Line3D
{
    public:
        Point3D origin;
        Vector3D direction;

        Line3D(void);
        Line3D(Tuple3D origin, Tuple3D direction);
        Point3D projectLine(Point3D ept);
        Point3D projectSegment(Point3D ept);
        double projectParam(const Tuple3D ept) const;
        friend std::ostream & operator<<(std::ostream &strm, const Line3D &l);

        inline bool isValid(void) { return valid; }

        inline static Line3D invalid(void)
        {
            Line3D l;
            l.valid = false;
            return l;
        }

    protected:
        bool valid;

    private:
        Point3D project(Point3D ept, bool segment);
};

#endif
