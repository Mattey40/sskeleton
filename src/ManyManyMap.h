#ifndef MANYMANYMAP_H
#define MANYMANYMAP_H

#include "Cache.h"
#include "MultiHashMap.h"

/**
 * When you have a many -> many correspondance, you want this class!
 */
template<
    typename A,
    typename B,
    typename ABMapT = MultiHashMap<A, B>,
    typename BAMapT = MultiHashMap<B, A>
>
class ManyManyMap
{
    public:
        void addForwards(A from, B to)
        {
            forwards.put(from, to);
            backwards.put(to, from);
        }

        std::vector<B> getNext(const A &from)
        {
            return forwards.get(from);
        }

        std::vector<A> getPrev(const B &to)
        {
            return backwards.get(to);
        }

        /**
         * Given a specified cache will convert everything that is an A to C.
         * That is the element type in the from will go from A to C.
         */
        //template<typename C>
        //class ConvertInputCollection
        //{
        //    public:
        //        ManyManyMap<A, B, ABMapT, BAMapT> *mmmap;
        //        IndirectCache<A, std::vector<C> > converter;

        //        ConvertInputCollection(ManyManyMap *mmmap,
        //                Cache<A, std::vector<C> > converter) :
        //            mmmap(mmmap), converter(converter) { }

        //        ManyManyMap<C, B> get(void)
        //        {
        //            ManyManyMap<C, B> out;

        //            for (typename MultiHashMap<A, B>::MapT::key_iterator esit =
        //                mmmap->forwards.map.begin();
        //                esit != mmmap->forwards.map.end(); ++esit)
        //            {
        //                std::vector<C> &cVec = converter.get(esit->first);
        //                for (typename std::vector<C>::iterator cIt =
        //                        cVec.begin(); cIt != cVec.end(); ++cIt)
        //                {
        //                    out.forwards.putAll(*cIt, cVec.begin(), cVec.end(),
        //                            true);
        //                }
        //            }

        //            for (typename MultiHashMap<B, A>::MapT::key_iterator esit =
        //                mmmap->backwards.map.begin();
        //                esit != mmmap->backwards.map.end(); ++esit)
        //            {
        //                std::vector<A> &aVec = esit->second;
        //                for (typename std::vector<A>::iterator aIt =
        //                        aVec.begin(); aIt != aVec.end(); ++aIt)
        //                {
        //                    std::vector<C> &cVec = converter.get(*aIt);
        //                    out.backwards.putAll(esit->first, cVec.begin(),
        //                            cVec.end(), true);
        //                }
        //            }

        //            return out;
        //        }
        //};

    private:
        ABMapT forwards;
        BAMapT backwards;
};

template<typename A, typename B>
class IndirectManyManyMap: public ManyManyMap<
                                        A,
                                        B,
                                        IndirectMultiHashMap<A, B>,
                                        IndirectMultiHashMap<B, A>
                                  >
{
};

#endif
