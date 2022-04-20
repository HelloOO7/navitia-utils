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
#include "conf.h"
#ifdef HAVE_ICONV_H
#include <iconv.h>
#include <string>

/// Classe permettant de convertir l'encodage de chaînes de caractères
class EncodingConverter {
public:
    EncodingConverter(const std::string& from, const std::string& to, size_t buffer_size);
    std::string convert(const std::string& str);
    virtual ~EncodingConverter();

private:
    iconv_t iconv_handler;
    char* iconv_input_buffer;
    char* iconv_output_buffer;
    size_t buffer_size;
};

#endif
