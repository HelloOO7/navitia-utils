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

#pragma once
#include "backtrace.h"

#include <string>
#include <exception>
#include <utility>

namespace navitia {
class exception : public std::exception {
protected:
    std::string msg;
    std::string _backtrace;

public:
    exception(std::string msg) : msg(std::move(msg)), _backtrace(get_backtrace()) {}
    exception() = default;
    exception(const exception&) = default;
    exception& operator=(const exception&) = default;
    ~exception() noexcept override;

    const char* what() const noexcept override { return msg.c_str(); }
    const std::string& backtrace() const noexcept { return _backtrace; }
};

/**
 * non fatal exception
 *
 * Used in kraken for exemple if the worker can continue to work after having caught the exception
 *
 */
struct recoverable_exception : public exception {
    recoverable_exception(const std::string& msg) : exception(msg) {}
    recoverable_exception() = default;
    recoverable_exception(const recoverable_exception&) = default;
    recoverable_exception& operator=(const recoverable_exception&) = default;
    ~recoverable_exception() noexcept override;
};
}  // namespace navitia
