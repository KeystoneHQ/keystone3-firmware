// this is the probably most important config macro (for more see config.hpp):
// if this is defined all dlg macros will do nothing (literally nothing, they
// will not produce a single instruction).
// #define DLG_DISABLE

// for other macros see the header of dlg.h
// dlg.hpp has some additional settings
// we could e.g. set a custom file name, shorter than its in-project path
#define DLG_FILE "example.cpp"

#include <dlg/dlg.hpp> // dlg macros and basic stuff
#include <dlg/output.h> // needed for custom output handler set later

#include <iostream> // since we manually use std::cout
#include <fstream> // since we might write to a file later on
#include <algorithm> // std::find

int main()
{
    // dlg offers assert and log calls mainly
    // asserts work with or without message
    dlg_assert(true);

    // assertions with message have the 'm' prefix
    dlg_assertm(true, "This message should never be seen");

    // there are various log levels,
    // realized via static named macros
    dlg_trace("A trace log");
    dlg_warn("A warn log");
    dlg_debug("A debug log");
    dlg_info("An info log");
    dlg_error("An error log");
    dlg_fatal("A fatal log");

    // you can also attach tags to log and assert calls
    // they can (as later shown) be used to e.g. filter certain
    // messages or to decide where messages should go
    // the tagged macros have a 't' suffix
    dlg_infot(("main"), "Use dlg tags to show where a log call came from");
    dlg_infot(("main"), "We are in the main function");
    dlg_infot(("main"), "That can also be done using a scoped tag guard");

    {
        // This set those tags in the current scope
        // Use dlg_tags_global to also appply it to other functions
        // called from the given scope
        dlg_tags("dlg", "example", "main_sub");

        dlg_info("Now we are using the tags specified above");
        dlg_info("Btw, if this output is confusing for you, look at example.cpp");
        dlg_info("The tags applied are not printed by default");
        dlg_info("But we could output and use them (e.g. as filter) in a custom handler");
    }

    // we can also use formatting
    // the default c api uses printf semantics but by including dlg.hpp we override
    // them to a small typesafe replacement that simply replaces a given
    // sequence (see dlg.hpp for config) with the printed object.
    // Uses std::ostream with operator<< so should work for everything and is extendable.
    // You could also use your custom formatting function (like libfmt) here if you wish.
    // It is not included (anymore) to reduce bloat.
    dlg_debug("Let's switch to debug output for some variation!");
    dlg_debug("We can also {} strings", "format");
    dlg_debug("You can use [{}], [{}] or [{}] formatter", "printf",
              "our own fmtlib-like, typesafe", "your own custom");

    std::cout << "\n";
    std::cout << "This is a message sent to std::cout\n";
    std::cerr << "And this one sent to std::cerr\n";
    std::cout << "They should not muddle with dlg in any way, i hope!\n";
    std::cout << "(The empty lines above/below are done intentionally)\n";
    std::cout << "\n";

    // we can also filter out certain message, switch the outputs
    // we set the function that is called everytime somethin is to be outputted
    dlg::set_handler([](const struct dlg_origin & origin, const char* msg) {
        // don't print anything below warning messages
        // note that if this is done for performance reasons or statically project-wide
        // prefer to use the config macros since they will result in zero
        // compile and runtime overhead, this way will not
        if (origin.level < dlg_level_warn)
            return;

        // we can e.g. also filter out certain tags
        auto tend = origin.tags;
        while (*tend) ++tend;

        if (std::find(origin.tags, tend, std::string("filtered")) != tend)
            return;

        // depending on tags/type/level or even based on the messages content (probably
        // a bad idea) we an print the output to different streams/files
        // in this case we check if the origin has an expression associated, i.e. if
        // it came from an assert call and then write it without color to a file stream.
        // everything else is dumped with color to cout
        // NOTE: the simple color switch here will lead to troubles when cout
        // is e.g. redirected to a file (but could be useful again when using unix's 'less -R')
        std::ostream* os = &std::cout;
        bool use_color = true;
        if (origin.expr) {
            static std::ofstream log_file("example_assert_log.txt");
            if (log_file.is_open()) {
                os = &log_file;
                use_color = false;
            }
        }

        // we could add additional switches for e.g. file/line, tags or msg content

        // we call the generic output handler that will take care
        // of formatting the origin (tags/type/expression/level/file/line) and color.
        // We could print more stuff (like tags/time, see dlg/output.h) but stick
        // with some clean defaults here
        unsigned int features = dlg_output_file_line | dlg_output_newline;
        features |= use_color * dlg_output_style;
        (*os) << dlg::generic_output(features, origin, msg);
    });

    // test out our custom output handler
    // assertions go into a file now
    // there are also custom assertion levels
    // we can also choose the level dynamically by using the macros with the 'l' suffix
    dlg_assertl(dlg_level_debug, 42 * 42 == -42); // should not be printed at all, level too low
    dlg_assertl(dlg_level_fatal, "dlg"[0] == 42); // should be printed into file
    dlg_assertm(false, "Error assert message"); // default assert level is error, printed to file

    // test the tag filtering
    dlg_trace("ayyy, this is never shown"); // level too low, not printed
    dlg_warn("Anyone for some beautiful yellow color?"); // printed to cout

    // this is filtered in our handler due to its tag; not printed at all
    dlg_fatalt(("filtered"), "I am {}: feelsbadman", "useless");

    // example of a dynamic leveled, tagged log
    // dynamic level logs just use the 'log' macro
    dlg_logt(dlg_level_error, ("tag1", "tag2"), "error!");

    // reset the output handler
    dlg_set_handler(dlg_default_output, NULL);

    dlg_assertm(true == false, "Assertions are printed to cout again"); // printed to cout
    dlg_tracet(("filtered"), "I am printed again, yeay"); // printed to cout

    dlg_info("If you don't like the colors here, don't despair; they may be changed!");
    dlg_info("Also make sure to check out the example_assert_log file i printed to\n");

    dlg_fatal("What now follows are the important bottom lines");
    dlg_info("You must be really confused right now if you don't know what this program is");
    dlg_info("Just read the dlg example file which uses all this to show off");
    dlg_info("Now, it is time. I'm a busy program after all. A good day m'lady or my dear sir!");

    // some of the utility functions for outputting custom stuff
    auto style = dlg_default_output_styles[dlg_level_trace];
    dlg_styled_fprintf(stdout, style, "*tips fedora and flies away*\n");
}
