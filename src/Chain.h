#ifndef CHAIN_H
#define CHAIN_H

#include <vector>
#include "Corner.h"
#include "Edge.h"
#include "LinkedHashContainer.h"

/**
 * A Chain is a set of edges colliding at a point. Described by the first
 * corner of each face.
 */

class Chain
{
    public:
        std::vector<Corner *> chain;
        bool loop;

        Chain(std::vector<Corner *> chain);
        Chain(std::vector<Corner *> chain, bool loop);
        std::vector<Chain> removeCornersWithoutEdges(
                IndirectLinkedHashSet<Edge *> liveEdges);
        bool operator==(const Chain &other) const;
        bool operator!=(const Chain &other) const;

    private:

        /* marker for having degraded from a loop to a list */
        static Chain *DELOOP;

        Chain * split(int index);
};

size_t hash_value(const Chain &s);

#endif
