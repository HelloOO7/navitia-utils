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
#pragma once
#include "utils/functions.h"
#include "utils/idx_map.h"

#include <vector>
#include <unordered_map>

namespace navitia {

/*
 * Factory handling all collections for one object
 * Object must inherit from navitia::type::Header
 */
template <typename ObjType>
class ObjFactory {
private:
    using inner_vector = typename std::vector<std::unique_ptr<ObjType>>;
    using inner_map = typename std::unordered_map<std::string, ObjType*>;

    inner_vector vec;
    inner_map map;

    void reindex_and_erase(const uint idx) {
        // reindex idx elements
        for (uint i = idx + 1; i < vec.size(); ++i) {
            vec[i].get()->idx--;
        }
        // erase vec member
        vec.erase(vec.begin() + idx);
    }

public:
    using iterator = typename inner_vector::iterator;
    using const_iterator = typename inner_vector::const_iterator;
    using const_reference = ObjType const* const;

    template <typename... Args>
    ObjType* emplace(const std::string& uri, Args&&... args) {
        if (navitia::contains(map, uri)) {
            throw(std::logic_error(std::string("In ") + typeid(*this).name() + ": Object with same uri (" + uri
                                   + ") already stored."));
        }
        vec.push_back(std::make_unique<ObjType>(std::forward<Args>(args)...));
        ObjType* obj = vec.back().get();
        obj->idx = vec.size() - 1;
        obj->uri = uri;
        map[obj->uri] = obj;
        return obj;
    }

    template <typename... Args>
    ObjType* get_or_create(const std::string& uri, Args&&... args) {
        ObjType* obj = get_mut(uri);
        if (obj == nullptr) {
            obj = emplace(uri, args...);
        }
        return obj;
    }

    ObjType* insert(const std::string& uri, ObjType&& obj) { return emplace(uri, std::move(obj)); }

    const ObjType* operator[](const std::string& uri) const { return find_or_default(uri, map); }

    const ObjType* operator[](const Idx<ObjType>& idx) const {
        if (!exists(idx)) {
            return nullptr;
        }
        return vec[idx.val].get();
    }

    ObjType* get_mut(const std::string& uri) { return find_or_default(uri, map); }

    ObjType* get_mut(const Idx<ObjType>& idx) {
        if (!exists(idx)) {
            return nullptr;
        }
        return vec[idx.val].get();
    }

    bool exists(const std::string& uri) const { return (map.find(uri) != map.end()); }

    bool exists(const Idx<ObjType>& idx) const { return (idx.val < vec.size()); }

    bool erase(const Idx<ObjType>& idx) {
        if (!exists(idx)) {
            return false;
        }
        // erase map member
        const auto* elem_ptr = vec[idx.val].get();
        if (map.erase(elem_ptr->uri) == 0) {
            return false;
        }
        reindex_and_erase(idx.val);
        return true;
    }

    bool erase(const std::string& uri) {
        if (!exists(uri)) {
            return false;
        }
        const auto* elem_ptr = map.at(uri);
        // erase map member
        if (map.erase(uri) == 0) {
            return false;
        }
        reindex_and_erase(elem_ptr->idx);
        return true;
    }

    iterator begin() { return std::begin(vec); }
    const_iterator begin() const { return std::begin(vec); }
    iterator end() { return std::end(vec); }
    const_iterator end() const { return std::end(vec); }

    size_t size() const noexcept { return vec.size(); }

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& vec& map;
    }
};

}  // namespace navitia

template <typename T>
const T* find_or_default(const std::string& key, const navitia::ObjFactory<T>& m) {
    return m[key];
}
