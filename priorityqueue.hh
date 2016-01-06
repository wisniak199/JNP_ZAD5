#include <iostream>
#include <set>
#include <memory>
#include <utility>

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
        typedef std::pair<K, V> key_value_pair;
        typedef std::shared_ptr<key_value_pair> key_value_ptr;

        //struktury do porownywania
        struct key_first_cmp {
            bool operator() (const key_value_ptr& a, const key_value_ptr& b) const {
                return a->first == b->first ? a->second < b->second : a->first < b->first;
            }
        };

        struct value_first_cmp {
            bool operator() (const key_value_ptr& a, const key_value_ptr& b) const {
                return a->second == b->second ? a->first < b->second : a->second < b->second;
            }
        };

        typedef std::multiset<key_value_ptr, value_first_cmp> value_set;
        typedef std::multiset<key_value_ptr, key_first_cmp> key_set;

        //oba sety zawieraja te same wskazniki na pary, roznia sie tylko kolejnoscia sortowania
        value_set values;
        key_set keys;

    public:

    PriorityQueue() : values(), keys(){}

    //UWAGA Czy kopiowanie moze sie nie powiesc
    PriorityQueue(const PriorityQueue<K, V>& queue) : values(queue.values), keys(queue.keys) {}

    //Uwaga Czy move moze sie wysypac, czy trzeba wyczyscic queue bo teraz bedzie w bardzo nieladnym stanie
    PriorityQueue(const PriorityQueue<K, V>&& queue) : values(std::move(queue.values)), keys(std::move(queue.keys)) {}

    size_type size() const {
        return values.size();
    }

    size_type empty() const {
        return values.empty();
    }

    void insert(const K& key, const V& value) {
        key_value_pair to_add(key, value);
        key_value_ptr to_add_ptr = std::make_shared<key_value_pair>(to_add);
        typename value_set::iterator where_to_add_values = values.lower_bound(to_add_ptr);

        //jezeli w secie istnieje juz taka para to nie wstawiamy do pointera wskazujacy na nowy obiekt stworzony przez make_shared
        //tylko zminiamy go na ptr ktory juz jest w secie, dzeki temu ten obiekt ktory przed chwila stworzyl sie przez make_shared
        //zostanie usuniety
        if (where_to_add_values != values.end() && **where_to_add_values == to_add)
            to_add_ptr = *where_to_add_values;

        //czy to wystarczy?
        //jak do pierwszego nie uda sie wrzucic to wszytko jest ok wyjatek leci
        typename value_set::iterator where_added = values.insert(to_add_ptr);
        try {
            //jak tu sie nie uda to trzeba usunac z porzedniego
            keys.insert(to_add_ptr);
        }
        catch (...) {
            //erase jest nothrow jezeli dajemy mu iterator
            values.erase(where_added);
            throw;
        }

    }

    const V& minValue() const {
        if (empty())
            throw new PriorityQueueEmptyException();
        return (*(values.begin()))->second;
    }

    const V& maxValue() const {
        if (empty())
            throw new PriorityQueueEmptyException();
        return (*(values.rbegin()))->second;
    }

    const K& minKey() const {
        if (empty())
            throw new PriorityQueueEmptyException();
        return (*(values.begin()))->first;
    }

    const K& maxKey() const {
        if (empty())
            throw new PriorityQueueEmptyException();
        return (*(values.rbegin()))->first;
    }

    void deleteMin() {
        if (empty())
            return;
        typename value_set::iterator to_delete_it_v = values.begin();
        typename key_set::iterator to_delete_it_k = keys.lower_bound(*to_delete_it_v);
        //erase jest no_throw jezeli dajemy mu iterator
        values.erase(to_delete_it_v);
        keys.erase(to_delete_it_k);
    }

    void deleteMax() {
        if (empty())
            return;
        typename value_set::iterator to_delete_it_v = values.end();
        to_delete_it_v--;
        typename key_set::iterator to_delete_it_k = keys.lower_bound(*to_delete_it_v);
        to_delete_it_k--;
        //erase jest no_throw jezeli dajemy mu iterator
        values.erase(to_delete_it_v);
        keys.erase(to_delete_it_k);
    }
};




