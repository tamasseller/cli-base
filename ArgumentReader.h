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

#ifndef CLI_BASE_ARGUMENTREADER_H_
#define CLI_BASE_ARGUMENTREADER_H_

#include <list>
#include <string>
#include <stdexcept>
#include <filesystem>

/// Argument value parser for option callbacks.
template<class T> struct ArgumentParser;

template<> struct ArgumentParser<std::string>
{
	static constexpr const auto typeName = "text";

	template<class It>
	static inline std::string parse(It& it, const It &end)
	{
		if(it != end)
		{
			return *it++;
		}

		throw std::runtime_error("missing string argument");
	}

	static inline std::pair<int, std::list<std::string>> suggest() {
		return {0, {typeName}};
	}
};

template<> struct ArgumentParser<int>
{
	static constexpr const auto typeName = "int";

	template<class It>
	static inline int parse(It& it, const It& end)
	{
		if(it != end)
		{
			return std::stoi(*it++);
		}

		throw std::runtime_error("missing integer argument");
	}

	static inline std::pair<int, std::list<std::string>> suggest() {
		return {0, {typeName}};
	}
};

template<> struct ArgumentParser<unsigned int>
{
	static constexpr const auto typeName = "uint";

	template<class It>
	static inline unsigned int parse(It& it, const It& end)
	{
		if(it != end)
		{
			return std::stoul(*it++);
		}

		throw std::runtime_error("missing unsigned integer argument");
	}

	static inline std::pair<int, std::list<std::string>> suggest() {
		return {0, {typeName}};
	}
};

template<> struct ArgumentParser<float>
{
	static constexpr const auto typeName = "float";

	template<class It>
	static inline int parse(It& it, const It& end)
	{
		if(it != end)
		{
			return std::stof(*it++);
		}

		throw std::runtime_error("missing integer argument");
	}

	static inline std::pair<int, std::list<std::string>> suggest() {
		return {0, {typeName}};
	}
};

#endif /* CLI_BASE_ARGUMENTREADER_H_ */
