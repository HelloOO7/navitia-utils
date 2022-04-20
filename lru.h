/* Copyright © 2001-2015, Hove and/or its affiliates. All rights reserved.

This file is part of Navitia,
    the software to build cool stuff with public transport.

Hope you'll enjoy and contribute to this project,
    powered by Hove (www.hove.com).
Help us simplify mobility and open public transport:
    a non ending quest to the responsive locomotion way of traveling!

LICENCE: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

Stay tuned using
twitter @navitia
IRC #navitia on freenode
https://groups.google.com/d/forum/navitia
www.navitia.io
*/

#include "functions.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <mutex>
#include <future>
#include <stdexcept>

namespace navitia {

// forward declare
template <typename T>
struct ConcurrentLru;

// Encapsulate a unary function, and provide a least recently used
// cache.  The function must be pure (same argument => same result),
// and a Lru object must not be shared across threads.
template <typename F>
class Lru {
private:
    typedef typename boost::remove_cv<typename boost::remove_reference<typename F::argument_type>::type>::type key_type;
    using mapped_type =
        typename boost::remove_cv<typename boost::remove_reference<typename F::result_type>::type>::type;
    using value_type = std::pair<const key_type, mapped_type>;
    using Cache = boost::multi_index_container<
        value_type,
        boost::multi_index::indexed_by<
            boost::multi_index::sequenced<>,
            boost::multi_index::ordered_unique<
                boost::multi_index::member<value_type, const key_type, &value_type::first>>>>;

    // the encapsulate function
    F f;

    // maximal cached values
    size_t max_cache;

    // the cache, mutable because side effect are not visible from the
    // exterior because of the purity of f
    mutable Cache cache;
    mutable size_t nb_cache_miss = 0;
    mutable size_t nb_calls = 0;

    std::vector<key_type> keys() const {
        auto& list = cache.template get<0>();
        std::vector<key_type> result;
        for (const auto& p : list) {
            result.push_back(p.first);
        }
        return result;
    }

    template <typename T>
    friend struct ConcurrentLru;

public:
    using result_type = const mapped_type&;
    using argument_type = typename F::argument_type;

    Lru(F fun, size_t max = 10) : f(std::move(fun)), max_cache(max) {
        if (max < 1) {
            throw std::invalid_argument("max (size of cache) must be strictly positive");
        }
    }

    result_type operator()(argument_type arg) const {
        ++nb_calls;
        auto& list = cache.template get<0>();
        auto& map = cache.template get<1>();
        const auto search = map.find(arg);
        if (search != map.end()) {
            // put the cached value at the begining of the cache
            list.relocate(list.begin(), cache.template project<0>(search));
            return search->second;
        } else {
            ++nb_cache_miss;
            // insert the new value at the begining of the cache
            const auto ins = list.push_front(std::make_pair(arg, f(arg)));
            // clean the cache by the end (where the entries are the
            // older ones) until the requested size
            while (list.size() > max_cache) {
                list.pop_back();
            }
            return ins.first->second;
        }
    }

    size_t get_nb_cache_miss() const { return nb_cache_miss; }
    size_t get_nb_calls() const { return nb_calls; }
    size_t get_max_size() const { return max_cache; }
};
template <typename F>
inline Lru<F> make_lru(F&& fun, size_t max = 10) {
    return Lru<F>(std::forward<F>(fun), max);
}

template <typename F>
struct ConcurrentLru {
private:
    struct SharedPtrF {
        F f;
        using argument_type = typename F::argument_type;
        using underlying_type =
            typename boost::remove_cv<typename boost::remove_reference<typename F::result_type>::type>::type const;
        using result_type = std::shared_future<std::shared_ptr<underlying_type>>;

        result_type operator()(argument_type arg) const {
            // build a future that will be lazy initialized
            return std::async(std::launch::deferred, [&]() { return std::make_shared<underlying_type>(f(arg)); })
                .share();
        }
    };
    Lru<SharedPtrF> lru;
    std::unique_ptr<std::mutex> mutex{std::make_unique<std::mutex>()};

    std::vector<typename Lru<SharedPtrF>::key_type> keys() const {
        std::lock_guard<std::mutex> lock(*mutex);
        return lru.keys();
    }

public:
    using result_type = typename std::shared_ptr<typename SharedPtrF::underlying_type>;
    using argument_type = typename SharedPtrF::argument_type;

    ConcurrentLru(F fun, size_t max = 10) : lru(SharedPtrF{std::move(fun)}, max) {}
    ConcurrentLru(ConcurrentLru&&) = default;  // NOLINT // needed by old version of gcc

    result_type operator()(argument_type arg) const {
        typename SharedPtrF::result_type future;
        {
            std::lock_guard<std::mutex> lock(*mutex);
            future = lru(arg);
        }
        // As arg might be a reference, the maybe newly created future must be run
        // before the end of the current method, else we can have a use after free.
        return future.get();
    }

    // We add mutex lock in all get_nb_** functions :
    // Without that a race condition could occurs if operator()
    // or warmup() functions are used by a thread and get_nb_** functions by another thread
    // in the same time

    size_t get_nb_cache_miss() const {
        std::lock_guard<std::mutex> lock(*mutex);
        return lru.get_nb_cache_miss();
    }
    size_t get_nb_calls() const {
        std::lock_guard<std::mutex> lock(*mutex);
        return lru.get_nb_calls();
    }
    size_t get_max_size() const {
        std::lock_guard<std::mutex> lock(*mutex);
        return lru.get_max_size();
    }

    void warmup(const ConcurrentLru<F>& other) {
        // we can't use the warmup of the lru direclty as it will mess with the future
        auto keys = other.keys();
        for (const auto& key : boost::adaptors::reverse(keys)) {
            this->operator()(key);
        }
    }
};
template <typename F>
inline ConcurrentLru<F> make_concurrent_lru(F&& fun, size_t max = 10) {
    return ConcurrentLru<F>(std::forward<F>(fun), max);
}

}  // namespace navitia
