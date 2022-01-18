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

#include "CliApp.h"

#include <iostream>
#include <sstream>

struct Autocompleter: CliApp
{
	static Autocompleter instance;
	inline void dummy() {}
	virtual ~Autocompleter() = default;
	Autocompleter(): CliApp("_autocomplete") {instance.dummy(); }

	virtual const char* getDesc() const override { return "Autocomplete helper"; };

	virtual bool visibleByDefault() const override { return false; }

	virtual std::pair<int, std::list<std::string>> autocomplete(std::list<std::string>::const_iterator from, std::list<std::string>::const_iterator to) override {
		return {-1, {"To understand recursion, you must first understand recursion"}};
	}

	virtual int operator()(int argc, const char* argv[])
	{
		try
		{
			std::list<std::string> nonOpt = {argv, argv + argc};

			if(nonOpt.size() >= 2)
			{
				auto it = nonOpt.begin();

				const auto wordIdx = std::stoul(*it++);
				assert(wordIdx > 0);

				const auto binaryName = *it++;

				std::list<std::string> args;
				for(auto i = 1u; i < wordIdx; i++)
				{
					if(it == nonOpt.end())
					{
						break;
					}

					args.push_back(std::move(*it++));
				}

				if(args.empty())
				{
					for(const auto& a: ::CliApp::apps)
					{
						if(a.second->visibleByDefault())
						{
							std::cout << a.first << std::endl;
						}
					}
				}
				else
				{
					auto argIt = args.cbegin();
					if(auto appIt = ::CliApp::apps.find(*argIt++); appIt != ::CliApp::apps.end())
					{
						auto ret = appIt->second->autocomplete(argIt, args.cend());

						for(const auto& a: ret.second)
						{
							std::cout << a << std::endl;
						}

						return ret.first;
					}
					else
					{
						return -1;
					}
				}

				return 0;
			}
		}
		catch(...) {}

		return -1;
	}
};

Autocompleter Autocompleter::instance;
