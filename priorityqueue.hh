#include <iostream>
#include <set>
#include <memory>
#include <utility>

class PriorityQueueEmptyException : public std::exception {
    public:
    virtual const char* what() const throw()
    {
        return "My exception happened";
    }
};

class PriorityQueueNotFoundException : public std::exception {
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
        typedef K key_type;
        typedef V value_type;

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
                return a->second == b->second ? a->first < b->first : a->second < b->second;
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

    //byc moze kazdy wyjatek trzeba lapac a potem rzucac ponownie - mozliwe ze jak nie zlapie to nie wywoluja sie destruktory
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
            throw PriorityQueueEmptyException();
        return (*(values.begin()))->second;
    }

    const V& maxValue() const {
        if (empty())
            throw PriorityQueueEmptyException();
        return (*(values.rbegin()))->second;
    }

    const K& minKey() const {
        if (empty())
            throw PriorityQueueEmptyException();
        return (*(values.begin()))->first;
    }

    const K& maxKey() const {
        if (empty())
            throw PriorityQueueEmptyException();
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
        //UWAGA to nizej chyba nie potrzebne
        //to_delete_it_k--;
        //erase jest no_throw jezeli dajemy mu iterator
        values.erase(to_delete_it_v);
        keys.erase(to_delete_it_k);
    }

    void changeValue(const K& key, const V& value) {
        if (empty())
            throw PriorityQueueNotFoundException();
        key_value_pair to_add(key, value);
        key_value_ptr to_add_ptr = std::make_shared<key_value_pair>(to_add);
        typename key_set::iterator to_remove_it_k = keys.lower_bound(to_add_ptr);

        if (to_remove_it_k == keys.end() || !((*to_remove_it_k)->first == key))
            to_remove_it_k--;
        if (**to_remove_it_k == to_add)
            return;
        if (!((*to_remove_it_k)->first == key))
            throw PriorityQueueNotFoundException();

        typename value_set::iterator to_remove_it_v = values.find(*to_remove_it_k);

        //dodajemy ptr

        typename value_set::iterator added_v = values.insert(to_add_ptr);
        try {
            keys.insert(to_add_ptr);
        }
        catch (...) {
            values.erase(added_v);
            throw;
        }
        keys.erase(to_remove_it_k);
        values.erase(to_remove_it_v);
    }

    void swap(PriorityQueue<K, V>& queue) {
        values.swap(queue.values);
        keys.swap(queue.keys);
    }

    void merge(PriorityQueue<K, V>& queue) {
        //dodajemy do nowych setow - jak sie wysypie to sie nic nie stanie bo nie dotykamy naszych wenwetrznych
        if (this == &queue)
            return;
        value_set new_values(this->values);
        key_set new_keys(this->keys);
        for (auto to_add : queue.values) {
            auto it = this->values.find(to_add);
            if (it != this->values.end()) {
                new_values.insert(*it);
                new_keys.insert(*it);
            }
            else {
                new_values.insert(to_add);
                new_keys.insert(to_add);
            }
        }
        //dodawanie sie udalo wiec mozemy umiescic te sety w naszej kolejsce
        this->values.swap(new_values);
        this->keys.swap(new_keys);
        if (this != &queue)
            queue.clear();
    }

    void clear() {
        values.clear();
        keys.clear();
    }

    PriorityQueue<K, V>& operator=(PriorityQueue<K, V> queue) {
        this->swap(queue);
        return *this;
    }

    bool operator==(const PriorityQueue<K, V>& queue) const {
        if (this->size() != queue.size())
            return false;
        typename value_set::iterator this_it = this->values.begin();
        typename value_set::iterator queue_it = queue.values.begin();
        while (this_it != this->values.end()) {
            if (**this_it != **queue_it)
                return false;
            this_it++;
            queue_it++;
        }
        return true;
    }

    bool operator!=(const PriorityQueue<K, V>& queue) const {
        return !(*this == queue);
    }

    bool operator<(const PriorityQueue<K, V>& queue) const {
        typename value_set::iterator this_it = this->values.begin();
        typename value_set::iterator queue_it = queue.values.begin();
        while (this_it != this->values.end() && queue_it != queue.values.end()) {
            if (**this_it != **queue_it) {
                if (**this_it < **queue_it)
                    return true;
                else
                    return false;
            }
            this_it++;
            queue_it++;
        }
        if (this_it == this->values.end() && queue_it == queue.values.end())
            return false;
        else if (this_it == this->values.end())
            return true;
        else
            return false;
    }

    bool operator<=(const PriorityQueue<K, V>& queue) const {
        return *this == queue || *this < queue;
    }

    bool operator>(const PriorityQueue<K, V>& queue) const {
        return !(*this <= queue);
    }

    bool operator>=(const PriorityQueue<K, V>& queue) const {
        return !(*this < queue);
    }

};

template <typename K, typename V>
void swap(PriorityQueue<K, V>& a, PriorityQueue<K, V>& b) {
    a.swap(b);
}





