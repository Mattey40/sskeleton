#include "HeightCollision.h"

using namespace boost;
using namespace std;

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
HeightCollision::HeightCollision(void) :
    height(0)
{
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
HeightCollision::HeightCollision(std::vector<EdgeCollision *> coHeighted) :
    coHeighted(coHeighted)
{
    height = coHeighted[0]->getHeight();
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
double HeightCollision::getHeight(void) const
{
    return height;
}

/*******************************************************************************

  Purpose - This is a bit of a quest!

  Assumption is that there are no parallel edges creating horizontal bisectors
  in the current loops. We create some here, then process them all, again
  removing all horizontal bisectors from the current loops.

  Returns true if topology has changed, false (we ignored all events)

 *******************************************************************************/
bool HeightCollision::process(Skeleton *skel)
{
    bool changed = false;

    vector<CoSitedCollision *> coSited;

    /* I love the smell of O(n^2) in the morning */
    // ec:
    for (vector<EdgeCollision *>::iterator ecit = coHeighted.begin();
            ecit != coHeighted.end(); ++ecit)
    {
        EdgeCollision *ec = *ecit;
        for (vector<CoSitedCollision *>::iterator cscit = coSited.begin();
                cscit != coSited.end(); ++cscit)
        {
            CoSitedCollision *csc = *cscit;
            if (ec->loc.distance(csc->loc) < 0.01)
            {
                csc->add(ec);
                goto continue_ec;
            }
        }
        coSited.push_back(GCNEW(CoSitedCollision(ec->loc, ec, this)));

        continue_ec: ;
    }

    /**
     * todo: This is a two-step process, for (I suspect) historical reasons. It
     * should be possible to find the chains as we go using line-projection.
     */
    for (vector<CoSitedCollision *>::iterator cit = coSited.begin();
            cit < coSited.end(); )
    {
        CoSitedCollision *css = *cit;

        bool newer = css->findChains(skel);
        vector<Chain> nC = css->chains;

        if (!newer)
            cit = coSited.erase(cit);
        else
            ++cit;
    }

    /**
     * We don't remove any points as it merges faces. All the information
     * (chains etc..) contains references to the faces that we don't want
     * destroyed as the faces merge.
     */
    skel->qu->holdRemoves();

    for (vector<CoSitedCollision *>::iterator cit = coSited.begin();
            cit != coSited.end(); ++cit)
    {
        CoSitedCollision *css = *cit;

        css->validateChains(skel);

        changed |= css->processChains(skel);
    }

    skel->qu->resumeRemoves();

    processHoriz(skel);

    return changed;
}

/*******************************************************************************

  Purpose - assumes that all corners with horizontal bisectors are at the same
  height(?), and that all faces that need to be merged, have been.

 *******************************************************************************/
void HeightCollision::processHoriz(Skeleton *skel)
{
    LinkedHashSet<Chain> chains;
    while (newHorizSet.size() > 0)
        chains.push_back(CoSitedCollision::buildChain2(newHorizSet.front(),
                    &newHorizSet));

    if (chains.size() == 0)
        return;

    /* if there are two lines of events at the same height (but different
     * lines), we need them to share their end points
     */
    IndirectLinkedHashSet<Corner *> intraface;
    for (LinkedHashSet<Chain>::iterator chainIt = chains.begin();
            chainIt != chains.end(); ++chainIt)
    {
        /* Naughty! But he does it in Java... they don't have const in Java, you
         * see, so things like this are unchecked.
         * -Tom
         */
        Chain &chain = (Chain &) *chainIt;

        vector<Edge *> priority;
        for (vector<Corner *>::iterator cIt = chain.chain.begin();
                cIt != chain.chain.end(); ++cIt)
        {
            Corner *c = *cIt;

            /* both edges are parallel---these are the only corners added to
             * newHoriz...
             */
            priority.push_back(c->nextL);
            priority.push_back(c->prevL);
        }

        /* find a set of coplanar edges that survive this transition in winners
         * (equal highest priority)
         */
        sort(priority.begin(), priority.end(),
                Skeleton::horizontalComparatorLT);
        IndirectLinkedHashSet<Edge *> winners;
        Edge *winner = priority.front(); priority.erase(priority.begin());
        winners.push_back(winner);
        while (!priority.empty() && Skeleton::horizontalComparator(winner, priority.front()) == 0)
        {
            winners.push_back(priority.front());
            priority.erase(priority.begin());
        }

        /* if first edge needs an additional corner---"if we're adding a cap at
         * the start" and "first isn't already an interface"
         */
        Corner *first = chain.chain.front();
        if (!winners.contains(first->prevL))
        {
            if (!intraface.contains(first->prevC)) // hasn't already been raised up by a previous chain
            {
                Corner *newFirst = GCNEW(Corner(Edge::collide(first->prevC,
                            first->z)));

                Edge *edgeArr[2];
                edgeArr[0] = first->prevL;
                edgeArr[1] = first->prevC->prevL;
                skel->output->addOutputSideTo(first->prevC, newFirst, 2,
                        edgeArr);
                Corner::replace(first->prevC, newFirst, skel);

                chain.chain.insert(chain.chain.begin(), newFirst);
                intraface.push_back(newFirst);
                first = newFirst;
            }
            else
                chain.chain.insert(chain.chain.begin(), first->prevC);
        }
        else
        {
            /* the edge before the first point is a winner, add it */
            chain.chain.insert(chain.chain.begin(), first = first->prevC);
        }
        Corner *last = chain.chain.at(chain.chain.size() - 1);
        /* if last edge needs an additional corner */
        if (!winners.contains(last->nextL))
        {
            if (!intraface.contains(last->nextC)) // hasn't already been raised up by a previous chain
            {
                Corner *newLast = GCNEW(Corner(Edge::collide(last->nextC,
                            last->z)));

                Edge *edgeArr[2];
                edgeArr[0] = last->nextL;
                edgeArr[1] = last->nextC->nextL;
                skel->output->addOutputSideTo(last->nextC, newLast, 2, edgeArr);
                Corner::replace(last->nextC, newLast, skel);

                chain.chain.push_back(newLast);
                intraface.push_back(newLast);
                last = newLast;
            }
            else
                chain.chain.push_back(last->nextC);
        }
        else
        {
            /* the edge after the last point is a winner, add it */
            chain.chain.push_back(last = last->nextC);
        }

        STLIterator<vector<Corner *> > stit(chain.chain);
        ConsecutivePairs<Corner *> pairIt(&stit, false);
        while (pairIt.hasNext())
        {
            tuple<Corner *, Corner *> *pair = pairIt.next();
            Corner *s = pair->get<0>();
            Corner *e = pair->get<1>();
            if (s->nextL == e->prevL)
                throw runtime_error("error! s->nextL == e->prevL");

            /* if this is he edge that spreads out all others */
            if (winners.contains(s->nextL))
            {
                if (s->nextL != winner)
                {
                    skel->output->merge(winner->start, s); // assumes start of edge forms part of its output
                }
                s->nextL->currentCorners.remove(e);
                s->nextL->currentCorners.remove(s);
            }
            else
            {
                /* this (section of this) edge ends at this height */
                s->nextL->currentCorners.remove(s);
                s->nextL->currentCorners.remove(e);

                Edge *edgeArr[2]; edgeArr[0] = s->nextL; edgeArr[1] = winner;
                skel->output->addOutputSideTo(s, e, 2, edgeArr);
            }

            skel->liveCorners.remove(s); // add in first and last below
            skel->liveCorners.remove(e);
        }

        skel->liveCorners.push_back(first);
        skel->liveCorners.push_back(last);

        winner->currentCorners.push_back(first);
        winner->currentCorners.push_back(last);

        first->nextC = last;
        last->prevC = first;
        first->nextL = winner;
        last->prevL = winner;

        for (vector<Corner *>::iterator it = chain.chain.begin();
                it != chain.chain.end(); ++it)
        {
            Corner *c = *it;
            if (c->nextL->currentCorners.size() == 0)
            {
                skel->liveEdges.remove(c->nextL);
            }
        }
    }

    /* no need to recalculate events---no faces added. wrong! any new
     * connectivity needs to be flagged as loop-of-two etc...
     */
    skel->qu->clearFaceEvents();

    /* See Skeleton::refindFaceEventsIfNeeded() for CloneConfirmIterator
     * shenanigans
     *
     * -Tom
     */
    IndirectLinkedHashSet<Corner *> bakLiveCorners;
    bakLiveCorners.insert(bakLiveCorners.end(), skel->liveCorners.begin(),
            skel->liveCorners.end());
    for (IndirectLinkedHashSet<Corner *>::iterator lcit =
            bakLiveCorners.begin(); lcit != bakLiveCorners.end(); ++lcit)
    {
        Corner *lc = *lcit;

        if (!skel->liveCorners.contains(lc))
        {
            cout << "*************DOES NOT CONTAIN!!!***********" << endl;
            continue;
        }
        skel->qu->addCorner(lc, this);
    }

    /* Just a check that my assumption that we don't /add/ to liveCorners
     * while iterating is correct.
     *
     * -Tom
     */
    for (IndirectLinkedHashSet<Corner *>::iterator lcit =
            skel->liveCorners.begin(); lcit != skel->liveCorners.end(); ++lcit)
    {
        Corner *lc = *lcit;
        if (!bakLiveCorners.contains(lc))
        {
            cout << "********liveCorners has but bakLiveCorners doesn't!: " <<
                lc << endl;
            assert(false);
        }
    }
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void HeightCollision::newHoriz(Corner *toAdd)
{
    newHorizSet.push_back(toAdd);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
std::ostream & operator<<(std::ostream &strm, const HeightCollision &h)
{
    return strm << "collisions at " << h.height;
}
