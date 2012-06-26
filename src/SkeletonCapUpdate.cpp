#include "SkeletonCapUpdate.h"

#include "Skeleton.h"

using namespace std;

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
SkeletonCapUpdate::EdgeInfo::EdgeInfo(void) :
    base(NULL)
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
SkeletonCapUpdate::EdgeInfo::EdgeInfo(Edge *base) :
    base(base)
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
void SkeletonCapUpdate::EdgeInfo::addTopSeg(Corner *c)
{
    segs.push_back(Segment(c, true, true));
    segs.push_back(Segment(c->nextC, true, false));
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
void SkeletonCapUpdate::EdgeInfo::addBottomSeg(Corner *c)
{
    segs.push_back(Segment(c, false, true));
    segs.push_back(Segment(c->nextC, false, false));
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
std::vector<SkeletonCapUpdate::Segment> SkeletonCapUpdate::EdgeInfo::sort(void)
{
    std::sort(segs.begin(), segs.end(), LineProjectionComparator(*base->start,
                *base->end));

    return segs;
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
SkeletonCapUpdate::Segment::Segment(void) :
    corner(NULL)
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
SkeletonCapUpdate::Segment::Segment(Corner *corner, bool top, bool start) :
    corner(corner), top(top), start(start)
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
SkeletonCapUpdate::LineProjectionComparator::
    LineProjectionComparator(Point3D start, Point3D end)
{
    Vector3D dir(end);
    dir.sub(start);
    line = Line3D(start, dir);
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
bool SkeletonCapUpdate::LineProjectionComparator::
    operator()(Segment o1, Segment o2) const
{
    return line.projectParam(*o1.corner) < line.projectParam(*o2.corner);
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
SkeletonCapUpdate::SkeletonCapUpdate(Skeleton *skel) :
    skel(skel)
{
}

/*******************************************************************************
 
  Purpose - Returns a copy of "old" loop. Users are expected to duplicate it.

  > you must not modify it
  > you must not change the .currentCorners

 *******************************************************************************/
LoopL<Corner> * SkeletonCapUpdate::getCap(double height)
{
    this->height = height;
    oldCorners = skel->capCopy(height);
    oBCorner = skel->cornerMap;

    for (LoopL<Corner>::EIterator cit = oldCorners->eIterator(); cit.hasNext();)
    {
        Corner *c = cit.next();
        
        c->z = 0;

        Corner *baseC = skel->cornerMap.get(c);

        EdgeInfo ei;
        if (!edgeInfo.containsKey(baseC->nextL))
        {
            ei = EdgeInfo(baseC->nextL);
            edgeInfo[baseC->nextL] = ei;
        }
        ei.addBottomSeg(c);
    }

    for (LoopL<Corner>::EIterator cit = oldCorners->eIterator(); cit.hasNext();)
    {
        Corner *c = cit.next();
        c->nextL->calculateLinearForm();
    }

    return oldCorners;
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
IndirectDHash<Corner *, Corner *> SkeletonCapUpdate::getOldBaseLookup(void)
{
    return oBCorner;
}

/*******************************************************************************

  Purpose - The complicated bit!

  Given a new topology, and a map of what correponds to the old bits, we ensure
  the output-faces of the skeleton are continuous. A picture of teh algorithm
  is a SVG file.

  When we update there are several possibilities:
  (*) One edge may split to several parallel edges
  (*) Several edges may combine to one (if parallel)
  (*) Edges are created/destroyed
  
  newPlan---The new plan that we're updating to
  nOSegments---The edge-correspondence between the new plan and the old. These
  two segments (corner->corner.nextC) are aligned, and the edge remains the same
  nOCorner---The corner-correspondence between the new plan and the old. A
  corner has an entry here if it is in exzacedey the same place.

  NewPlan contains new edges, if there is a mapping for them in nOSegments, then
  the skeleton should only contain the corresponding edge from base.

 *******************************************************************************/
void SkeletonCapUpdate::update(LoopL<Corner> *newPlan,
        IndirectSetCorrespondence<Corner *, Corner *> nOSegments,
        IndirectDHash<Corner *, Corner *> nOCorner)
{
    this->nOSegments = nOSegments;
    this->nOCorner = nOCorner;

    for (IndirectDHash<Corner *, Corner *>::ABMapT::key_iterator oldIt =
            oBCorner.ab.begin(); oldIt != oBCorner.ab.end(); ++oldIt)
        oldIt->first->z = height;

    for (LoopL<Corner>::EIterator cit = newPlan->eIterator(); cit.hasNext();)
        cit.next()->z = height;

    for (LoopL<Corner>::EIterator neuIt = newPlan->eIterator();
            neuIt.hasNext();)
    {
        Corner *neu = neuIt.next();

        neu->nextL->calculateLinearForm();

        /* before neu list is broken up (by collectCorners), store the segments
         * in edgeInfo
         */
        IndirectLinkedHashSet<Corner *> *setA = nOSegments.getSetA(neu);
        for (IndirectLinkedHashSet<Corner *>::iterator it = setA->begin();
                it != setA->end(); ++it)
        {
            Corner *oldSegment = *it;
            
            Edge *baseEdge = oBCorner.get(oldSegment)->nextL;

            EdgeInfo ei;
            if (!edgeInfo.containsKey(baseEdge))
            {
                ei = EdgeInfo(baseEdge);
                edgeInfo[baseEdge] = ei;
            }
            ei.addTopSeg(neu);
        }
    }

    IndirectLinkedHashSet<Corner *> cornersToDelete;
    cornersToDelete.insert(cornersToDelete.end(), skel->liveCorners.begin(),
            skel->liveCorners.end());

    for (LoopL<Corner>::EIterator cit = newPlan->eIterator(); cit.hasNext();)
        collectCorners(cit.next(), &cornersToDelete);

    for (IndirectLinkedHashSet<Corner *>::iterator it = cornersToDelete.begin();
            it != cornersToDelete.end(); ++it)
    {
        Corner *baseC = *it;

        Corner *old = oBCorner.teg(baseC);
        /* add vertical edge */
        Edge *edgeArr[2]; edgeArr[0] = baseC->prevL; edgeArr[1] = baseC->nextL;
        skel->output->addOutputSideTo(old, baseC, 2, edgeArr);

        baseC->prevL->currentCorners.remove(baseC);
        baseC->nextL->currentCorners.remove(baseC);

        /* actual removal happens below */
    }

    for (IndirectLinkedHashSet<Corner *>::iterator it = cornersToDelete.begin();
            it != cornersToDelete.end(); ++it)
        skel->liveCorners.remove(*it);

    /* edgeInfo contains all segments from old and new that pretend to come from
     * the same old edge
     */
    //oldEdges:
    for (IndirectLinkedHashMap<Edge *, EdgeInfo>::key_iterator it =
            edgeInfo.begin(); it != edgeInfo.end(); ++it)
    {
        vector<Segment> segs;
        Segment trailing;

        Edge *baseE = it->first;

        /* ei = it->second?? -Tom */
        EdgeInfo ei = edgeInfo[baseE];
        if (ei.base == NULL)
            goto continueOldEdges;
            
        segs = ei.sort();

        //segs:
        for (vector<Segment>::iterator it = segs.begin(); it != segs.end();
                ++it)
        {
            Segment &s = *it;

            if (nOCorner.containsA(s.corner) || nOCorner.containsB(s.corner))
            {
                /* the corner isn't being replaced by edge, nothing to do */
                if (trailing.corner != NULL)
                    throw std::runtime_error("trailing != NULL");
                /* don't count towards towards alternating trailing */
                goto continueSegs;
            }
            /* if it's one of the bottoms */
            else if (!s.top)
            {
                /* add vertical end for base->old */
                Corner *baseC = oBCorner.get(s.corner);
                Edge *edgeArr[2];
                edgeArr[0] = baseC->prevL; edgeArr[1] = baseC->nextL;
                skel->output->addOutputSideTo(s.corner, baseC, 2, edgeArr);
                baseE->currentCorners.remove(s.corner);
            }
            /* one of the tops is being used */
            else
            {
                /* just add the relevant entries in the corner maps */
                skel->liveCorners.push_back(s.corner);
                baseE->currentCorners.push_back(s.corner);
                if (s.start)
                    s.corner->nextL = baseE;
                else
                    s.corner->prevL = baseE;
            }

            if (trailing.corner != NULL) // polarity needs fixing up? first section sometimes capped, sometimes not.
            {
                /* because the cap (base) is at height 0, we elevate it here */
                Edge *edgeArr[1]; edgeArr[0] = baseE;
                skel->output->addOutputSideTo(true, trailing.corner, s.corner,
                        1, edgeArr);
                trailing.corner = NULL; // every other segment
            }
            else
                trailing = s; // every other segment

            continueSegs: ;
        }

        continueOldEdges: ;
    }

    /**
     * Pointers may have been removed by previous/next edges, or some edges may
     * have been removed entirely
     */
    /* this is broken---where do we remove other corners that might reference
     * this edge?
     */
    for (IndirectLinkedHashSet<Edge *>::iterator it = skel->liveEdges.begin();
            it != skel->liveEdges.end(); )
    {
        Edge *e = *it;
        if (e->currentCorners.size() == 0)
        {
            it = skel->liveEdges.erase(it);
            skel->liveCorners.remove(e->start);
            skel->liveCorners.remove(e->end);
        }
        else
            ++it;
    }

    skel->refindAllFaceEventsLater();

    cerr << "post cap update" << endl;
    skel->validate();
}

/*******************************************************************************
 
  Purpose - This accumulates the structure of EdgeInfos, detailing the points to
  be inserted into each edge of the old structure.

  toDelete---remove the corner from the list if it shouldn't be deleted

 *******************************************************************************/
void SkeletonCapUpdate::collectCorners(Corner *neu,
        IndirectLinkedHashSet<Corner *> *toDelete)
{
    IndirectLinkedHashSet<Corner *> *oldSegments = nOSegments.getSetA(neu);

    Corner *old = NULL;
    Corner *base = NULL;
    try { old = nOCorner.get(neu); } catch (std::exception &e) { }
    try { base = oBCorner.get(old); } catch (std::exception &e) { }

    Edge *neuEdge = neu->nextL;
    if (oldSegments->empty()) // is an entirely new edge, add it in!
    {
        if (!skel->liveEdges.contains(neuEdge))
        {
            skel->liveEdges.push_back(neuEdge);

            skel->output->newEdge(neuEdge, NULL);
            neuEdge->machine->addEdge(neuEdge, skel);
        }

        skel->output->newDefiningSegment(neu);
    }

    if (old != NULL)
    {
        if (base == NULL)
            throw std::runtime_error("topology of old and base does not match");

        neu->prevC->nextC = base;
        neu->nextC->prevC = base;
        base->nextC = neu->nextC;
        base->prevC = neu->prevC;

        toDelete->remove(base); // we'll use that, thanks
    }
    else // old == null (=> base == null)
    {
        skel->liveCorners.push_back(neu);

        neuEdge->currentCorners.push_back(neu);
        neu->prevL->currentCorners.push_back(neu);
    }
}
