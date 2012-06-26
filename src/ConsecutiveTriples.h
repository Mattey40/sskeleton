#ifndef CONSECUTIVETRIPLES_H
#define CONSECUTIVETRIPLES_H

#include <boost/tuple/tuple.hpp>
#include <vector>
#include "Iterator.h"

/* Iterator that returns all consecutuve triples in a list
 */

template<typename T>
class ConsecutiveTriples: public Iterator<boost::tuple<T, T, T> >
{
    public:
        /* inputItr -- an iterator to the list to return triples from
         * loop -- is it cyclic? (do we return {end-1, end, start} etc....)
         */
        ConsecutiveTriples(Iterator<T> *inputItr, bool loop) :
            loop(loop), a(0), n(NULL)
        {
            while (inputItr->hasNext())
                input.push_back(inputItr->next());
            size = input.size();
        }

        virtual ~ConsecutiveTriples(void)
        {
            delete n;
        }

        virtual bool hasNext(void)
        {
            return a >= 0;
        }

        virtual boost::tuple<T, T, T> * next(void)
        {
            delete n;
            n = new boost::tuple<T, T, T>(
                    *input.at(a % size),
                    *input.at((a + 1) % size),
                    *input.at((a + 2) % size));

            a++;
            if (!loop && a == size - 2)
                a = -1; // end
            else if (loop && a == size)
                a = -1; // end

            return n;
        }

    private:
        std::vector<T *> input;
        bool loop;
        int a;
        int size;
        boost::tuple<T, T, T> *n;
};

#endif
