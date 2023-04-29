/*
 * Copyright (c) 2016-2021 Belledonne Communications SARL.
 *
 * This file is part of bctoolbox.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BCTBX_UTILS_H
#define BCTBX_UTILS_H

#include <string>
#include <vector>
#include <utility>

#include "ortp/port.h"

namespace ortp {

	namespace Utils {
		ORTP_PUBLIC std::vector<std::string> split (const std::string &str, const std::string &delimiter);

		ORTP_PUBLIC inline std::vector<std::string> split (const std::string &str, char delimiter) {
			return split(str, std::string(1, delimiter));
		}

		template<typename T>
		inline const T &getEmptyConstRefObject () {
			static const T object{};
			return object; 
		}

		ORTP_PUBLIC std::string fold (const std::string &str);
		ORTP_PUBLIC std::string unfold (const std::string &str);
		
		// Replace all "from" by "to" in source. Use 'recursive' to avoid replacing what has been replaced.
		ORTP_PUBLIC void replace(std::string& source, const std::string& from, const std::string& to, const bool& recursive = true);
		
		// Return the current state of memory as a string. This is currently implemented only for Windows.
		ORTP_PUBLIC std::string getMemoryReportAsString();
		
// Replace const_cast in order to be adapted from types. Be carefull when using it.
		template <typename From>
		class auto_cast {
		public:
			explicit constexpr auto_cast(From const& t) noexcept : val { t }{}
		
			template <typename To>
			constexpr operator To() const noexcept(noexcept(const_cast<To>(std::declval<From>()))) {
				return const_cast<To>(val);
			}
		private:
			From const& val;
		};
	
	}

}

#endif /* BCTBX_UTILS_H */
