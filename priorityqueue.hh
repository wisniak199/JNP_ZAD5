/**
Autor: Piotr Wiśniewski 361286
*/

#ifndef __PRIORITYQUEUE_HH__
#define __PRIORITYQUEUE_HH__

#include <set>
#include <memory>


// Wyjatki
class PriorityQueueEmptyException : public std::exception {
    public:
    virtual const char* what() const throw()
    {
        return "PriorityQueue is empty!";
    }
};

class PriorityQueueNotFoundException : public std::exception {
    public:
    virtual const char* what() const throw()
    {
        return "Key not found in PriorityQueue!";
    }
};

// Klasa przechowuje 2 multisety zawierajace shared_ptr
// na pary key value values sortowany najpierw po value
// potem key keys sortowny najpierw po key potem po value
// oba sety zawieraja te same wskazniki na pary, roznia sie
// tylko kolejnoscia sortowania
template<typename K, typename V>
class PriorityQueue {
    public:
        // typy globalne
        typedef size_t size_type;
        typedef K key_type;
        typedef V value_type;

    private:
        // typy pomocnicze
        typedef std::pair<K, V> key_value_pair;
        typedef std::shared_ptr<key_value_pair> key_value_ptr;

        // struktury pomocnicze do porownywana pointerow na pary
        // pointery sortowane sa po parach na ktore wskazuja
        struct key_first_cmp {
            bool operator() (const key_value_ptr& a, const key_value_ptr& b) const {
                return !(a->first < b->first || b->first < a->first)
                    ? a->second < b->second : a->first < b->first;
            }
        };

        struct value_first_cmp {
            bool operator() (const key_value_ptr& a, const key_value_ptr& b) const {
                return !(a->second < b->second || b->second < a->second)
                    ? a->first < b->first : a->second < b->second;
            }
        };

        // typy pomocnicze dla setow
        typedef std::multiset<key_value_ptr, value_first_cmp> value_set;
        typedef std::multiset<key_value_ptr, key_first_cmp> key_set;

        // sety do przechowywania pointerow
        value_set values;
        key_set keys;

        // funkcje pomocnicze do stwerdzania czy 2 pary sa rowne
        // korzysta tylko z  operatora <
        bool equal_key(const K& k1, const K& k2) const {
            return !(k1 < k2 || k2 < k1);
        }

        bool equal_value(const V& v1, const V& v2) const {
            return !(v1 < v2 || v2 < v1);
        }

        bool equal_pair(const key_value_pair& p1, const key_value_pair& p2) const {
            return equal_key(p1.first, p2.first) && equal_value(p1.second, p2.second);
        }

    public:

        // Konstruktor bezparametrowy tworzacy pusta kolejke [O(1)]
        PriorityQueue() : values(), keys(){}

        // Konstruktor kopiujacy [O(queue.size())]
        PriorityQueue(const PriorityQueue<K, V>& queue) :
            values(queue.values), keys(queue.keys) {}

        // Operator przypisania [O(queue.size()) dla uzycia P = Q, a O(1) dla uzycia
        // P = move(Q)]
        PriorityQueue(PriorityQueue<K, V>&& queue) :
            values(std::move(queue.values)), keys(std::move(queue.keys)) {}

        // Metoda zwracajaca liczbe par (klucz, wartosc) przechowywanych w kolejce
        // [O(1)]
        size_type size() const {
            return values.size();
        }

        // Metoda zwracajaca true wtedy i tylko wtedy, gdy kolejka jest pusta [O(1)]
        size_type empty() const {
            return values.empty();
        }

        // Metoda wstawiajaca do kolejki pare o kluczu key i wartosci value
        // [O(log size())] (dopuszczamy możliwosc wystepowania w kolejce wielu
        // par o tym samym kluczu)
        // Wyjatki: silna gwarancja
        void insert(const K& key, const V& value) {
            key_value_ptr to_add_ptr = std::make_shared<key_value_pair>(key, value);
            typename value_set::iterator where_to_add_values = values.find(to_add_ptr);

            // jezeli w secie istnieje juz taka para, ktora chcemy teraz wstawic
            // to nie wstawiamy pointera wskazujacego na nowy obiekt stworzony przez make_shared
            // tylko zminiamy go na ptr ktory juz jest w secie,
            // dzeki temu ten obiekt ktory, przed chwila stworzyl sie przez make_shared
            // zostanie usuniety
            if (where_to_add_values != values.end())
                to_add_ptr = *where_to_add_values;

            // jezeli nie uda sie wstawic, to nic sie nie stalo bo insert ma silna gw.
            typename value_set::iterator where_added = values.insert(to_add_ptr);
            try {
                // jak tu sie nie uda to trzeba usunac z porzedniego seta
                keys.insert(to_add_ptr);
            }
            catch (...) {
                // erase jest nothrow jezeli dajemy mu iterator
                values.erase(where_added);
                throw;
            }

        }

        // Metoda zwracajaca najmniejsza wartosc przechowywana w kolejce [O(1)]
        // Wyjatki: silna gwarancja
        const V& minValue() const {
            if (empty())
                throw PriorityQueueEmptyException();
            return (*(values.begin()))->second;
        }

        // Metoda zwracajaca najwieksza wartosc przechowywana w kolejce [O(1)]
        // Wyjatki: silna gwarancja
        const V& maxValue() const {
            if (empty())
                throw PriorityQueueEmptyException();
            return (*(values.rbegin()))->second;
        }

        // Metoda zwracajaca klucz o przypisanej najmniejszej wartosci przechowywanej
        // w kolejce [O(1)]
        // Wyjatki: silna gwarancja
        const K& minKey() const {
            if (empty())
                throw PriorityQueueEmptyException();
            return (*(values.begin()))->first;
        }

        // Metoda zwracajaca klucz o przypisanej najwiekszej wartosci przechowywanej
        // w kolejce [O(1)]
        // Wyjatki: silna gwarancja
        const K& maxKey() const {
            if (empty())
                throw PriorityQueueEmptyException();
            return (*(values.rbegin()))->first;
        }

        // Metoda usuwajaca z kolejki jedna pare o najmniejszej wartości [O(log size())]
        // Wyjatki: silna gwarancja
        void deleteMin() {
            if (empty())
                return;
            // najpierw szukamy odpowednich elementow - moze rzucic wyjatkiem
            typename value_set::iterator to_delete_it_v = values.begin();
            typename key_set::iterator to_delete_it_k = keys.lower_bound(*to_delete_it_v);
            // erase jest no_throw jezeli dajemy mu iterator
            values.erase(to_delete_it_v);
            keys.erase(to_delete_it_k);
        }

        // Metoda usuwajaca z kolejki jedna pare o najwiekszej wartości [O(log size())]
        // Wyjatki: silna gwarancja
        void deleteMax() {
            if (empty())
                return;
            // najpierw szukamy odpowiednich elementow - moze rzucic wyjatkiem
            typename value_set::iterator to_delete_it_v = values.end();
            to_delete_it_v--;
            typename key_set::iterator to_delete_it_k = keys.lower_bound(*to_delete_it_v);
            // erase jest no_throw jezeli dajemy mu iterator
            values.erase(to_delete_it_v);
            keys.erase(to_delete_it_k);
        }

        // Metoda zmieniajaca dotychczasowa wartosc przypisana kluczowi key na nowa
        // wartosc value [O(log size())]; w przypadku kiedy w kolejce jest kilka par
        // o kluczu key, zmienia wartość w dowolnie wybranej parze o podanym kluczu
        // Wyjatki: silna gwarancja
        void changeValue(const K& key, const V& value) {
            if (empty())
                throw PriorityQueueNotFoundException();
            // tworzymy obiekt ktory byc moze bedziemy wstawiac
            key_value_pair to_add(key, value);
            key_value_ptr to_add_ptr = std::make_shared<key_value_pair>(to_add);
            // szukamy obiektu o tym samym kluczu
            typename key_set::iterator to_remove_it_k = keys.lower_bound(to_add_ptr);

            // w przypadku lower_bound iterator na ptr na pare o kluczu key bedzie albo
            // w zwroconym iteratorze, albo w miescu przed tym iteratorem
            // cofamy sie tylko wtedy jak nie ejstesmy na pcozatku
            if (to_remove_it_k == keys.end() ||
                    (!equal_key((*to_remove_it_k)->first, key) && to_remove_it_k != keys.begin()))
                to_remove_it_k--;
            // jezeli znalezlismy pare rowna tej, ktora chcemy wstawic to nic nie robimy
            // to zgodne ze specyfikacja - mozemy dowolny
            if (equal_pair(**to_remove_it_k, to_add))
                return;
            // jezeli tu nie ma odpowiedniego klucza to na pewno go nie ma
            if (!equal_key((*to_remove_it_k)->first, key))
                throw PriorityQueueNotFoundException();

            // to bedziemy pozniej usuwac
            typename value_set::iterator to_remove_it_v = values.find(*to_remove_it_k);

            // dodajemy nasza nowa pare - jak sie nie uda to nic sie nie stanie
            typename value_set::iterator added_v = values.insert(to_add_ptr);

            // dodajac druga jezeli sie nie uda to musimy usunac poprzednia -
            // mamy iterator wiec erase ejst no throw
            try {
                keys.insert(to_add_ptr);
            }
            catch (...) {
                values.erase(added_v);
                throw;
            }
            // dopiero teraz usuwamy juz niepotrzebna pare - erase jest no throw
            keys.erase(to_remove_it_k);
            values.erase(to_remove_it_v);
        }

        // Metoda zamieniajaca zawartosc kolejki z podana kolejka queue (tak jak
        // wiekszosc kontenerow w bibliotece standardowej) [O(1)]
        // Wyjatki: no throw
        void swap(PriorityQueue<K, V>& queue) {
            values.swap(queue.values);
            keys.swap(queue.keys);
        }

        // Metoda scalajaca zawartosc kolejki z podana kolejka queue; ta operacja usuwa
        // wszystkie elementy z kolejki queue i wstawia je do kolejki *this
        // [O(size() + queue.size() * log (queue.size() + size()))]
        // Wyjatki: silna gwarancja
        void merge(PriorityQueue<K, V>& queue) {
            if (this == &queue)
                return;
            // dodajemy do nowych setow - jak sie wysypie to sie nic nie stanie bo
            // nie dotykamy naszych wenwetrznych
            value_set new_values(this->values);
            key_set new_keys(this->keys);
            for (auto to_add : queue.values) {
                auto it = this->values.find(to_add);
                // jezeli taka para istnieje w naszej kolejce to musimy dodac
                // ten ktory juz tam jest, w przeciwnym przypadku kolejka moglaby
                // trzymac te same pary kilka razy w pamieci
                if (it != this->values.end()) {
                    new_values.insert(*it);
                    new_keys.insert(*it);
                }
                // jak nie ma w kolejce takiej pary to nie mamy wyboru
                else {
                    new_values.insert(to_add);
                    new_keys.insert(to_add);
                }
            }
            // dodawanie sie udalo wiec mozemy umiescic te sety w naszej kolejsce
            // swap jest no throw
            this->values.swap(new_values);
            this->keys.swap(new_keys);
            queue.clear();
        }

        // Czysci zawartosc kolejki [O(1)]
        // Wyjatki: no throw
        void clear() {
            values.clear();
            keys.clear();
        }

        // Operator przypisania [O(queue.size()) dla użycia P = Q, a O(1) dla użycia
        // P = move(Q)]
        // Wyjatki: silna gwarancja
        PriorityQueue<K, V>& operator=(PriorityQueue<K, V> queue) {
            queue.swap(*this);
            return *this;
        }

        // Operator rownosci [O(size())]
        // Wyjatki: silna gwarancja
        bool operator==(const PriorityQueue<K, V>& queue) const {
            if (this->size() != queue.size())
                return false;
            // iterujemy sie po wszytkich elementach w kolejnosci leksykograficznej <V, K>
            typename value_set::iterator this_it = this->values.begin();
            typename value_set::iterator queue_it = queue.values.begin();
            while (this_it != this->values.end()) {
                if (!equal_pair(**this_it, **queue_it))
                    return false;
                this_it++;
                queue_it++;
            }
            return true;
        }

        // Operator rozny [O(size())]
        // Wyjatki: silna gwarancja
        bool operator!=(const PriorityQueue<K, V>& queue) const {
            return !(*this == queue);
        }

        // Operator mniejszy [O(min(size(), queue.size())]
        // Wyjatki: silna gwarancja
        bool operator<(const PriorityQueue<K, V>& queue) const {
            typename key_set::iterator this_it = this->keys.begin();
            typename key_set::iterator queue_it = queue.keys.begin();
            // iterujemy sie po wszytkich elementach w kolejnosci leksykograficznej <V, K>
            while (this_it != this->keys.end() && queue_it != queue.keys.end()) {
                if (!equal_pair(**this_it, **queue_it)) {
                    if (**this_it < **queue_it)
                        return true;
                    else
                        return false;
                }
                this_it++;
                queue_it++;
            }
            if (this_it == this->keys.end() && queue_it == queue.keys.end())
                return false;
            else if (this_it == this->keys.end())
                return true;
            else
                return false;
        }

        // Operator mniejszy lub rowny [O(min(size(), queue.size())]
        // Wyjatki: silna gwarancja
        bool operator<=(const PriorityQueue<K, V>& queue) const {
            return *this == queue || *this < queue;
        }

        // Operatorwiekszy [O(min(size(), queue.size())]
        // Wyjatki: silna gwarancja
        bool operator>(const PriorityQueue<K, V>& queue) const {
            return !(*this <= queue);
        }

        // Operator wiekszy lub rowny [O(min(size(), queue.size())]
        // Wyjatki: silna gwarancja
        bool operator>=(const PriorityQueue<K, V>& queue) const {
            return !(*this < queue);
        }

};

// Metoda zamieniajaca zawartosc kolejek a i b [O(1)]
// Wyjatki: no throw
template <typename K, typename V>
void swap(PriorityQueue<K, V>& a, PriorityQueue<K, V>& b) {
    a.swap(b);
}

#endif /*__PRIORITYQUEUE_HH__*/



