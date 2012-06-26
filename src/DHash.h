#ifndef DHASH_H
#define DHASH_H

#include <stdexcept>
#include "LinkedHashContainer.h"

/**
 * Two directional hash map
 */

template<
    typename A,
    typename B,
    typename ABMapType = LinkedHashMap<A, B>,
    typename BAMapType = LinkedHashMap<B, A>
>
class DHash
{
    public:
        typedef ABMapType ABMapT;
        typedef BAMapType BAMapT;

        ABMapType ab;
        BAMapType ba;

        void clear(void)
        {
            ab.clear();
            ba.clear();
        }

        void put(const A &a, const B &b)
        {
            ab[a] = b;
            ba[b] = a;
        }

        B & get(const A &a)
        {
            if (!ab.containsKey(a))
                throw std::runtime_error("DHash does not contain key!");

            return ab[a];
        }

        A & teg(const B &b)
        {
            if (ba.containsKey(b))
                throw std::runtime_error("DHash does not contain key!");

            return ba[b];
        }

        bool containsA(const A &a)
        {
            return ab.containsKey(a);
        }

        bool containsB(const B &b)
        {
            return ba.containsKey(b);
        }

        bool operator==(const DHash &other) const
        {
            return this->ab == other.ab && this->ba == other.ba;
        }

        void removeA(const A &a)
        {
            try
            {
                ba.remove(get(a));
            }
            catch (std::exception &e)
            {
            }

            ab.remove(a);
        }

        void removeB(const B &b)
        {
            ba.remove(b);

            try
            {
                ab.remove(teg(b));
            }
            catch (std::exception &e)
            {
            }
        }

        /* Prob not even necessary in C++ */
        void shallowDupe(DHash<A, B, ABMapType, BAMapType> &out)
        {
            out.ab.insert(out.ab.begin(), ab.begin(), ab.end());
            out.ba.insert(out.ba.begin(), ba.begin(), ba.end());
        }
};

template<typename A, typename B>
class IndirectDHash: public DHash<
                        A,
                        B,
                        IndirectLinkedHashMap<A, B>,
                        IndirectLinkedHashMap<B, A>
                     >
{
};

#endif
