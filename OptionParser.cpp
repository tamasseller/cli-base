#include "OptionParser.h"

#include <iostream>
#include <numeric>

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

		std::cout << this->header << std::endl << std::endl;
		std::cout << "Options: "<< std::endl;

		for(auto& o: flattened)
		{
			const auto names = o.first;
			const auto options = o.second.first;

			std::cout << names << std::string(maxNameLength - names.length() + 1, ' ')
					<< options << std::string(maxOptionsLength - options.length() + 1, ' ') << o.second.second << std::endl;
		}

		throw SimplyExit{};
	});
}

bool OptionParser::processArgs(const std::list<std::string>& args)
{
	for(auto it = args.cbegin(); it != args.cend();)
	{
		const auto name = *it++;

		if(const auto opt = options.find(name); opt == options.end())
		{
			std::cerr << "Unknown option: " << name << std::endl;
			return false;
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
				return false;
			}
			catch(const SimplyExit&)
			{
				return false;
			}
		}
	}

	return true;
}
