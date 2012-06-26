#include "CornerClone.h"

using namespace std;

/*******************************************************************************

  Purpose - Clones a new set of corners, edges.

  Edges machines are not cloned.

  input---the input to be cloned. Result dumped into variables.

 *******************************************************************************/
CornerClone::CornerClone(LoopL<Corner> *input)
{
    output = GCNEW(LoopL<Corner>());

    class CornerCache: public IndirectCache<Corner *, Corner *>
    {
        public:
            virtual Corner * create(Corner *&i)
            {
                return GCNEW(Corner(*i));
            }
    };
    CornerCache cornerCache;

    class EdgeCache: public IndirectCache<Edge *, Edge *>
    {
        public:
            CornerCache &cornerCache;

            EdgeCache(CornerCache &cornerCache) : cornerCache(cornerCache) { }
            virtual Edge * create(Edge *&i)
            {
                Edge *edge = GCNEW(Edge(cornerCache.get(i->start),
                        cornerCache.get(i->end)));

                edge->setAngle(i->getAngle());
                edge->machine = i->machine; // nextL is null when we have a non root global

                for (IndirectLinkedHashSet<Corner *>::iterator it =
                        i->currentCorners.begin();
                        it != i->currentCorners.end(); ++it)
                {
                    Corner *c = *it;
                    edge->currentCorners.push_back(cornerCache.get(c));
                }

                return edge;
            }
    };

    EdgeCache edgeCache(cornerCache);

    for (size_t i = 0; i < input->v.size(); i++)
    {
        Loop<Corner> *inputLoop = input->v[i];

        Loop<Corner> *loop = GCNEW(Loop<Corner>());

        output->v.push_back(loop);

        for (Loop<Corner>::LoopIterator lit = inputLoop->iterator();
                lit.hasNext();)
        {
            Corner *current = lit.next();

            Corner *s = cornerCache.get(current);
            Corner *e = cornerCache.get(current->nextC);

            /* one edge may have two segments, but the topology will not change
             * between old and new, so we may store the leading edge to match
             * segments
             */
            nOSegments.put(s, current);
            nOCorner.put(s, current);

            Edge *edge = edgeCache.get(current->nextL);

            loop->append(s);
            s->nextC = e;
            e->prevC = s;
            s->nextL = edge;
            e->prevL = edge;
        }
    }
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
bool CornerClone::addSegment(void)
{
    return true;
}

/*******************************************************************************
 
 Purpose - 

 *******************************************************************************/
bool CornerClone::addCorner(void)
{
    return true;
}
