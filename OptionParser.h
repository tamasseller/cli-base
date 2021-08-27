#ifndef LIBUNO_TEST_OPTIONPARSER_H_
#define LIBUNO_TEST_OPTIONPARSER_H_

#include <map>
#include <list>
#include <memory>
#include <functional>

#include <cassert>

/**
 * Option parsing and usage information generator utility for CLI.
 */
struct OptionParser
{
	using cListIter = std::list<std::string>::const_iterator;

	/// Argument value parser for option callbacks.
	template<class T, class D = void> struct ArgumentParser;

	template<class D> struct ArgumentParser<std::string, D>
	{
		static constexpr const auto typeName = "text";

		static inline std::string parse(cListIter& it, cListIter end)
		{
			if(it == end)
				throw std::runtime_error("missing string argument");

			return *it++;
		}
	};

	template<class D> struct ArgumentParser<int, D>
	{
		static constexpr const auto typeName = "int";

		static inline int parse(cListIter& it, cListIter end)
		{
			if(it == end)
				throw std::runtime_error("missing integer argument");

			return std::stoi(*it++);
		}
	};

	template<class D> struct ArgumentParser<unsigned int, D>
	{
		static constexpr const auto typeName = "uint";

		static inline unsigned int parse(cListIter& it, cListIter end)
		{
			if(it == end)
				throw std::runtime_error("missing integer argument");

			return std::stoul(*it++);
		}
	};

	template<class D> struct ArgumentParser<float, D>
	{
		static constexpr const auto typeName = "float";

		static inline int parse(cListIter& it, cListIter end)
		{
			if(it == end)
				throw std::runtime_error("missing integer argument");

			return std::stof(*it++);
		}
	};

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
		 * the first argument, which needs to be incremented if used. The second one
		 * is the end of the input sequence.
		 */
		const std::function<void(cListIter&, cListIter)> parse;

		/// Forwarding constructor
		Option(const decltype(description) &description, decltype(optionTypes) &&optionTypes, decltype(parse) &&parse):
			description(description), optionTypes(optionTypes), parse(parse) {}
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

	/**
	 * Helper used to invoke the correct
	 */
	template<class Obj, class... Args>
	static inline void parseOptions(void (Obj::* method)(Args...) const, const Obj& obj, cListIter& it, cListIter end)
	{
		(obj.*method)(ArgumentParser<std::remove_const_t<std::remove_reference_t<Args>>>::parse(it, end)...);
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
	bool processArgs(const std::list<std::string>& args);

	/**
	 * Add a user callback that is called when an option (specified
	 * with a set of keys and description) is encountered.
	 */
	template<class C>
	inline void addOptions(const std::list<std::string>& names, const std::string& description, C&& c)
	{
		auto opt = std::make_shared<Option>(description, optionTypes(&C::operator()), std::function([c{std::forward<C>(c)}](cListIter& it, cListIter end)
		{
			parseOptions(&C::operator(), c, it, end);
		}));

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

#endif /* LIBUNO_TEST_OPTIONPARSER_H_ */
