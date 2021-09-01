#include "CliApp.h"

#include <iostream>
#include <algorithm>
#include <iterator>
#include <numeric>

#include <libgen.h>

inline size_t levenshteinDistance(std::string_view a, std::string_view b)
{
	std::unique_ptr<size_t[]> prev(new size_t[b.length() + 1]), next(new size_t[b.length() + 1]);

	for(auto j = 0u; j <= b.length(); j++)
	{
		prev[j] = j;
	}

	for(auto i = 0u; i < a.length(); i++)
	{
		next[0] = i;

		for(auto j = 0u; j < b.length(); j++)
		{
			next[j + 1] = (a[i] == b[j]) ? prev[j] : 1 + std::min({prev[j], prev[j + 1], next[j]});
		}

		std::swap(prev, next);
	}

	return prev[b.length()];
}

int CliApp::main(int argc, const char* argv[])
{
	if(argc > 1)
	{
		const auto requested = argv[1];
		if(auto it = apps.find(requested); it != apps.end())
		{
			return (*it->second)(argc - 2, argv + 2);
		}
		else
		{
			std::cerr << "Unknown operation: '" << argv[1] << "'" << std::endl;

			std::list<std::pair<size_t, std::string>> lDists;
			std::transform(CliApp::apps.begin(), CliApp::apps.end(), std::back_inserter(lDists), [requested](const auto& l) {
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

		const auto maxLen = std::accumulate(CliApp::apps.begin(), CliApp::apps.end(), size_t(0), [](size_t l, const auto& p) {
			return std::max(l, p.first.length());
		});

		std::transform(CliApp::apps.begin(), CliApp::apps.end(), std::ostream_iterator<std::string>(std::cerr, "\n"), [maxLen](const auto& l)
		{
			return std::string(maxLen + 6 - l.first.length(), ' ') + l.first + "  -  " + l.second->getDesc();
		});

		std::cerr << std::endl << std::endl;
	}

	return -1;
}
