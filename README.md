# 🛠️ cli-base 🛠️

Command line interface framework for C++ multi-function utility applications (like **git** or **docker**), with support for:

 - ℹ️ Automatic help generation,
 - ☑️ Command line option parsing,
 - ⏩ Tab completion,
 - 😏 Suggestions for mistyped commands based on minimal [Levenshtein distance](https://en.wikipedia.org/wiki/Levenshtein_distance),
 - ▶️ Applet dispatch in independent compilation units.
 
## Format

It supports parsing command line arguments with the following format:

```
<command> <function> [<options>] [<arguments>]
```

Where _command_ is the name used to invoke the program, _function_ is a name of a utility declared using the CLI_APP macro.
The rest is parsed similarly to _getopt_.

### Usage

The entry point of the program can be as simple as:

```c++
#include "CliApp.h"

int main(int argc, const char* argv[])
{
	return CliApp::main(argc, argv);
}
```

The _CliApp::main_ method executes the selected applet or writes suggestions to _stderr_ on error.
Entry points for applets can be defined using the CLI_APP macro, which creates a subclass of the _CliApp_ base and starts the definition of the entry point method.

```c++
#include "CliApp.h"

CLI_APP(awsome, "Awsome utility applet")
{
	bool quiet = false;
	addOptions({"-q", "--quiet"}, "Quiet mode", [&](){ quiet = true; });
	
	std::string name = "daq";
	addOptions({"-n", "--name"}, "The name of something", [&](const std::string& arg){ name = arg; });
	
	std::map<std::string, std::string> options;
	addOption("--config", "Additional configuration options", [&](const std::string& key, const std::string& value)
	{
		options.insert({key, value});
	});
	
	if(auto nonOptionArguments = processCommandLine())
	{
		doStuff(quite, name, options);
		return 0;
	}
	
	return -1;
}
```

The body of the method first registers argument handlers using the _addOption_ and _addOptions_ methods for a single named option and multiple aliases respectively.
The first argument defines the name(s), the second is description and the third is an object that has an _operator()_.
The arguments of the operator are used to deduce the required number and type of arguments for that option.

Then the _processCommandLine_ method is called to do the actual parsing.
It returns a list of strings that are the non option arguments for normal operation.

If an empty _optional_ is returned the applet must return an error value immediately.
The applet must refrain from doing any meaningful (observable) work before calling  _processCommandLine_, 
because the body is also invoked by the auxiliary functions in which case it returns an empty optional to make the applet exit immediately.

The _processCommandLine_ method returns an empty optional on error as well, in which case the diagnostic is already printed, 
so there is no need to handle this in the application code.
A proper return value indicating usage error must be returned either way.

## Completion script installation

Most of the completion logic is implemented inside the application using the hidden __autocomplete_ applet.
But it still needs to be glued together with the shell's autocompletion logic.
This is done by a simple script that invokes the magic autocompletion applet inside the application.

This script must be installed into the appropriate directory on the system.
There are multiple suitable locations based on the distribution and cofiguration. 
However the system wide installation directory can be queried using `pkg-config --variable=completionsdir bash-completion` which should yield `/usr/share/bash-completion/completions` on Debian based systems.

The supplied completion script is universal, meaning that it registers the completion function based on the name of script file itself, 
thus it needs to be renamed appropriately for the application.
For example the tool foobar would need:

```bash
cp bash-completion/completions/example-tool `pkg-config --variable=completionsdir bash-completion`/foobar
```

If the build system suppurts that the helper script can be simply symlinked into the application tree using the appropriate name.
