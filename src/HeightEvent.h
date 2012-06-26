#ifndef HEIGHTEVENT_H
#define HEIGHTEVENT_H

#include "Skeleton.h"

class Skeleton;
class HeightEvent
{
    public:
        virtual ~HeightEvent(void) { }

        virtual double getHeight(void) const = 0;

        /**
         * Return true if the event takes some action, false otherwise
         */
        virtual bool process(Skeleton *skel) = 0;


        struct PtrComparator
        {
            bool operator()(const HeightEvent *o1, const HeightEvent *o2)
            {
                return o2->getHeight() < o1->getHeight();
            }
        };

        struct Comparator
        {
            bool operator()(const HeightEvent &o1, const HeightEvent &o2)
            {
                return o2.getHeight() < o1.getHeight();
            }
        };
};

#endif
