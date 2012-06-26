#ifndef LOOP_H
#define LOOP_H

#include <cstddef>
#include <vector>
#include "GC.h"
#include "Iterator.h"

template<typename T>
class Loopable
{
    public:
        Loopable(T *me) :
            me(me), next(NULL), prev(NULL) { }
        T * get(void) { return me; }
        Loopable<T> * getNext(void) { return next; }
        Loopable<T> * getPrev(void) { return prev; }
        void setNext(Loopable<T> *s) { next = s; }
        void setPrev(Loopable<T> *s) { prev = s; }

        friend bool operator<(const Loopable &lhs, const Loopable &rhs)
        {
            return *lhs.me < *rhs.me;
        }

    private:
        T *me;
        Loopable<T> *next, *prev;
};

template<typename T>
class Loop
{
    public:
        class LoopableIterator: public Iterator<Loopable<T> >
        {
            public:
                Loop<T> *parent;
                Loopable<T> *s, *n;

                LoopableIterator(Loop<T> *parent) :
                    parent(parent), s(parent->start), n(NULL) { }

                virtual bool hasNext(void)
                {
                    if (s == NULL)
                        return false;
                    if (n == NULL)
                        return true;
                    return n != parent->start;
                }

                virtual Loopable<T> * next(void)
                {
                    if (n == NULL)
                        n = parent->start;

                    Loopable<T> *out = n;
                    n = n->getNext();
                    return out;
                }
        };

        class LoopIterator: public Iterator<T>
        {
            public:
                LoopableIterator lit;

                LoopIterator(Loop<T> *parent) :
                    lit(parent) { }

                virtual bool hasNext(void) { return lit.hasNext(); }
                virtual T * next(void) { return lit.next()->get(); }
        };

        Loopable<T> *start;

        Loop(void) :
            start(NULL) { }

        int count(void)
        {
            int count = 0;

            LoopIterator it = iterator();
            while (it.hasNext())
            {
                count++;
                it.next();
            }

            return count;
        }

        void removeAll(void) { start = NULL; }

        Loopable<T> * append(T *a)
        {
            if (start == NULL)
            {
                start = GCNEW(Loopable<T>(a));
                start->setNext(start);
                start->setPrev(start);
                return start;
            }
            else
            {
                Loopable<T> *toAdd = GCNEW(Loopable<T>(a));

                toAdd->setPrev(start->getPrev());
                toAdd->setNext(start);
                start->getPrev()->setNext(toAdd);
                start->setPrev(toAdd);

                return toAdd;
            }
        }

        Loopable<T> * addAfter(Loopable<T> *loopable, T *a)
        {
            Loopable<T> *n = GCNEW(Loopable<T>(a));
            n->setPrev(loopable);
            n->setNext(loopable->getNext());
            n->getPrev()->setNext(n);
            n->getNext()->setPrev(n);
            return n;
        }

        void remove(T *r)
        {
            Loopable<T> *togo = find(r);
            remove(togo);
        }

        void remove(Loopable<T> *togo)
        {
            if (togo == start)
            {
                if (togo->getPrev() == togo)
                    start = NULL;
                else
                    start = togo->getPrev();
            }

            togo->getPrev()->setNext(togo->getNext());
            togo->getNext()->setPrev(togo->getPrev());
        }

        Loopable<T> * find(T *remove)
        {
            Loopable<T> *n = start;

            while (n->getNext() != start)
            {
                if (*n->get() == *remove)
                    return n;

                n = n->getNext();
            }

            if (*n->get() == *remove)
                return n;

            return NULL;
        }

        Loopable<T> * getFirstLoopable(void) { return start; }

        T * getFirst(void)
        {
            if (start == NULL)
                return NULL;

            return start->get();
        }

        LoopableIterator loopableIterator(void)
        {
            return LoopableIterator(this);
        }

        LoopIterator iterator(void)
        {
            return LoopIterator(this);
        }

        /**
         * Appears to have bug---first element stays at the beginning. Not going
         * to fix bug as it is in the original code too.
         * -Tom
         */
        void reverse(void)
        {
            if (start == NULL)
                return;

            Loopable<T> *m = start;

            do
            {
                Loopable<T> *tmp = m->getNext();
                m->setNext(m->getPrev());
                m->setPrev(tmp);

                m = m->getPrev(); // reversed ;)
            }
            while (m != start);
        }

    private:
};

/**
 * LoopL context - an item and it's location in a loop of loops
 */
template<typename T>
class LContext
{
    public:
        Loopable<T> *loopable;
        Loop<T> *loop;

        LContext(Loopable<T> *loopable, Loop<T> *loop) :
            loopable(loopable), loop(loop) { }
        T * get(void) { return loopable->get(); }
};

/**
 * A loop of loops, with an iterator for the contained primitive (corners!)
 */
template<typename T>
class LoopL
{
    typedef typename Loop<T>::LoopIterator LoopIterT;
    typedef typename Loop<T>::LoopableIterator LoopableIterT;

    public:
        /**
         * Iterate over all individual elements (of the sub-loops). Original
         * Java code had some complicated generics stuff with ItIt but we won't
         * bother trying to port that (can't see how you'd do it with C++
         * anyway).
         */
        class EIterator: public Iterator<T>
        {
            public:
                EIterator(LoopL<T> *parent) :
                    parent(parent), loopIdx(0), it2(NULL), n(NULL)
                {
                    findNext();
                }
                
                virtual ~EIterator(void)
                {
                    delete it2;
                }

                virtual bool hasNext(void)
                {
                    return n != NULL;
                }

                virtual T * next(void)
                {
                    T *out = n;
                    findNext();
                    return out;
                }

            private:
                LoopL<T> *parent;
                size_t loopIdx;
                LoopIterT *it2;
                T *n;

                void findNext(void)
                {
                    if (it2 != NULL && it2->hasNext())
                        n = it2->next();
                    else if (loopIdx < parent->v.size())
                    {
                        delete it2;
                        it2 = new LoopIterT(parent->v[loopIdx]);
                        loopIdx++;
                        findNext();
                    }
                    else
                        n = NULL;
                }
        };

        class ContextIt: public Iterator<LContext<T> >
        {
            public:
                ContextIt(LoopL<T> *parent) :
                    parent(parent), loopIdx(0), it2(NULL), loopable(NULL),
                    loop(NULL)
                {
                    findNext();
                }

                virtual ~ContextIt(void)
                {
                    delete it2;
                }

                virtual bool hasNext(void)
                {
                    return it2 != NULL;
                }

                /**
                 * Warning! Need to delete after you've finished with it
                 */
                virtual LContext<T> * next(void)
                {
                    LContext<T> *out = GCNEW(LContext<T>(loopable, loop));
                    findNext();
                    return out;
                }

            private:
                LoopL<T> *parent;
                size_t loopIdx;
                LoopableIterT *it2;

                // next values to return
                Loopable<T> *loopable;
                Loop<T> *loop;

                void findNext(void)
                {
                    using namespace std;
                    if (loopIdx > parent->v.size())
                        return; // finished!
                    else if (it2 != NULL && // start
                            it2->hasNext())
                        loopable = it2->next();
                    else if (loopIdx < parent->v.size())
                    {
                        delete it2;
                        loop = parent->v[loopIdx];
                        it2 = new LoopableIterT(loop);
                        loopIdx++;
                        findNext();
                    }
                    else
                    {
                        delete it2;
                        it2 = NULL;
                    }
                }
        };

        std::vector<Loop<T> *> v;

        LoopL(void) { }

        LoopL(Loop<T> *fromPoints) { v.push_back(fromPoints); }

        //~LoopL(void)
        //{
        //    for (size_t i = 0; i < v.size(); i++)
        //        delete v[i];
        //}

        EIterator eIterator(void) { return EIterator(this); }

        ContextIt getCIterator(void) { return ContextIt(this); }

        int count(void)
        {
            int c = 0;
            for (size_t i = 0; i < v.size(); i++)
                c += v[i]->count();

            return c;
        }
};

#endif
