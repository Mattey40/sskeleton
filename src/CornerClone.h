#ifndef CORNERCLONE_H
#define CORNERCLONE_H

#include "Cache.h"
#include "Corner.h"
#include "DHash.h"
#include "Edge.h"
#include "Loop.h"
#include "SetCorrespondence.h"

class CornerClone
{
    public:
        LoopL<Corner> *output;
        IndirectSetCorrespondence<Corner *, Corner *> nOSegments;
        IndirectDHash<Corner *, Corner *> nOCorner;

        CornerClone(LoopL<Corner> *input);
        bool addSegment(void);
        bool addCorner(void);
};

#endif
