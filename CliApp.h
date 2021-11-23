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

#ifndef CLI_BASE_CLIAPP_H_
#define CLI_BASE_CLIAPP_H_

#include "OptionParser.h"

#include <map>
#include <list>
#include <string>

/**
 * Polymorphic top level base class (and static registry) for all utility applets.
 */
class CliApp
{
	friend int main(int argc, const char* argv[]);
	friend class Autocompleter;

	/// Global registry of applets.
	static inline std::map<std::string, class CliApp*> apps;

	/// Entry point of an applet.
	virtual int operator()(int argc, const char* argv[]) = 0;

	/// Get description of the applet.
	virtual const char* getDesc() const = 0;

	/// Get whether the applet should be visible.
	virtual bool visibleByDefault() const = 0;

	/// Autocompletion entry point.
	virtual std::list<std::string> autocomplete(std::list<std::string>::iterator from, std::list<std::string>::iterator to) = 0;

protected:
	/// Registers an applet in the global registry.
	inline CliApp(const char* name) {
		apps.insert({name, this});
	}

public:
	virtual ~CliApp() = default;
	static int main(int argc, const char* argv[]);
};

/**
 * CRTP intermediate base for applets that provides static instance.
 *
 * NOTE: The constructor of the subclass must call the dummy method
 * on the instance member in order for the automatic inclusion in
 * the final binary to happen.
 */
template<class Child>
class CliAppBase: CliApp, OptionParser
{
	/// Stored arguments, for applet processing.
	std::list<std::string> args;

	/// Makes processCommandLine return error no matter what.
	bool dryRun = false;

	/// Forward description of the applet from CRTP child.
	virtual const char* getDesc() const final override {
		return Child::appDesc;
	}

	/// Forward visibility information of the applet from CRTP child.
	virtual bool visibleByDefault() const final override {
		return true;
	}

protected:
	/// Static (singleton) instance of the applet class.
	static inline Child instance;

	/// Linker hack: must be called from constructor of applet subclass.
	inline void dummy() {}

	/// Constructor that forwards static applet name and description strings from Child class.
	inline CliAppBase(): CliApp(Child::appName),
		OptionParser(std::string(Child::appDesc) + "\nUsage: " + Child::appName + " [options]")  {}

	/// Process stored arguments (proxy for child)
	inline std::optional<std::list<std::string>> processCommandLine()
	{
		if(dryRun)
		{
			return std::nullopt;
		}

		return this->processArgs(args);
	}

	/// Entry point of the applet.
	inline virtual int operator()(int argc, const char* argv[]) final override
	{
		args = {argv, argv + argc};
		return static_cast<Child*>(this)->run();
	}

	/// Autocompletion entry point.
	inline virtual std::list<std::string> autocomplete(std::list<std::string>::iterator from, std::list<std::string>::iterator to) final override
	{
		dryRun = true;
		static_cast<Child*>(this)->run();
		std::list<std::string> ret;
		std::transform(options.begin(), options.end(), std::back_inserter(ret), [](const auto& p) {return p.first; });
		return ret;
	}

public:
	using OptionParser::addOption;
	using OptionParser::addOptions;

	virtual ~CliAppBase() = default;
};

#define CLI_APP(name, desc)												\
struct CliApp_##name: CliAppBase<CliApp_##name>  						\
{																		\
	static constexpr const char* appName = #name;						\
	static constexpr const char* appDesc = desc;						\
	virtual ~CliApp_##name() = default;									\
																		\
	CliApp_##name() {	 CliAppBase::instance.CliAppBase::dummy(); } 	\
																		\
    int run();															\
};																		\
																		\
int CliApp_##name::run()


#endif /* CLI_BASE_CLIAPP_H_ */
