#ifndef CACHE_H
#define CACHE_H

#include "LinkedHashContainer.h"

template<typename I, typename O, typename MapType = LinkedHashMap<I, O> >
class Cache
{
    public:
        MapType cache;

        virtual ~Cache(void) { }

        virtual O get(I &in)
        {
            if (!cache.containsKey(in))
                cache[in] = create(in);
            return cache[in];
        }

        virtual O create(I &i) = 0;

        virtual void put(I &start, O &start0)
        {
            cache[start] = start0;
        }
};

template<typename I, typename O>
class IndirectCache: public Cache<
                             I, 
                             O, 
                             IndirectLinkedHashMap<I, O> 
                         >
{
};

#endif
