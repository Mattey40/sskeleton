#ifndef DIRECTIONHEIGHTEVENT_H
#define DIRECTIONHEIGHTEVENT_H

#include "CornerClone.h"
#include "HeightEvent.h"
#include "SkeletonCapUpdate.h"

class Machine;

class DirectionHeightEvent: public HeightEvent
{
    public:
        double newAngle;
        Machine *machine;

        DirectionHeightEvent(Machine *machine, double angle);
        DirectionHeightEvent(Machine *machine, double height, double angle);
        double getAngle(void);
        virtual double getHeight(void) const;
        virtual bool process(Skeleton *skel);

    protected:
        double height;

    private:
        void init(Machine *machine, double height, double angle);
};

#endif
