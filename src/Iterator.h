#ifndef ITERATOR_H
#define ITERATOR_H

#include <stdexcept>

template<typename T>
class Iterator
{
    public:
        virtual ~Iterator(void) { }

        virtual bool hasNext(void) = 0;
        virtual T * next(void) = 0;

        virtual void remove(void)
        {
            throw std::runtime_error("Not implemented!");
        }
};

/* Adapter to stl-style iterators */
template<typename Container,
    typename ContainerIterator = typename Container::iterator>
class STLIterator: public Iterator<typename Container::value_type>
{
    public:
        STLIterator(Container &container) :
            container(container)
        {
            it = container.begin();
        }

        virtual bool hasNext(void)
        {
            return it != container.end();
        }

        virtual typename Container::value_type * next(void)
        {
            typename Container::value_type * out = &*it;
            it++;
            return out;
        }

    private:
        Container &container;
        ContainerIterator it;
};

#endif
