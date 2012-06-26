#ifndef MULTIHASHMAP_H
#define MULTIHASHMAP_H

#include <vector>
#include "LinkedHashContainer.h"

/**
 * HashMap backed multi-item hash. E.g. a list for every entry in the hash
 * table.
 *
 * Bit different from stl multimap! -Tom
 */
template<
    typename A,
    typename B,
    typename MapType = LinkedHashMap<A, std::vector<B> >
>
class MultiHashMap
{
    public:
        typedef MapType MapT;

        MapType map;
        static std::vector<B> emptyVector;

        void addEmpty(const A &a)
        {
            if (!map.containsKey(a))
                map[a] = std::vector<B>();
        }

        int size(void)
        {
            return map.size();
        }

        bool empty(void)
        {
            return map.empty();
        }

        bool containsKey(const A &a)
        {
            return map.containsKey(a);
        }

        std::vector<B> & get(const A &a)
        {
            if (map.containsKey(a))
                return map[a];
            else
            {
                emptyVector.clear();
                return emptyVector;
            }
        }

        std::vector<B> & getOrAdd(const A &a)
        {
            return map[a];
        }

        void remove(const A &key, const B &value)
        {
            if (!map.containsKey(key))
                return;

            std::vector<B> &out = map[key];
            out.erase(find(out.begin(), out.end(), value));
        }

        void put(const A &key, const B &value, bool dupeCheck = false)
        {
            std::vector<B> &out = map[key];

            if (dupeCheck && find(out.begin(), out.end(), value) != out.end())
                return;

            out.push_back(value);
        }

        /**
         * Removes entire list indexed by key
         */
        void remove(const A &key)
        {
            map.remove(key);
        }

        template<typename MapIterator>
        void putAll(MapIterator first, MapIterator last)
        {
            MapIterator it;
            for (it = first; it != last; ++it)
                put(it->first, it->second);
        }

        void clear(void)
        {
            map.clear();
        }

        template<typename InputIterator>
        void putAll(const A &a, InputIterator first, InputIterator last,
                bool dupeCheck)
        {
            for (InputIterator it = first; it != last; ++it)
                put(a, *it, dupeCheck);
        }

        friend std::ostream & operator<<(std::ostream &strm,
                const MultiHashMap &m)
        {
            strm << "[" << std::endl;

            typename MapType::iterator it;
            for (it = m.map.begin(); it != m.map.end(); ++it)
            {
                strm << it->first << " || ";

                typename std::vector<B>::iterator bit;
                for (bit = it->second.begin(); bit != it->second.end(); ++bit)
                    strm << *bit << ", ";
                strm << std::endl;
            }

            return strm << "]" <<std::endl;
        }
};

template<typename A, typename B, typename MapType>
std::vector<B> MultiHashMap<A, B, MapType>::emptyVector;

template<typename A, typename B>
class IndirectMultiHashMap: public MultiHashMap<
                                    A, 
                                    B, 
                                    IndirectLinkedHashMap<A, std::vector<B> > 
                                >
{
};

#endif
