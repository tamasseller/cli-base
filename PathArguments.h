/*******************************************************************************
 *
 * Copyright (c) 2021 Tamás Seller. All rights reserved.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *******************************************************************************/

#ifndef CLI_BASE_PATHARGUMENTS_H_
#define CLI_BASE_PATHARGUMENTS_H_

#include "ArgumentReader.h"

#include <filesystem>

struct FilePath: std::filesystem::path {
	using path::path;
};

template<> struct ArgumentParser<FilePath>
{
	static constexpr const auto typeName = "file";

	template<class It>
	static inline std::string parse(It& it, const It &end)
	{
		if(it != end)
		{
			return *it++;
		}

		throw std::runtime_error("missing file name argument argument");
	}

	static inline std::pair<int, std::list<std::string>> suggest() {
		return {1, {std::list<std::string>{}}};
	}
};

struct DirectoryPath: std::filesystem::path {
	using path::path;
};

template<> struct ArgumentParser<DirectoryPath>
{
	static constexpr const auto typeName = "directory";

	template<class It>
	static inline std::string parse(It& it, const It &end)
	{
		if(it != end)
		{
			return *it++;
		}

		throw std::runtime_error("missing directory name argument argument");
	}

	static inline std::pair<int, std::list<std::string>> suggest() {
		return {2, {std::list<std::string>{}}};
	}
};



#endif /* CLI_BASE_PATHARGUMENTS_H_ */
