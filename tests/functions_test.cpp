/* Copyright © 2001-2014, Hove and/or its affiliates. All rights reserved.

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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE functions_test
#include <boost/test/unit_test.hpp>
#include "utils/functions.h"

BOOST_AUTO_TEST_CASE(regex_strip_accents_and_lower_tests) {

    std::string test("républiquê");
    BOOST_CHECK_EQUAL(navitia::strip_accents(test), "republique");
    test = "RÉpUblIquê";
    BOOST_CHECK_EQUAL(navitia::strip_accents_and_lower(test), "republique");
    test = "ÂâÄäçÇÉéÈèÊêËëÖöÔôÜüÎîÏïæœ";
    BOOST_CHECK_EQUAL(navitia::strip_accents(test), "aaaacceeeeeeeeoooouuiiiiaeoe");
}

BOOST_AUTO_TEST_CASE(trim) {
    std::string test(" ab ; cd ;k ; r");
    std::vector<std::string> res = split_string(test, ";");
    BOOST_CHECK_EQUAL(res.at(0), "ab");
    BOOST_CHECK_EQUAL(res.at(1), "cd");
    BOOST_CHECK_EQUAL(res.at(2), "k");
    BOOST_CHECK_EQUAL(res.at(3), "r");
}
