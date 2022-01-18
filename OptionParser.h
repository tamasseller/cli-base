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

#ifndef CLI_BASE_OPTIONPARSER_H_
#define CLI_BASE_OPTIONPARSER_H_

#include <map>
#include <list>
#include <memory>
#include <optional>
#include <functional>
#include <string>
#include <initializer_list>

#include <cassert>

#include "ArgumentReader.h"

/**
 * Option parsing and usage information generator utility for CLI.
 */
struct OptionParser
{
	using cListIter = std::list<std::string>::const_iterator;

	/**
	 * A command line option.
	 */
	struct Option
	{
		/// The description of the option to be displayed to the user in usage info.
		const std::string description;

		/// The data types of the arguments expected by the option.
		const std::list<std::string> optionTypes;

		/**
		 * Argument parser callback.
		 *
		 * Reads arguments and calls registered user method, invoked when the option
		 * key is matched. First argument is a reference to an iterator pointing to
		 * the first argument, which is incremented when a value is used. The second
		 * one is the end of the input sequence.
		 */
		const std::function<void(cListIter&, cListIter)> parse;

		/**
		 * Argument suggestion callback.
		 *
		 * Tries to consume arguments, similarly to the parse method but if it runs
		 * out of values calls the suggestion candidate generator for the type of the
		 * missing argument and returns the suggested values as a list of strings.
		 * If all arguments are present returns an empty list. The first argument is
		 * a reference to an iterator pointing to the first argument, which is moved
		 * forward as values are used up. The second one is the end of the input sequence.
		 */
		const std::function<std::optional<std::pair<int, std::list<std::string>>>(cListIter&, cListIter)> suggest;

		/// Forwarding constructor
		Option(const decltype(description) &description, decltype(optionTypes) &&optionTypes, decltype(parse) &&parse, decltype(suggest) &&suggest):
			description(description), optionTypes(optionTypes), parse(parse), suggest(suggest) {}
	};

	/**
	 * The collection option keys registered for parsing.
	 *
	 * Multiple entries may correspond to the same option if multiple
	 * keys are added (e.g. long and short names, like -h and --help).
	 */
	std::map<std::string, std::shared_ptr<Option>> options;

	/**
	 * The header string to be printed at the beginning of
	 * the usage information page.
	 */
	const std::string header;

	struct CallArgumentEvaluationSequencingHelper
	{
		template<class C, class... Args>
		inline constexpr CallArgumentEvaluationSequencingHelper(C&& c, Args&&... args)
		{
			c(std::forward<Args>(args)...);
		}
	};

	/**
	 * Helper used to invoke the correct argument readers.
	 */
	template<class Obj, class... Args>
	static inline void parseOptions(void (Obj::* method)(Args...) const, const Obj& obj, cListIter& it, cListIter end)
	{
		CallArgumentEvaluationSequencingHelper{
			[&obj, &method](auto&&... x){ (obj.*method)(std::forward<decltype(x)>(x)...); },
			ArgumentParser<std::remove_const_t<std::remove_reference_t<Args>>>::parse(it, end)...
		};
	}

	template<class T>
	static inline std::optional<std::pair<int, std::list<std::string>>> suggestHelper(cListIter& it, cListIter end)
	{
		if(it == end)
		{
			return ArgumentParser<T>::suggest();
		}

		return std::nullopt;
	}

	/**
	 * Helper used to invoke the correct argument value candidate generators.
	 */
	template<class Obj, class... Args>
	static inline std::optional<std::pair<int, std::list<std::string>>> generateArgumentCandidates(void (Obj::* method)(Args...) const, cListIter& it, cListIter end)
	{
		std::optional<std::pair<int, std::list<std::string>>> ret;

		CallArgumentEvaluationSequencingHelper{
			[&ret](auto&&... x)
			{
				std::optional<std::pair<int, std::list<std::string>>> vs[] = {std::forward<decltype(x)>(x)...};

				for(const auto v: vs)
				{
					if(v.has_value())
					{
						ret = v;
						break;
					}
				}
			},
			suggestHelper<std::remove_const_t<std::remove_reference_t<Args>>>(it, end)...
		};

		return ret;
	}

	/**
	 * Generates the names of argument types for an option.
	 */
	template<class Obj, class... Args>
	static inline auto optionTypes(void (Obj::* method)(Args...) const)
	{
		return std::list<std::string>{ArgumentParser<std::remove_const_t<std::remove_reference_t<Args>>>::typeName...};
	}

public:
	/**
	 * Create a parser that has a single option to display usage page.
	 *
	 * The pre-installed option can be invoked using the -h or --help
	 * keys, has no further arguments and prints usage information as
	 * expected.
	 */
	OptionParser(const std::string &header);

	/**
	 * Process the command line arguments (expected in the form of a
	 * list of strings) and invoke registered option callbacks when
	 * matches are found.
	 *
	 * If there is an error during parsing it print error message to
	 * standard error and returns false.
	 *
	 * If all arguments are parsed successfully **and** the usage page
	 * is not requested with -h or --help then it returns true.
	 */
	std::optional<std::list<std::string>> processArgs(const std::list<std::string>& args);

	/**
	 * Add a user callback that is called when an option (specified
	 * with a set of keys and description) is encountered.
	 */
	template<class C>
	inline void addOptions(const std::list<std::string>& names, const std::string& description, C&& c)
	{
		auto opt = std::make_shared<Option>(
				description,
				optionTypes(&C::operator()),
				std::function([c{std::forward<C>(c)}](cListIter& it, cListIter end) { parseOptions(&C::operator(), c, it, end); }),
				std::function([](cListIter& it, cListIter end) { return generateArgumentCandidates(&C::operator(), it, end); })
		);

		for(auto n: names)
		{
			auto result = options.insert(std::make_pair(n, opt));
			assert(result.second);
		}
	}

	/**
	 * Add a user callback that is called when an option (specified
	 * with a single key and a description) is encountered.
	 */
	template<class C>
	inline void addOption(const std::string& name, const std::string& description, C&& c) {
		addOptions(std::list<std::string>{name}, description, std::forward<C>(c));
	}
};

#endif /* CLI_BASE_OPTIONPARSER_H_ */
