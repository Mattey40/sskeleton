#ifndef LINKEDHASHCONTAINER_H
#define LINKEDHASHCONTAINER_H

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/type_traits.hpp>
#include <iostream>

template<typename T>
struct indirect_hash
{
    size_t operator()(const T &t) const {
        boost::hash<typename boost::remove_pointer<T>::type> hasher;
        return hasher(*t);
    }
};

template<typename T>
struct indirect_equal_to
{
    bool operator()(const T &t1, const T &t2) const
    {
        return *t1 == *t2;
    }
};

template<typename T1, typename T2>
struct mutable_pair
{
    typedef T1 first_type;
    typedef T2 second_type;

    mutable_pair() :
        first(T1()), second(T2()) { }
    mutable_pair(const T1 &f, const T2 &s) :
        first(f), second(s) { }
    mutable_pair(const std::pair<T1, T2> &p) :
        first(p.first), second(p.second) { }

    T1 first;
    mutable T2 second;
};

template<
    typename Key,
    typename Value,
    typename HashType = boost::hash<
        typename boost::multi_index::member<
            mutable_pair<Key, Value>,
            Key,
            &mutable_pair<Key, Value>::first>::result_type>,
    typename PredType = std::equal_to<
        typename boost::multi_index::member<
            mutable_pair<Key, Value>,
            Key,
            &mutable_pair<Key, Value>::first>::result_type>
>
class LinkedHashMap: public boost::multi_index::multi_index_container<
                        mutable_pair<Key, Value>,
                        boost::multi_index::indexed_by<
                            boost::multi_index::sequenced<>,
                            boost::multi_index::hashed_unique<
                                boost::multi_index::member<
                                    mutable_pair<Key, Value>,
                                    Key,
                                    &mutable_pair<Key, Value>::first
                                >,
                                HashType,
                                PredType
                            >
                        >
                    >
{
    public:
        typedef typename boost::multi_index::nth_index<
            LinkedHashMap<Key, Value, HashType, PredType>, 0>::type
            sequenced_index_type;
        typedef typename boost::multi_index::nth_index<
            LinkedHashMap<Key, Value, HashType, PredType>, 1>::type
            hashed_index_type;
        typedef typename sequenced_index_type::iterator key_iterator;
        typedef typename hashed_index_type::iterator hash_iterator;

        sequenced_index_type & sequencedIndex(void)
        {
            return boost::multi_index::get<0>(*this);
        }

        hashed_index_type & hashedIndex(void)
        {
            return boost::multi_index::get<1>(*this);
        }

        Value & operator[](const Key &k)
        {
            using namespace boost::multi_index;

            hash_iterator it;

            it = hashedIndex().find(k);
            if (it == hashedIndex().end())
            {
                this->push_back(mutable_pair<Key, Value>(k, Value()));
                it = hashedIndex().find(k);
            }

            return it->second;
        }

        bool containsKey(const Key &k)
        {
            return hashedIndex().find(k) != hashedIndex().end();
        }

        void remove(const Key &k)
        {
            hash_iterator it = hashedIndex().find(k);
            if (it != hashedIndex().end())
                hashedIndex().erase(it);
        }

        friend std::ostream & operator<<(std::ostream &strm,
                const LinkedHashMap &t)
        {
            strm << "{ ";

            key_iterator it;
            for (it = t.begin(); it != t.end(); ++it)
                strm << "[" << it->first << "=" << it->second << "], ";

            strm << "}";
            return strm;
        }
};

template<typename Key, typename Value>
class IndirectLinkedHashMap: public LinkedHashMap<
        Key,
        Value,
        indirect_hash<
            typename boost::multi_index::member<
                mutable_pair<Key, Value>,
                Key,
                &mutable_pair<Key, Value>::first>::result_type>,
        indirect_equal_to<
            typename boost::multi_index::member<
                mutable_pair<Key, Value>,
                Key,
                &mutable_pair<Key, Value>::first>::result_type>
    >
{
    public:
        friend std::ostream & operator<<(std::ostream &strm,
                const IndirectLinkedHashMap &t)
        {
            strm << "{ ";

            typename IndirectLinkedHashMap<Key, Value>::key_iterator it;
            for (it = t.begin(); it != t.end(); ++it)
                strm << "[" << *it->first << "=" << *it->second << "], ";

            strm << "}";
            return strm;
        }
};

template<
    typename T,
    typename HashType = boost::hash<
        typename boost::multi_index::identity<T>::result_type>,
    typename PredType = std::equal_to<
        typename boost::multi_index::identity<T>::result_type>
>
class LinkedHashSet: public boost::multi_index::multi_index_container<
                        T,
                        boost::multi_index::indexed_by<
                            boost::multi_index::sequenced<>,
                            boost::multi_index::hashed_unique<
                                boost::multi_index::identity<T>,
                                HashType,
                                PredType
                            >
                        >
                     >
{
    public:
        typedef typename boost::multi_index::nth_index<
            LinkedHashSet<T, HashType, PredType>, 0>::type sequenced_index_type;
        typedef typename boost::multi_index::nth_index<
            LinkedHashSet<T, HashType, PredType>, 1>::type hashed_index_type;
        typedef typename sequenced_index_type::iterator iterator;
        typedef typename hashed_index_type::iterator hash_iterator;

        sequenced_index_type & sequencedIndex(void)
        {
            return boost::multi_index::get<0>(*this);
        }

        hashed_index_type & hashedIndex(void)
        {
            return boost::multi_index::get<1>(*this);
        }

        bool contains(const T &t)
        {
            return hashedIndex().find(t) != hashedIndex().end();
        }

        void remove(const T &t)
        {
            hash_iterator it = hashedIndex().find(t);
            if (it != hashedIndex().end())
                hashedIndex().erase(it);
        }

        friend std::ostream & operator<<(std::ostream &strm,
                const LinkedHashSet &t)
        {
            strm << "{ ";

            iterator it;
            for (it = t.begin(); it != t.end(); ++it)
                strm << *it << ", ";

            strm << "}";
            return strm;
        }
};

template<typename T>
class IndirectLinkedHashSet: public LinkedHashSet<
        T,
        indirect_hash<typename boost::multi_index::identity<T>::result_type>,
        indirect_equal_to<typename boost::multi_index::identity<T>::result_type>
    >
{
    public:
        friend std::ostream & operator<<(std::ostream &strm,
                const IndirectLinkedHashSet &t)
        {
            strm << "{ ";

            typename IndirectLinkedHashSet<T>::iterator it;
            for (it = t.begin(); it != t.end(); ++it)
                strm << **it << ", ";

            strm << "}";
            return strm;
        }
};

template<typename T>
size_t hash_value(const LinkedHashSet<T> &s)
{
    size_t val = 0;
    boost::hash<T> hasher;
    for (typename LinkedHashSet<T>::iterator it = s.begin(); it != s.end();
            ++it)
        val += hasher(*it);

    return val;
}

template<typename T>
size_t hash_value(const IndirectLinkedHashSet<T> &s)
{
    size_t val = 0;
    boost::hash<typename boost::remove_pointer<T>::type> hasher;
    for (typename IndirectLinkedHashSet<T>::iterator it = s.begin();
            it != s.end(); ++it)
    {
        T t = *it;
        val += hasher(*t);
    }

    return val;
}

#endif
