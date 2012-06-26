#include "DirectionHeightEvent.h"

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
DirectionHeightEvent::DirectionHeightEvent(Machine *machine, double angle)
{
    init(machine, 0, angle);
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
DirectionHeightEvent::DirectionHeightEvent(Machine *machine, double height,
        double angle)
{
    init(machine, height, angle);
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
void DirectionHeightEvent::init(Machine *machine, double height, double angle)
{
    this->machine = machine;
    this->height = height;
    this->newAngle = angle;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
double DirectionHeightEvent::getAngle(void)
{
    return newAngle;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
double DirectionHeightEvent::getHeight(void) const
{
    return height;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
bool DirectionHeightEvent::process(Skeleton *skel)
{
    machine->currentAngle = newAngle;

    SkeletonCapUpdate update(skel);

    /* add in the output edges for the outgoing face */
    LoopL<Corner> *cap = update.getCap(height);

    CornerClone cc(cap);

    /* preserve corner information for assigning parents, later */
    IndirectDHash<Corner *, Corner *> nOCorner;
    cc.nOCorner.shallowDupe(nOCorner);

    for (LoopL<Corner>::EIterator cit = cc.output->eIterator(); cit.hasNext();)
    {
        Corner *c = cit.next();

        /* corners are untouched if neither attached edge has this machine */
        if (c->nextL->machine == machine || c->prevL->machine == machine)
            nOCorner.removeA(c);

        /* segments are untouched if they don't contain this machine */
        if (c->nextL->machine == machine)
            cc.nOSegments.removeA(c);
    }

    update.update(cc.output, cc.nOSegments, nOCorner);

    /**
     * Must now update the parent field in output for the new edges, so that we
     * have a know where a face came from
     */
    for (IndirectLinkedHashSet<Corner *>::iterator cit =
            skel->liveCorners.begin(); cit != skel->liveCorners.end(); ++cit)
    {
        Corner *c = *cit;

        if (c->nextL->machine == machine)
        {
            Corner *old = update.getOldBaseLookup().get(cc.nOCorner.get(c));

            skel->output->setParent(c->nextL->start, old->nextL->start);
        }
    }

    machine->findNextHeight(skel);
    return true;
}
