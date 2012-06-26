#ifndef IDENTITYLOOKUP_H
#define IDENTITYLOOKUP_H

#include "LinkedHashContainer.h"

template<typename T>
class IdentityLookup
{
    public:
        LinkedHashMap<T, T> map;

        void put(T &t)
        {
            map[t] = t;
        }

        T & get(T t)
        {
            if (!map.containsKey(t))
                put(t);

            return map[t];
        }
};

#endif
