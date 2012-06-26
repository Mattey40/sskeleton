#ifndef GC_H
#define GC_H

#include <iostream>
#include <list>

/* This isn't really a proper garbage collector, but it suffices for our
 * purpose. This program was ported from Java, where of course they have a nice
 * garbage collector; inevitably the author ended up allocating memory all over
 * the place and we don't really know what belongs to who; multiple objects
 * seem to have ownership of the same thing and it's all very hairy.
 *
 * The easiest thing to do seemed to be this hack.
 *
 * As I say, it's not a proper garbage collector. We simply record `new'
 * allocations, and at some point we call `collect' and they're all deleted.
 *
 * This works well for the use case of this program, which is to set up a
 * loop of edges, run the skeleton algorithm on them, then return results. At
 * this point we can call `collect'.
 *
 * To use, we replace calls to `new' with GCNEW(), as per the macro below.
 *
 * Needless to say, you shouldn't try to delete a pointer allocated with GCNEW
 * with a manual delete in the destructor.
 *
 * You'll note the static variables, template magic, and global GC variable
 * declared extern in this header. Tread carefully.
 *
 * It's application is limited and anybody extending the scope of this program
 * or garbage collector should fully understand what they are doing.
 *
 * -Tom
 */

#define GCNEW(x) (GC.cnew(new x))

class CollectorBase
{
    public:
        virtual void collect(void) = 0;
};

template<typename T>
class Collector: public CollectorBase
{
    public:
        std::list<T *> ptrs;

        Collector(std::list<CollectorBase *> *collectors)
        {
            collectors->push_back(this);
            std::cout << "#collectors: " << collectors->size() << std::endl;
        }

        T * cnew(T *t)
        {
            ptrs.push_back(t);
            return t;
        }

        virtual void collect(void)
        {
            while (!ptrs.empty())
            {
                delete ptrs.front();
                ptrs.pop_front();
            }
        }
};

class GCollector
{
    public:
        std::list<CollectorBase *> collectors;

        GCollector(void);

        template<typename T>
        T * cnew(T *t)
        {
            static Collector<T> ctor(&collectors);

            return ctor.cnew(t);
        }

        void collect(void)
        {
            for (std::list<CollectorBase *>::iterator it = collectors.begin();
                    it != collectors.end(); it++)
                (*it)->collect();
        }

    private:
};

extern GCollector GC;

#endif
