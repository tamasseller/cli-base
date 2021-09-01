#ifndef LIBUNO_TEST_CLIAPP_H_
#define LIBUNO_TEST_CLIAPP_H_

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

	/// Global registry of applets.
	static inline std::map<std::string, class CliApp*> apps;

	/// Entry point of an applet.
	virtual int operator()(int argc, const char* argv[]) = 0;

	/// Get description of the applet.
	virtual const char* getDesc() = 0;

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

	/// Forward description of the applet from CRTP child.
	virtual const char* getDesc() override {
		return Child::appDesc;
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
	inline bool processCommandLine() {
		return this->processArgs(args);
	}

	/// Entry point of the applet.
	inline virtual int operator()(int argc, const char* argv[]) final override
	{
		args = {argv, argv + argc};
		return static_cast<Child*>(this)->run();
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


#endif /* LIBUNO_TEST_CLIAPP_H_ */
