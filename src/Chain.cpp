#include "Chain.h"

using namespace std;

Chain * Chain::DELOOP = new Chain(std::vector<Corner *>(), false);

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
Chain::Chain(std::vector<Corner *> chain) :
    chain(chain)
{
    loop = (chain.at(chain.size() - 1)->nextC == chain.at(0));
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
Chain::Chain(std::vector<Corner *> chain, bool loop) :
    chain(chain), loop(loop)
{
}

/*******************************************************************************

  Purpose - index---first element of the split

 *******************************************************************************/
Chain * Chain::split(int index)
{
    /* decompose a loop a the given index */
    if (loop)
    {
        vector<Corner *> nc;
        vector<Corner *>::iterator it;

        it = chain.begin();
        advance(it, index);

        nc.insert(nc.end(), it, chain.end());
        nc.insert(nc.end(), chain.begin(), it);
        loop = false;
        chain = nc;
        return DELOOP;
    }

    /* first element already split */
    if (index == 0)
        return NULL;

    vector<Corner *> nc;
    vector<Corner *>::iterator it;

    it = chain.begin();
    advance(it, index);

    nc.insert(nc.end(), chain.begin(), it);

    vector<Corner *> sublist(it, chain.end());
    chain = sublist;

    return new Chain(nc);
}

/*******************************************************************************

  Purpose - Returns list of new chains, in order, that should be appended to the
  master list before this chain.

 *******************************************************************************/
std::vector<Chain> Chain::removeCornersWithoutEdges(
        IndirectLinkedHashSet<Edge *> liveEdges)
{
    vector<Chain> newChains;

    for (;;)
    {
        for (int i = 0; i < (int) chain.size(); i++)
        {
            Corner *c = chain[i];
            if (!liveEdges.contains(c->nextL))
            {
                Chain *n = split(i);

                chain.erase(chain.begin()); // removed the specified element

                if (n == DELOOP)
                {
                    /* restart, next time first element will split with no
                     * effect
                     */
                    vector<Chain> v = removeCornersWithoutEdges(liveEdges);
                    newChains.insert(newChains.end(), v.begin(), v.end());
                    return newChains;
                }

                if (n != NULL)
                    newChains.push_back(*n);

                if (n != DELOOP)
                    delete n;

                i = -1; // process element 0 next time
            }
        }
        /* iterated entire chain - done! */
        break;
    }
    return newChains;
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
bool Chain::operator==(const Chain &other) const
{
    return (loop == other.loop && chain == other.chain);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
bool Chain::operator!=(const Chain &other) const
{
    return !(*this == other);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
size_t hash_value(const Chain &s)
{
    return (size_t) &s;
}
