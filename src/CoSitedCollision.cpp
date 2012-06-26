#include "CoSitedCollision.h"

using namespace boost;
using namespace std;

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
CoSitedCollision::CoSitedCollision(Point3D loc, EdgeCollision *ec,
        HeightCollision *parent) :
    loc(loc), parent(parent)
{
    add(ec);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void CoSitedCollision::add(EdgeCollision *ec)
{
    edges.push_back(ec);
}

/*******************************************************************************

  Purpose - (findChains) Returns true if valid chains found at this site

 *******************************************************************************/
double chainComparatorHeight;
Point3D chainComparatorLoc;
bool chainComparator(const Chain &lhs, const Chain &rhs)
{
    Corner *c1 = lhs.chain[0];
    Corner *c2 = rhs.chain[0];

    /* except for the first and last point
     * chain's non-start/end points are always at the position of the collision;
     * so to find the angle of the first edge at the specified height, we
     * project its start coordinate the desired height and take the angle
     * relative to the collision
     * !could speed up with a chain-class that caches this info!
     */
    Tuple3D p1 = Edge::collide(c1, chainComparatorHeight);
    Tuple3D p2 = Edge::collide(c2, chainComparatorHeight);

    p1.sub(chainComparatorLoc);
    p2.sub(chainComparatorLoc);

    /* start/end line is (+-)Pi */
    return atan2(p1.y, p1.x) < atan2(p2.y, p2.x);
}
bool CoSitedCollision::findChains(Skeleton *skel)
{
    chains.clear();

    /* remove duplicate edges */
    IndirectLinkedHashSet<Edge *> allEdges;
    IndirectLinkedHashSet<EdgeCollision *>::iterator ecIt;
    for (ecIt = edges.begin(); ecIt != edges.end(); ++ecIt)
    {
        allEdges.push_back((*ecIt)->a);
        allEdges.push_back((*ecIt)->b);
        allEdges.push_back((*ecIt)->c);
    }

    vector<Edge *> edgesToRemove;
    IndirectLinkedHashSet<Edge *>::iterator eit;
    for (eit = allEdges.begin(); eit != allEdges.end(); ++eit)
        if (!skel->liveEdges.contains(*eit))
            edgesToRemove.push_back(*eit);
    for (size_t i = 0; i < edgesToRemove.size(); i++)
        allEdges.remove(edgesToRemove[i]);

    if (allEdges.size() < 3)
        return false;
        /* fixme: why do we add all starting corners? just to filter them out
         * later?
         */
    IndirectLinkedHashSet<Corner *> edgeStarts;
    for (eit = allEdges.begin(); eit != allEdges.end(); ++eit)
    {
        Edge *e = *eit;
        IndirectLinkedHashSet<Corner *>::iterator cit;
        for (cit = e->currentCorners.begin(); cit != e->currentCorners.end();
                ++cit)
        {
            Corner *c = *cit;
            if (c->nextL == e)
                edgeStarts.push_back(c);
        }
    }

    while (!edgeStarts.empty())
    {
        Corner *start = *edgeStarts.begin();
        Chain chain = buildChain2(start, &edgeStarts);
        chains.push_back(chain);
    }

    edgeStarts.clear();
    for (size_t i = 0; i < chains.size(); i++)
        if (chains[i].chain.size() > 1)
            edgeStarts.insert(edgeStarts.end(), chains[i].chain.begin(),
                    chains[i].chain.end());

    vector<Chain>::iterator chit;
    for (chit = chains.begin(); chit < chains.end(); )
    {
        if (chit->chain.size() == 1)
        {
            /* first corner of edge is not necessarily the corner of the edge
             * segment bounding the collision
             */
            Corner *s = chit->chain[0];
            Corner *found = EdgeCollision::findCorner(s->nextL, loc, skel);

            /* fixme: because we (strangely) add all the chain starts above, we
             * can just check if it's unchanged...
             */
            if (found == s && !edgeStarts.contains(s))
            {
                // chain.chain.clear();
                // chain.chain.add(found);
            }
            else
            {
                chit = chains.erase(chit);
                continue;
            }
        }
        ++chit;
    }

    /* while still no-horizontals in chains (there may be when dealing with
     * multiple sites at one height), process chains to a counter clockwise
     * order
     */
    if (chains.size() > 1) // size == 1 may have parallels in it (run away!)
    {
        chainComparatorHeight = loc.z;
        chainComparatorLoc = loc;
        sort(chains.begin(), chains.end(), chainComparator);
    }

    return true;
}

/*******************************************************************************

  Purpose - If another collision has been evaluated at the same height, this
  method checks for any changes in the Corners involved in a skeleton. This is a
  problem when several collisions at the same height occur against one smash
  edge.

  If the chain has length > 1, then the lost corner can be recovered using the
  following corner (the corners central to the collision, every one after the
  first will all remain valid at one height).

  If the chain has length 1, we're in for a bit of a trek.

  We can skip the finding edges part if the current height only has one
  collision?

 *******************************************************************************/
void CoSitedCollision::validateChains(Skeleton *skel)
{
    /* in case an edge has been removed by a previous event at this height */
    vector<Chain>::iterator c;
    for (c = chains.begin(); c != chains.end(); ++c)
    {
        if (c->loop)
            continue; // nothing to do here

        if (c->chain.size() > 1) // previous
        {
            c->chain.erase(c->chain.begin());
            c->chain.insert(c->chain.begin(), c->chain[0]->prevC);
        }
        else // smash edge, search for correct corner (edges not in liveEdges, removed next)
        {
            /* This covers the "corner situation" where a concave angle creates
             * another loop with its point closer to the target, than the
             * original loop.
             *
             * It may well break down with lots of adjacent sides
             */

            Corner *s = c->chain[0];
            Edge *e = s->nextL;

            Line3D projectionLine(loc, e->direction());

            LinearForm3D ceiling(0, 0, 1, -loc.z);
            /* project start onto line of collisions above smash edge */
            try
            {
                Tuple3D start = e->linearForm.collide(s->prevL->linearForm,
                        ceiling);

                /* line defined using collision point, so we're finding the line
                 * before 0
                 */
                double targetParam = 0;

                /* we should only end with start if it hasn't been elevated
                 * yet
                 */
                Corner *bestPrev = s;
                /* ignore points before start (but allow the first point to
                 * override start!)
                 */
                double bestParam = projectionLine.projectParam(start) - 0.001;

                /* parameterize each corner in e's currentCorners by the line */
                IndirectLinkedHashSet<Corner *>::iterator rit;
                for (rit = e->currentCorners.begin();
                        rit != e->currentCorners.end(); ++rit)
                {
                    Corner *r = *rit;

                    if (r->nextL == e)
                    {
                        /* parameterize */
                        Tuple3D rOnHigh = abs(r->z - loc.z) < 0.001 ? *r :
                            ceiling.collide(r->prevL->linearForm,
                                    r->nextL->linearForm);
                        double param = projectionLine.projectParam(rOnHigh);
                        /* if this was the previous (todo: does this want a
                         * tolerance on < targetParam? why not?)
                         */
                        if (param > bestParam && param <= targetParam)
                        {
                            bestPrev = r;
                            bestParam = param;
                        }
                    }
                }

                c->chain.erase(c->chain.begin());
                c->chain.insert(c->chain.begin(), bestPrev);

                /* might have formed a loop */
                c->loop = c->chain[c->chain.size() - 1]->nextC ==
                    c->chain[0];
            }
            catch (std::exception &except)
            {
#ifdef __GNUC__
                void *array[50];
                size_t size = backtrace(array, 50);
#endif
                cerr << "didn't like colliding " << *e << " and " << *s->prevL
                    << endl;
                cerr << "exception: " << except.what() << endl;
#ifdef __GNUC__
                backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
            }
        }
    }

    LinkedHashMap<Edge *, Corner *> edgeToCorner;
    for (c = chains.begin(); c != chains.end(); ++c)
    {
        Chain *cc = &*c;
        vector<Corner *>::iterator cit;
        for (cit = cc->chain.begin(); cit != cc->chain.end(); ++cit)
        {
            Corner *c = *cit;
            edgeToCorner[c->nextL] = c;
        }
    }

    /* Find valid triples ~ now topology is as it will be before evaluation,
     * we can check that the input edge triplets still have two consecutive
     * edges.
     */
    IndirectLinkedHashSet<Edge *> validEdges;
    for (IndirectLinkedHashSet<EdgeCollision *>::iterator ecit = edges.begin();
            ecit != edges.end(); ++ecit)
    {
        EdgeCollision *ec = *ecit;

        /* todo: adjancent pairs may not be parallel! */
        Corner *c1 = edgeToCorner.containsKey(ec->a) ?
            edgeToCorner[ec->a] : NULL;
        Corner *c2 = edgeToCorner.containsKey(ec->b) ?
            edgeToCorner[ec->b] : NULL;
        Corner *c3 = edgeToCorner.containsKey(ec->c) ?
            edgeToCorner[ec->c] : NULL;

        if (hasAdjacent(c1, c2, c3))
        {
            if (    skel->liveEdges.contains(ec->a) &&
                    skel->liveEdges.contains(ec->b) &&
                    skel->liveEdges.contains(ec->c))
            {
                validEdges.push_back(ec->a);
                validEdges.push_back(ec->b);
                validEdges.push_back(ec->c);
            }
        }
    }

    vector<Chain> chainOrder(chains);

    /* remove parts of chains that aren't a valid triple */
    for (vector<Chain>::iterator ccit = chainOrder.begin();
            ccit != chainOrder.end(); ++ccit)
    {
        Chain &cc = *ccit;

        /* remove and split */
        vector<Chain>::iterator ccloc = find(chains.begin(), chains.end(), cc);
        vector<Chain> v = cc.removeCornersWithoutEdges(validEdges);
        chains.insert(ccloc, v.begin(), v.end());
    }

    /* kill 0-length chains */
    for (vector<Chain>::iterator ccit = chains.begin();
            ccit < chains.end(); )
    {
        if (ccit->chain.size() == 0)
            ccit = chains.erase(ccit);
        else
            ++ccit;
    }
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
bool CoSitedCollision::processChains(Skeleton *skel)
{
    if (moreOneSmashEdge()) // no test example showing this is required
        return false;

    IndirectLinkedHashSet<Corner *> allCorners;
    for (vector<Chain>::iterator cc = chains.begin(); cc != chains.end(); ++cc)
        allCorners.insert(allCorners.end(), cc->chain.begin(), cc->chain.end());

    /* after all the checks, if there are less than three faces involved, it's
     * not a collision anymore
     */
    if (allCorners.size() < 3)
        return false;

    for (vector<Chain>::iterator cit = chains.begin(); cit < chains.end(); )
    {
        Chain &chain = *cit;

        STLIterator<vector<Corner *> > stit(chain.chain);
        ConsecutivePairs<Corner *> pit(&stit, chain.loop);
        while (pit.hasNext())
        {
            tuple<Corner *, Corner *> *p = pit.next();
            EdgeCollision::processConsecutive(&loc, p->get<0>(), p->get<1>(),
                    skel);
        }

        /* remove the middle faces in the loop from the list of live corners,
         * liveEdges if there are no more live corners, and the liveCorners list
         */
        if (chain.chain.size() >= 3)
        {
            STLIterator<vector<Corner *> > stit(chain.chain);
            ConsecutiveTriples<Corner *> tit(&stit, chain.loop);
            while (tit.hasNext())
            {
                Edge *middle = tit.next()->get<1>()->nextL;

                /* face no longer referenced, remove from list of live edges */
                if (middle->currentCorners.empty())
                    skel->liveEdges.remove(middle);
            }
        }

        if (chain.loop)
            cit = chains.erase(cit);
        else
            ++cit;
    }

    /* was entirely closed loops */
    if (chains.empty())
        return true;

    /* connect end of previous chain, to start of next */

    /* in case we are colliding against a smash (no-corner/split event)-edge,
     * we cache the next-corner before any alterations
     */
    LinkedHashMap<Corner *, Corner *> aNext;
    for (vector<Chain>::iterator it = chains.begin(); it != chains.end(); ++it)
    {
        Chain &chain = *it;
        Corner *c = chain.chain.at(chain.chain.size() - 1);
        aNext[c] = c->nextC;
    }

    /* process intra-chain collisions (non-consecutive edges) */
    STLIterator<vector<Chain> > stit(chains);
    ConsecutivePairs<Chain> cp(&stit, true);
    while (cp.hasNext())
    {
        tuple<Chain, Chain> *adjacentChains = cp.next();

        vector<Corner *> first = adjacentChains->get<0>().chain;
        Corner *a = first.at(first.size() - 1);
        Corner *b = adjacentChains->get<1>().chain.at(0);
        EdgeCollision::processJump(&loc, a, aNext[a], b, skel, parent);
    }

    return true;
}

/*******************************************************************************

  Purpose - Is this actually needed in any of the examples?

 *******************************************************************************/
bool CoSitedCollision::moreOneSmashEdge(void)
{
    /* if two chains have length one, this is not a valid collision point */
    int oneCount = 0;
    for (vector<Chain>::iterator ch = chains.begin(); ch != chains.end(); ++ch)
        if (ch->chain.size() == 1)
            oneCount++;
    if (oneCount > 1)
        return false;

    return oneCount > 1; // hang on, won't this always return false?! WTF. -Tom
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
bool CoSitedCollision::hasAdjacent(Corner *a, Corner *b, Corner *c)
{
    if (a == NULL || b == NULL || c == NULL)
        return false;

    if (a->nextC == b || a->nextC == c) // todo: speedup by puttin consec in a, b always?
        return true;
    if (b->nextC == c || b->nextC == a)
        return true;
    if (c->nextC == a || c->nextC == b)
        return true;

    return false;
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
Chain CoSitedCollision::buildChain2(Corner *start,
        IndirectLinkedHashSet<Corner *> *input)
{
    vector<Corner *> chain;

    /* Check backwards */
    Corner *a = start;
    while (input->contains(a))
    {
        chain.insert(chain.begin(), a);
        input->remove(a);
        a = a->prevC;
    }

    /* check forwards */
    a = start->nextC;
    while (input->contains(a))
    {
        chain.push_back(a);
        input->remove(a);
        a = a->nextC;
    }

    return Chain(chain);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
std::ostream & operator<<(std::ostream &strm, const CoSitedCollision &c)
{
    strm << "{";
    for (IndirectLinkedHashSet<EdgeCollision *>::iterator it = c.edges.begin();
            it != c.edges.end(); ++it)
        strm << *it;
    return strm << "}";
}
