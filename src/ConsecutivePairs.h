#ifndef CONSECUTIVEPAIRS_H
#define CONSECUTIVEPAIRS_H

#include <boost/tuple/tuple.hpp>
#include <vector>
#include "Iterator.h"

/**
 * Iterator that returns all consecutive triples<H><H><H><H><H><H><H>pairs in a
 * list.
 *
 * If size is 1 and we're looping we return one pair of (element 1, element 1).
 */
template<typename T>
class ConsecutivePairs: public Iterator<boost::tuple<T, T> >
{
    public:
        /* inputItr -- an iterator to the list to return pairs from
         * loop -- is it cyclic? (do we return {end-1, end, start} etc...)
         */
        ConsecutivePairs(Iterator<T> *inputItr, bool loop) :
            loop(loop), a(0), n(NULL)
        {
            while (inputItr->hasNext())
                input.push_back(inputItr->next());
            size = input.size();
        }

        virtual ~ConsecutivePairs(void)
        {
            delete n;
        }

        virtual bool hasNext(void)
        {
            if (size == 1 && !loop)
                return false;
            return a >= 0;
        }

        virtual boost::tuple<T, T> * next(void)
        {
            delete n;
            n = new boost::tuple<T, T>(
                    *input.at(a % size),
                    *input.at((a + 1) % size));

            a++;
            if (!loop && a == size - 1)
                a = -1; //end
            else if (loop && a == size)
                a = -1; //end

            return n;
        }

    private:
        std::vector<T *> input;
        bool loop;
        int a;
        int size;
        boost::tuple<T, T> *n;
};

#endif
