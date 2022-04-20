/* Copyright © 2001-2017, Hove and/or its affiliates. All rights reserved.

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

#include <boost/regex.hpp>

namespace navitia {

class wrong_coordinate : public std::runtime_error {
public:
    wrong_coordinate(const std::string& what) : std::runtime_error(what) {}
    wrong_coordinate(const wrong_coordinate&) = default;
    ~wrong_coordinate() noexcept override;
};

extern const boost::regex coord_regex;

/**
 *
 * return the coord as pair<lon, lat>
 */
std::pair<double, double> parse_coordinate(const std::string& uri);

}  // namespace navitia
