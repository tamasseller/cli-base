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
#include "Levenshtein.h"

#include <iostream>
#include <algorithm>
#include <iterator>
#include <numeric>

#include <libgen.h>

static constexpr const char* showAllEnvVarName = "CLI_BASE_SHOW_ALL";

int CliApp::main(int argc, const char* argv[])
{
	const bool allVisible = std::getenv(showAllEnvVarName);

	std::list<std::pair<std::string, CliApp*>> visibleApps;
	std::copy_if(CliApp::apps.begin(), CliApp::apps.end(), std::back_inserter(visibleApps), [allVisible](const auto& p) {
		return p.second->visibleByDefault() || allVisible;
	});

	if(argc > 1)
	{
		const auto requested = argv[1];
		if(auto it = CliApp::apps.find(requested); it != CliApp::apps.end())
		{
			return (*it->second)(argc - 2, argv + 2);
		}
		else
		{
			std::cerr << "Unknown operation: '" << argv[1] << "'" << std::endl;

			std::list<std::pair<size_t, std::string>> lDists;
			std::transform(visibleApps.begin(), visibleApps.end(), std::back_inserter(lDists), [requested](const auto& l) {
				return std::make_pair(levenshteinDistance(requested, l.first), l.first);
			});

			const auto suggested = std::min_element(lDists.begin(), lDists.end(), [](const auto& a, const auto& b){return a.first < b.first;});

			std::cerr << std::endl << "Did you mean: " << suggested->second << "?" << std::endl;
		}
	}
	else
	{
		std::cerr << "No operation requested." << std::endl;
		std::cerr << std::endl << "Usage: " << basename(const_cast<char*>(argv[0])) << " <operation> [options]" << std::endl;
		std::cerr << std::endl << "Available operations:" << std::endl << std::endl;

		const auto maxLen = std::accumulate(visibleApps.begin(), visibleApps.end(), size_t(0), [](size_t l, const auto& p) {
			return std::max(l, p.first.length());
		});

		std::transform(visibleApps.begin(), visibleApps.end(), std::ostream_iterator<std::string>(std::cerr, "\n"), [maxLen](const auto& l)
		{
			return std::string(maxLen + 6 - l.first.length(), ' ') + l.first + "  -  " + l.second->getDesc();
		});

		std::cerr << std::endl << std::endl;
	}

	return -1;
}
