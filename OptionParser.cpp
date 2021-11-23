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

#include "OptionParser.h"
#include "Levenshtein.h"

#include <iostream>
#include <numeric>
#include <algorithm>

struct SimplyExit {};

OptionParser::OptionParser(const std::string &header): header(header)
{
	addOptions({"-h", "--help"}, "Displays information about available options", [this]()
	{
		std::multimap<std::shared_ptr<Option>, std::string> reverse;
		std::transform(options.begin(), options.end(), std::inserter(reverse, reverse.end()), [](const auto &p){ return std::make_pair(p.second, p.first);});
		std::map<std::shared_ptr<Option>, std::vector<std::string>> grouped;

		while(!reverse.empty())
		{
			auto range = reverse.equal_range(reverse.begin()->first);
			std::vector<std::string> names;
			std::transform(range.first, range.second, std::back_inserter(names), [](const auto &p){ return p.second;});
			std::sort(names.begin(), names.end(), [](const auto& a, const auto& b)
			{
				if(a.length() < b.length())
					return true;
				else if(a.length() == b.length())
					return a < b;
				else
					return false;
			});
			grouped.insert({range.first->first, names});
			reverse.erase(range.first, range.second);
		}

		std::map<std::string, std::pair<std::string, std::string>> flattened;
		std::transform(grouped.begin(), grouped.end(), std::inserter(flattened, flattened.end()), [](const auto &p)
		{
			auto flat = std::accumulate(p.second.begin(), p.second.end(), std::string("   "), [](const auto& a, const auto& b){
				return a + " " + b;
			});

			std::list<std::string> bracketed;
			std::transform(p.first->optionTypes.begin(), p.first->optionTypes.end(), std::back_inserter(bracketed), [](const auto &a) {
				return "<" + a + ">";
			});

			auto options = std::accumulate(bracketed.begin(), bracketed.end(), std::string{}, [](const auto& a, const auto& b){
				return a + " " + b;
			});

			return std::make_pair(flat, std::make_pair(options, p.first->description));
		});

		auto longestNameIt = std::max_element(flattened.begin(), flattened.end(), [](const auto &a, const auto &b){ return a.first.length() < b.first.length(); });
		auto maxNameLength = longestNameIt->first.length();

		auto longestOptionsIt = std::max_element(flattened.begin(), flattened.end(), [](const auto &a, const auto &b){ return a.second.first.length() < b.second.first.length(); });
		auto maxOptionsLength = longestOptionsIt->second.first.length();

		std::cerr << this->header << std::endl << std::endl;
		std::cerr << "Options: "<< std::endl;

		for(auto& o: flattened)
		{
			const auto names = o.first;
			const auto options = o.second.first;

			std::cerr << names << std::string(maxNameLength - names.length() + 1, ' ')
					<< options << std::string(maxOptionsLength - options.length() + 1, ' ') << o.second.second << std::endl;
		}

		std::cerr << std::endl;

		throw SimplyExit{};
	});
}

std::optional<std::list<std::string>> OptionParser::processArgs(const std::list<std::string>& args)
{
	std::list<std::string> ret;

	for(auto it = args.cbegin(); it != args.cend();)
	{
		const auto name = *it++;

		if(const auto opt = options.find(name); opt == options.end())
		{
			if(name.length() > 1 && name[0] == '-')
			{
				std::cerr << "Unknown option: '" << name << "' use -h or --help flag to display usage information" << std::endl;

				std::list<std::pair<size_t, std::string>> lDists;

				std::transform(options.begin(), options.end(), std::back_inserter(lDists), [name](const auto& l) {
					return std::make_pair(levenshteinDistance(name, l.first), l.first);
				});

				const auto suggested = std::min_element(lDists.begin(), lDists.end(), [](const auto& a, const auto& b){return a.first < b.first;});
				std::cerr << std::endl << "Did you mean: " << suggested->second << "?" << std::endl;

				return std::nullopt;
			}
			else
			{
				ret.push_back(name);
			}
		}
		else
		{
			try
			{
				opt->second->parse(it, args.cend());
			}
			catch(const std::exception &e)
			{
				std::cerr << "Could not process option " << name  << ": " << e.what() << std::endl;
				return std::nullopt;
			}
			catch(const SimplyExit&)
			{
				return std::nullopt;
			}
		}
	}

	return ret;
}
