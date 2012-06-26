#include "CollisionQ.h"
#include "Machine.h"
#include "Skeleton.h"

using namespace std;

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Skeleton::Skeleton(void) :
    height(0), refindFaceEvents(true)
{
    output = new Output(this);
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Skeleton::Skeleton(LoopL<Edge> *input) :
    height(0), refindFaceEvents(true)
{
    output = new Output(this);

    setupForEdges(input);
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Skeleton::Skeleton(LoopL<Corner> *corners) :
    height(0), refindFaceEvents(true)
{
    output = new Output(this);

    setup(corners);
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
Skeleton::~Skeleton(void)
{
    delete qu;
    delete output;
}

/*******************************************************************************
 
 Purpose - Stop-gap measure to convert loops of edges (BAD!) to loops of corners
 (GOOD!)

 *******************************************************************************/
void Skeleton::setupForEdges(LoopL<Edge> *input)
{
    LoopL<Corner> corners;
    for (vector<Loop<Edge> *>::iterator leit = input->v.begin();
            leit != input->v.end(); ++leit)
    {
        Loop<Edge> *le = *leit;

        Loop<Corner> *lc = GCNEW(Loop<Corner>);
        corners.v.push_back(lc);
        for (Loop<Edge>::LoopIterator eit = le->iterator(); eit.hasNext(); )
        {
            Edge *e = eit.next();

            lc->append(e->start);
            e->start->nextL = e;
            e->end->prevL = e;
            e->start->nextC = e->end;
            e->end->prevC = e->start;
        }
    }

    setup(&corners);
}

/*******************************************************************************
 
 Purpose - Sanitise input

 *******************************************************************************/
void Skeleton::setup(LoopL<Corner> *input)
{
    /* reset all! (not needed... but maybe in future) */
    height = 0;
    liveCorners.clear();
    liveEdges.clear();

    IndirectMultiHashMap<Edge *, Corner *> allEdges;

    for (LoopL<Corner>::EIterator cit = input->eIterator(); cit.hasNext(); )
    {
        Corner *c = cit.next();
        allEdges.put(c->nextL, c);
    }

    /* combine shared edges into single output faces */
    for (IndirectMultiHashMap<Edge *, Corner *>::MapT::iterator it =
            allEdges.map.begin(); it != allEdges.map.end(); ++it)
    {
        Edge *e = it->first;

        e->currentCorners.clear();
        vector<Corner *> corners = allEdges.get(e);
        Corner *first = corners.at(0);

        output->newEdge(first->nextL, NULL);

        for (size_t i = 1; i < corners.size(); i++)
            output->merge(first, corners.at(i));

        liveEdges.push_back(e);
    }

    for (LoopL<Corner>::EIterator cit = input->eIterator(); cit.hasNext(); )
    {
        Corner *c = cit.next();
        
        output->newDefiningSegment(c);
        liveCorners.push_back(c);
        c->nextL->currentCorners.push_back(c);
        c->prevL->currentCorners.push_back(c);
    }

    qu = new CollisionQ(this); // yay closely coupled classes

    for (IndirectMultiHashMap<Edge *, Corner *>::MapT::iterator it =
            allEdges.map.begin(); it != allEdges.map.end(); ++it)
    {
        Edge *e = it->first;

        e->machine->addEdge(e, this);
    }

    cout << "Setup(corners) liveCorners: " << liveCorners << endl;
    cout << "Setup(corners) liveEdges: " << liveEdges << endl;

    /* now all angles are set, find initial set of intersections (will remove)
     * corners if parallel enough)
     */
    refindFaceEventsIfNeeded();
}

/*******************************************************************************
 
 Purpose - Execute the skeleton algorithm

 *******************************************************************************/
void Skeleton::skeleton(void)
{
    validate();
    HeightEvent *he;

    while ((he = qu->poll()) != NULL)
    {
        try
        {
            cout << "***About to process he with height " << he->getHeight() <<
                " of type " << typeid(*he).name() << endl;
            if (he->process(this)) // business happens here
            {
                height = he->getHeight();
                validate();
            }
            cerr << "done at " << he->getHeight() << endl;
            refindFaceEventsIfNeeded();
        }
        catch (std::exception &e)
        {
            cerr << e.what() << endl;
        }
    }

    /* build output polygons from constructed graph */
    output->calculate(this);
}

/*******************************************************************************
 
 Purpose - This method returns a set of edges representing a horizontal slice
 through the skeleton at the specified height (given that no other events happen
 between current height and given cap height).

 Topology assumed final---e.g.---we take a copy at of the slice at the given
 height, not processing any more height events.

 Non-destructive---this doesn't change the skeleton. This routine is for taking
 output mid-way through evaluation.

 All output edges have the same machines as their originators.

 *******************************************************************************/
LoopL<Corner> * Skeleton::capCopy(double height)
{
    LinearForm3D ceiling(0, 0, 1, -height);

    for (IndirectLinkedHashSet<Corner *>::iterator cit = liveCorners.begin();
            cit != liveCorners.end(); ++cit)
    {
        Corner *c = *cit;

        try
        {
            Tuple3D t;

            // don't introduce instabilities if height is already as requested
            if (dIsApprox(height, c->z))
                t = Point3D(*c);
            else
                t = ceiling.collide(c->prevL->linearForm, c->nextL->linearForm);

            cornerMap.put(GCNEW(Corner(t)), c);
        }
        catch (std::exception &e)
        {
            /* assume they're all coincident? */
            cornerMap.put(GCNEW(Corner(c->x, c->y, height)), c);
        }
    }

    class EdgeCache: public IndirectCache<Corner *, Edge *>
    {
        public:
            EdgeCache(Skeleton *owner) : owner(owner) { }

            virtual Edge * create(Corner *&i)
            {
                Edge *edge = GCNEW(Edge(owner->cornerMap.teg(i),
                        owner->cornerMap.teg(i->nextC)));

                lowToHighEdge[i->nextL] = edge;

                edge->setAngle(i->nextL->getAngle());
                edge->machine = i->nextL->machine;

                return edge;
            }

        private:
            Skeleton *owner;
            IndirectLinkedHashMap<Edge *, Edge *> lowToHighEdge;
    };
    EdgeCache edgeCache(this);

    LoopL<Corner> *out = GCNEW(LoopL<Corner>());

    IndirectLinkedHashSet<Corner *> workingSet;
    workingSet.assign(liveCorners.begin(), liveCorners.end());
    while (!workingSet.empty())
    {
        Loop<Corner> *loop = GCNEW(Loop<Corner>);
        out->v.push_back(loop);
        Corner *current = *workingSet.begin();
        do
        {
            Corner *s = cornerMap.teg(current);
            Corner *e = cornerMap.teg(current->nextC);

            /* one edge may have two segments, but the topology will not change
             * between old and new, so we may store the leading corner to match
             * segments
             */
            segmentMap.addForwards(current, s);

            Edge *edge = edgeCache.get(current);

            loop->append(s);
            s->nextC = e;
            e->prevC = s;
            s->nextL = edge;
            e->prevL = edge;

            workingSet.remove(current);
            current = current->nextC;
        }
        while (workingSet.contains(current));
    }

    return out;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
void Skeleton::refindAllFaceEventsLater(void)
{
    refindFaceEvents = true;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
void Skeleton::refindFaceEventsIfNeeded(void)
{
    /* on demand */
    if (!refindFaceEvents)
        return;

    /**
     * Very expensive part---refind all collisions (including those already
     * processed)
     * MachineEvents remain in their current state
     *
     * should really only be done for those edges that have changed (change for
     * when we integrate eppstein's stuff)
     */

    /* context collects events that must be processed immediately following
     * (e.g. horizontals)
     */
    HeightCollision *context = GCNEW(HeightCollision);

    qu->clearFaceEvents();

    /* Because of CloneConfirmIterator stuff below where somewhere under
     * qu->addCorner() we delete parts of the set while iterating (Christ, what
     * a design), we back up the set first, iterating over the backed-up copy,
     * but checking first whether the original liveCorners still contains the
     * corner.
     * 
     * -Tom
     */
    IndirectLinkedHashSet<Corner *> bakLiveCorners;
    bakLiveCorners.insert(bakLiveCorners.end(), liveCorners.begin(),
            liveCorners.end());
    for (IndirectLinkedHashSet<Corner *>::iterator lcit = bakLiveCorners.begin();
            lcit != bakLiveCorners.end(); ++lcit)
    {
        Corner *lc = *lcit;
        if (!liveCorners.contains(lc))
        {
            cout << "*************DOES NOT CONTAIN!!!***********" << endl;
            continue;
        }
        qu->addCorner(lc, context);
    }

    /* Just a check that my assumption that we don't /add/ to liveCorners
     * while iterating is correct.
     *
     * -Tom
     */
    for (IndirectLinkedHashSet<Corner *>::iterator lcit = liveCorners.begin();
            lcit != liveCorners.end(); ++lcit)
    {
        Corner *lc = *lcit;
        if (!bakLiveCorners.contains(lc))
        {
            cout << "********liveCorners has but bakLiveCorners doesn't!: " <<
                lc << endl;
            assert(false);
        }
    }

    /* if we are not adding new events (and this isn't adding the input the
     * first time) this shouldn't do anything
     */
    context->processHoriz(this);
}

/*******************************************************************************
 
 Purpose - Debug!

 *******************************************************************************/
void Skeleton::validate(void)
{
    if (true)
    {
        cerr << "Validating..." << endl;
        IndirectLinkedHashSet<Corner *> all;
        all.insert(all.end(), liveCorners.begin(), liveCorners.end());

        //outer:
        while (!all.empty())
        {
            Corner *start = *all.begin();
            all.remove(start);

            Corner *next = start;

            int count = 0;

            do
            {
                cout << "...validation corner(" << count << "): " << *next << endl;
                count++;
                Corner *c = next->nextC;
                all.remove(c);

                Edge *e = next->nextL;

                struct ErrPrinter
                {
                    ErrPrinter(Edge *e, Corner *c, int lineNum)
                    {
                        cerr << " on edge is " << e << endl;
                        cerr << " validate error on corner " << c << " on line "
                            << lineNum << endl;
                    }
                };

                if (c->nextC->prevC != c)
                {
                    ErrPrinter(e, c, __LINE__);
                    goto finishedErrors;
                }
                if (c->prevC->nextC != c)
                {
                    ErrPrinter(e, c, __LINE__);
                    goto finishedErrors;
                }

                if (c->prevL != e)
                {
                    ErrPrinter(e, c, __LINE__);
                    goto finishedErrors;
                }
                if (c->prevC->nextL != e)
                {
                    ErrPrinter(e, c, __LINE__);
                    goto finishedErrors;
                }

                for (IndirectLinkedHashSet<Corner *>::iterator it =
                        liveCorners.begin(); it != liveCorners.end(); ++it)
                {
                    Corner *d = *it;

                    if (d->nextL == e || d->prevL == e)
                    {
                        if (!e->currentCorners.contains(d))
                        {
                            ErrPrinter(e, c, __LINE__);
                            goto finishedErrors;
                        }
                    }
                    else
                    {
                        if (e->currentCorners.contains(d))
                        {
                            ErrPrinter(e, c, __LINE__);
                            goto finishedErrors;
                        }
                    }
                }

                if (count >= 100)
                {
                    ErrPrinter(e, c, __LINE__);
                    goto finishedErrors;
                }

finishedErrors:
                if (count > 100)
                    goto continueOuter;

                next = c;
            }
            while (next != start);

            continueOuter: ;
        }
    }
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
int Skeleton::horizontalComparator(Edge *o1, Edge *o2)
{
    if (abs(o1->getAngle() - o2->getAngle()) < 0.0001)
        return 0;
    else if (o1->getAngle() < o2->getAngle())
        return -1;
    else
        return 1;
}
bool Skeleton::horizontalComparatorLT(Edge *o1, Edge *o2)
{
    /* Volume maximising resolution */
    return horizontalComparator(o1, o2) == -1;
}
