#ifndef TUPLE3D_H
#define TUPLE3D_H

#include <boost/functional/hash.hpp>
#include <sstream>
#include "Defines.h"

class Tuple3D
{
    public:
        double x;
        double y;
        double z;

        Tuple3D(void);
        Tuple3D(double x, double y, double z);
        virtual ~Tuple3D(void) { }
        virtual void add(const Tuple3D t1);
        virtual void scale(double s);
        virtual void sub(const Tuple3D t1);
        bool operator==(const Tuple3D &other) const;
        bool operator!=(const Tuple3D &other) const;
        friend bool operator<(const Tuple3D &lhs, const Tuple3D &rhs);
        friend std::ostream & operator<<(std::ostream &strm, const Tuple3D &t);
        inline bool isValid(void) { return valid; }

        inline static Tuple3D invalid(void)
        {
            Tuple3D t;
            t.valid = false;
            return t;
        }

    protected:
        bool valid;
};
size_t hash_value(Tuple3D const& t);

#endif
