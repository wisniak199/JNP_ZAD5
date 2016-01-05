#include <iostream>
#include <map>
#include <set>

using namespace std;
namespace {
    template<typename K>
    struct key_it_comp {
        bool operator() (const typename set<K>::iterator& a, const typename set<K>::iterator& b) const {return *a < *b;}
    };

    template<typename V>
    struct value_it_comp {
        bool operator() (const typename set<V>::iterator a, const typename set<V>::iterator& b) const {return *a < *b;}
    };
}

class PriorityQueueEmptyException {
    public:
    virtual const char* what() const throw()
    {
        return "My exception happened";
    }
};

template<typename K, typename V>
class PriorityQueue {
    public:
    typedef size_t size_type;

    private:
    typedef set<V> value_set;
    typedef set<K> key_set;
    typedef map<typename value_set::iterator, multiset<typename key_set::iterator, key_it_comp<K>>, value_it_comp<V>> value_to_key;
    typedef map<typename key_set::iterator, multiset<typename value_set::iterator, value_it_comp<V>>, key_it_comp<K>> key_to_value;
    size_type elem;
    set<V> values;
    set<K> keys;
    value_to_key v_to_k;
    key_to_value k_to_v;

    public:
    PriorityQueue() : elem(), values(), keys(), v_to_k(), k_to_v() {}

    size_type size() const {
        return elem;
    }

    size_type empty() const {
        return elem == 0;
    }

    void insert(const K& key, const V& value) {
        elem++;
        values.insert(value);
        typename value_set::iterator value_it = values.find(value);
        keys.insert(key);
        typename key_set::iterator key_it = keys.find(key);

        if (v_to_k.find(value_it) == v_to_k.end()) {
            multiset<typename key_set::iterator, key_it_comp<K>> temp;
            v_to_k.emplace(value_it, temp);
        }
        typename value_to_key::iterator it1 = v_to_k.find(value_it);
        it1->second.insert(key_it);

        if (k_to_v.find(key_it) == k_to_v.end()) {
            multiset<typename value_set::iterator, value_it_comp<V>> temp;
            k_to_v.emplace(key_it, temp);
        }
        typename key_to_value::iterator it2 = k_to_v.find(key_it);
        it2->second.insert(value_it);
    }

    const V& minValue() const {
        if (empty())
            throw new PriorityQueueEmptyException();
        return *(values.begin());
    }

    const V& maxValue() const {
        if (empty())
            throw new PriorityQueueEmptyException();
        return *(values.rbegin());
    }

    const K& minKey() const {
        if (empty())
            throw new PriorityQueueEmptyException();
        typename value_set::iterator it = values.begin();
        return **((v_to_k.find(it)->second).begin());
    }

    const K& maxKey() const {
        if (empty())
            throw new PriorityQueueEmptyException();
        typename value_set::iterator it = values.end();
        it--;
        return **((v_to_k.find(it)->second).begin());
    }

    void deleteMin() {
        typename value_set::iterator min_v = values.begin();

    }
};




