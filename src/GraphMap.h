#ifndef GRAPHMAP_H
#define GRAPHMAP_H

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <vector>
#include "LinkedHashContainer.h"

template<
    typename T,
    typename MapT = LinkedHashMap<T, std::vector<T> >
>
class GraphMap
{
    public:
        typedef MapT MapType;
        MapType map;

        void add(T a, T b)
        {
            addEntry(a, b);
            addEntry(b, a);
        }

        void addEntry(T a, T b)
        {
            std::vector<T> &res = map[a];

            if (std::find(res.begin(), res.end(), b) == res.end())
                res.push_back(b);
        }

        std::vector<T> * get(T a)
        {
            if (map.containsKey(a))
                return &map[a];
            else
                return NULL;
        }

        void clear(void)
        {
            map.clear();
        }

        void addEntriesFrom(GraphMap<T> &otherMap)
        {
            using namespace std;

            if (&otherMap == this)
                return; // done!

            typename MapType::key_iterator it1;
            for (it1 = otherMap.map.begin(); it1 != otherMap.map.end(); ++it1)
            {
                vector<T> &v = it1->second;
                typename vector<T>::iterator it2;
                for (it2 = v.begin(); it2 != v.end(); ++it2)
                    addEntry(it1->first, *it2);
            }
        }

        friend std::ostream & operator<<(std::ostream &strm,
                const GraphMap &t)
        {
            using namespace std;

            typename MapType::key_iterator it1;
            for (it1 = t.map.begin(); it1 != t.map.end(); ++it1)
            {
                strm << it1->first << " |||   ";
                vector<T> &v = it1->second;
                typename vector<T>::iterator it2;
                for (it2 = v.begin(); it2 != v.end(); ++it2)
                    strm << *it2 << ",";

                strm << endl;
            }

            return strm;
        }

        void remove(T a, T b)
        {
            remove_(a, b);
            remove_(b, a);
        }

    private:
        void remove_(T a, T b)
        {
            using namespace std;

            vector<T> *e = get(a);
            if (e == NULL)
                return;
            e->erase(find(e->begin(), e->end(), b));
            if (e->size() == 0)
                map.remove(a);
        }
};

template<typename T>
class IndirectGraphMap: public GraphMap<
                                    T,
                                    IndirectLinkedHashMap<T, std::vector<T> >
                                >
{
};

#endif
