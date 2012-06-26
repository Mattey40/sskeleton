#include "CollisionQ.h"

using namespace std;

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
CollisionQ::CollisionQ(Skeleton *skel) :
    skel(skel), currentCoHeighted(NULL), _holdRemoves(false)
{
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
HeightEvent * CollisionQ::nextEvent(void)
{
    EdgeCollision *ec = NULL;

    while (!faceEvents.empty())
    {
        ec = faceEvents.top();
        faceEvents.pop();

        /* valid if we haven't seen it, and it's height is "greater" than the
         * current skeleton height
         */
        if (!skel->seen.contains(ec) && ec->loc.z - skel->height > -0.001)
            break;
    }

    HeightEvent *he = miscEvents.empty() ? NULL : miscEvents.top();

    if (ec == NULL)
    {
        HeightEvent *tmp = NULL;
        if (!miscEvents.empty())
        {
            tmp = miscEvents.top();
            miscEvents.pop();
        }
        return tmp; // might be null!
    }
    if (he == NULL)
    {
        skel->seen.push_back(ec);
        return ec; // might be null! (I hope not, look above -Tom)
    }

    if (he->getHeight() <= ec->getHeight())
    {
        faceEvents.push(ec);

        HeightEvent *tmp = NULL;
        if (!miscEvents.empty())
        {
            tmp = miscEvents.top();
            miscEvents.pop();
        }
        return tmp;
    }
    else
    {
        skel->seen.push_back(ec);
        return ec;
    }
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
HeightEvent * CollisionQ::poll(void)
{
    currentCoHeighted = NULL; // now working at a new height

    HeightEvent *next = nextEvent();

    EdgeCollision *ec = dynamic_cast<EdgeCollision *>(next);
    if (ec)
    {
        vector<EdgeCollision *> coHeighted;
        coHeighted.push_back(ec);

        double height = ec->getHeight();

        while (!faceEvents.empty())
        {
            EdgeCollision *higher = faceEvents.top();
            
            if (higher->getHeight() - height < 0.00001) // ephermeral random constant #34 was 0.00001
            {
                faceEvents.pop(); // same as higher

                if (skel->seen.contains(higher))
                    continue;

                height = higher->getHeight();
                skel->seen.push_back(higher);
                coHeighted.push_back(higher);
            }
            else
                break;
        }

        return currentCoHeighted = GCNEW(HeightCollision(coHeighted));
    }
    else
        return next;
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void CollisionQ::add(HeightEvent *he)
{
    EdgeCollision *ec = dynamic_cast<EdgeCollision *>(he);
    if (ec)
        faceEvents.push(ec);
    else
        miscEvents.push(he);
}

/*******************************************************************************

  Purpose - Collide the new edge (toAdd->prev, toAdd->next) against all other
  edges. Will also add 3 consecutive edges.

 *******************************************************************************/
void CollisionQ::addCorner(Corner *toAdd, HeightCollision *postProcess)
{
    cout << endl << endl << "CollisionQ adding corner: " << *toAdd << endl;
    /* check these two edges don't share the same face */
    if (toAdd->prevL->sameDirectedLine(toAdd->nextL))
    {
        removeCorner(toAdd);

        cout << "Two edges share the same face" << endl;

        return;
    }

    /* loop of two---dissolves to a ridge */
    if (toAdd->prevL == toAdd->nextC->nextL)
    {
        Edge *edgearr[2]; edgearr[0] = toAdd->prevL; edgearr[1] = toAdd->nextL;
        skel->output->addOutputSideTo(toAdd, toAdd->nextC, 2, edgearr);

        /* we really should automate this */
        toAdd->nextL->currentCorners.remove(toAdd);
        toAdd->nextL->currentCorners.remove(toAdd->nextC);
        toAdd->prevL->currentCorners.remove(toAdd);
        toAdd->prevL->currentCorners.remove(toAdd->nextC);

        if (toAdd->nextL->currentCorners.empty())
            skel->liveEdges.remove(toAdd->nextL);

        if (toAdd->prevL->currentCorners.empty())
            skel->liveEdges.remove(toAdd->prevL);

        skel->liveCorners.remove(toAdd);
        skel->liveCorners.remove(toAdd->nextC);

        cout << "Loop of two, dissolves to a ridge" << endl;

        return;
    }

    /* Horizontal bisectors are rounded up and evaluated before leaving the
     * current height event
     */
    cout << "About to collide edges: " << *toAdd->prevL << " and " << *toAdd->nextL << endl;
    if (toAdd->prevL->isCollisionNearHoriz(toAdd->nextL))
    {
        /* if not a peak, add as a unsolved horizontal bisector */
        if (toAdd->nextL->direction().angle(toAdd->prevL->direction()) < 0.01)
            postProcess->newHoriz(toAdd);
        /* if just a peak, assume the loops-of-two-rule will finish it awf */

        cout << "Collision near horiz" << endl;

        return;
    }

    for (IndirectLinkedHashSet<Edge *>::iterator it = skel->liveEdges.begin();
            it != skel->liveEdges.end(); ++it)
        cornerEdgeCollision(toAdd, *it);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void CollisionQ::cornerEdgeCollision(Corner *corner, Edge *edge)
{
    cout << endl << "cornerEdgeCollision: " << *corner << " and " << *edge << endl;
    /* check for the uphill vector of both edges being too similar (parallel
     * edges)
     * also rejects e == corner->nextL or corner->prevL
     * updated to take into account vertical edges---will always have same
     * uphill! - (so we check edge direction too)
     */
    if (    (edge->uphill.angle(corner->prevL->uphill) < 0.0001 && 
                edge->direction().angle(corner->prevL->uphill) < 0.0001) ||
            (edge->uphill.angle(corner->nextL->uphill) < 0.0001 && 
                edge->direction().angle(corner->nextL->uphill) < 0.0001))
        return;

    Tuple3D res = Tuple3D::invalid();
    try
    {
        /* sometimes locks up here if edge linear form has NaN component */
        if (corner->prevL->linearForm.hasNaN() ||
                corner->nextL->linearForm.hasNaN() || edge->linearForm.hasNaN())
            throw runtime_error("linear form has NaN component");
        res = edge->linearForm.collide(corner->prevL->linearForm,
                corner->nextL->linearForm);
        cout << "res: " << res << endl << endl;
    }
    catch (std::exception &e)
    {
        /* trying to collide parallel-ish faces, don't bother */
        cerr << e.what() << endl;
        cerr << "didn't like colliding " << *edge << " and " << *corner->prevL
            << " and " << *corner->nextL << endl;
        return;
    }

    if (res.isValid())
    {
        /* cheap reject: if collision is equal or below (not the correct place
         * to check) the corner, don't bother with it
         */
        if (res.z < corner->z || res.z < edge->start->z)
            return;

        EdgeCollision *ec = GCNEW(EdgeCollision(res, corner->prevL, corner->nextL,
                edge));

        if (!skel->seen.contains(ec))
            faceEvents.push(ec);
        cout << "Have sent edgecollision for res: " << res << endl;
    }
}

/*******************************************************************************

  Purpose - Given corner should be fully linked into the network. Needs to be
  removed as it connects two parallel faces. We remove toAdd->nextL

 *******************************************************************************/
void CollisionQ::removeCorner(Corner *toAdd)
{
    if (_holdRemoves)
    {
        removes.push_back(toAdd);
        return;
    }

    cerr << "about to delete " << *toAdd << endl;

    /* update corners */
    toAdd->prevC->nextC = toAdd->nextC;
    toAdd->nextC->prevC = toAdd->prevC;

    /* update edges */
    toAdd->nextC->prevL = toAdd->prevL;

    /* update main corner list */
    skel->liveCorners.remove(toAdd);

    /* brute force search for all references to old edge (if this was on a per-
     * face basis it'd be much nicer)
     */
    for (IndirectLinkedHashSet<Corner *>::iterator lcit =
            skel->liveCorners.begin(); lcit != skel->liveCorners.end(); ++lcit)
    {
        Corner *lc = *lcit;
        if (lc->nextL == toAdd->nextL)
            lc->nextL = toAdd->prevL;
        if (lc->prevL == toAdd->nextL)
            lc->prevL = toAdd->prevL;
    }

    if (toAdd->prevL != toAdd->nextL)
    {
        /* update live edge list */
        skel->liveEdges.remove(toAdd->nextL);

        /* update edges's live corners */
        for (IndirectLinkedHashSet<Corner *>::iterator cit =
                toAdd->nextL->currentCorners.begin();
                cit != toAdd->nextL->currentCorners.end(); ++cit)
        {
            Corner *c = *cit;
            toAdd->prevL->currentCorners.push_back(c);
        }

        /* add to the results map likewise */
        skel->output->merge(toAdd->prevC, toAdd);

        /* all collisions need recalculation. This situation could be avoided
         * if collisions occur strictly with infinite faces. Recurse through
         * all consecutive colinear faces...?
         */
        skel->refindAllFaceEventsLater();
    }

    /* update edges's live corners (might have copied this over from nextL) */
    toAdd->prevL->currentCorners.remove(toAdd);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void CollisionQ::holdRemoves(void)
{
    removes.clear();
    _holdRemoves = true;
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void CollisionQ::resumeRemoves(void)
{
    _holdRemoves = false;
    for (vector<Corner *>::iterator it = removes.begin(); it != removes.end();
            ++it)
        if (skel->liveCorners.contains(*it)) // if hasn't been removed by horiz decomp
            removeCorner(*it);
    removes.clear();
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void CollisionQ::dump(void)
{
    vector<EdgeCollision *> tmpV;

    int i = 0;
    while (!faceEvents.empty())
    {
        cout << i++ << " : [" << faceEvents.top()->getHeight() << "] " << *faceEvents.top() << endl;
        tmpV.push_back(faceEvents.top());
        faceEvents.pop();
    }

    while (!tmpV.empty())
    {
        faceEvents.push(tmpV.back());
        tmpV.pop_back();
    }
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void CollisionQ::clearFaceEvents(void)
{
    while (!faceEvents.empty())
        faceEvents.pop();
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void CollisionQ::clearOtherEvents(void)
{
    while (!miscEvents.empty())
        miscEvents.pop();
}
