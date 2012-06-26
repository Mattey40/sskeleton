#ifndef SETCORRESPONDENCE_H
#define SETCORRESPONDENCE_H

#include "DHash.h"
#include "LinkedHashContainer.h"

/**
 * Data type form mappings between two sets of sets
 */
template<typename A, typename B>
class IndirectSetCorrespondence
{
    public:
        /**
         * If either a or b belongs to an existing set, both a and b collapse
         * into that a/b set pair.
         * Symmetrical operation.
         */
        void put(A a, B b)
        {
            IndirectLinkedHashSet<A> *aSet = getSet(a, whichA);
            IndirectLinkedHashSet<B> *bSet = getSet(b, whichB);

            IndirectLinkedHashSet<A> *shouldBeA = NULL;
            IndirectLinkedHashSet<B> *shouldBeB = NULL;
            try { shouldBeA = setToSet.teg(bSet); } catch (std::exception &e){ }
            try { shouldBeB = setToSet.get(aSet); } catch (std::exception &e){ }

            if (shouldBeA != NULL && shouldBeA != aSet) // existing B set already references another A set
            {
                if (aSet->size() != 1)
                    throw std::runtime_error("size of aSet should be 1!");

                shouldBeA->insert(shouldBeA->end(), aSet->begin(), aSet->end());

                for (typename IndirectLinkedHashSet<A>::iterator a2it =
                        aSet->begin(); a2it != aSet->end(); ++a2it)
                    whichA[*a2it] = shouldBeA;
                aSet = shouldBeA;
            }
            else
            {
                whichA[a] = aSet; // either new set, or
            }

            if (shouldBeB != NULL && shouldBeB != bSet) // a references another b (not bSet)
            {
                if (bSet->size() != 1)
                    throw std::runtime_error("size of bSet should be 1!");

                shouldBeB->insert(shouldBeB->end(), bSet->begin(), bSet->end());

                for (typename IndirectLinkedHashSet<B>::iterator b2it =
                        bSet->begin(); b2it != bSet->end(); ++b2it)
                    whichB[*b2it] = shouldBeB;
                bSet = shouldBeB;
            }
            else
            {
                whichB[b] = bSet;
            }

            setToSet.put(aSet, bSet);
        }

        IndirectLinkedHashSet<B> * getSetA(const A &a)
        {
            IndirectLinkedHashSet<A> *aSet = getSet(a, whichA);

            try
            {
                return setToSet.get(aSet);
            }
            catch (std::exception &e)
            {
                return new IndirectLinkedHashSet<B>();
            }
        }

        IndirectLinkedHashSet<A> * getSetB(const B &b)
        {
            IndirectLinkedHashSet<B> bSet = getSet(b, whichB);

            try
            {
                return setToSet.teg(bSet);
            }
            catch(std::exception &e)
            {
                return new IndirectLinkedHashSet<A>();
            }
        }

        template<typename T>
        IndirectLinkedHashSet<T> * getSet(T o,
                IndirectLinkedHashMap<T, IndirectLinkedHashSet<T> *> set)
        {
            IndirectLinkedHashSet<T> *res = NULL;
            if (set.containsKey(o))
                res = set[o];
            else
            {
                res = new IndirectLinkedHashSet<T>();
                res->push_back(o);
                set[o] = res;
            }

            if (!set.containsKey(o))
                throw std::runtime_error("set (map) does not contain key!");

            return res;
        }

        void removeA(const A &a)
        {
            IndirectLinkedHashSet<A> *aSet = getSet(a, whichA);
            aSet->remove(a);
            whichA.remove(a);
            if (aSet->empty())
                setToSet.removeA(aSet);
        }

    private:
        IndirectLinkedHashMap<A, IndirectLinkedHashSet<A> *> whichA;
        IndirectLinkedHashMap<B, IndirectLinkedHashSet<B> *> whichB;
        IndirectDHash<IndirectLinkedHashSet<A> *, IndirectLinkedHashSet<B> *>
            setToSet;
};

#endif
