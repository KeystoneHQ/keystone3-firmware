Up-to-date api
==============

# Synopsis of dlg.h

## Config macros with default and semantics:

```c
// Define this macro to make all dlg macros have no effect at all
// #define DLG_DISABLE

// the log/assertion levels below which logs/assertions are ignored
#define DLG_LOG_LEVEL dlg_level_trace
#define DLG_ASSERT_LEVEL dlg_level_trace

// the assert level of dlg_assert
#define DLG_DEFAULT_ASSERT dlg_level_error

// evaluated to the 'file' member in dlg_origin
#define DLG_FILE dlg__strip_root_path(__FILE__, DLG_BASE_PATH)

// the base path stripped from __FILE__. If you don't override DLG_FILE set this to
// the project root to make 'main.c' from '/some/bullshit/main.c'
#define DLG_BASE_PATH ""

// Default tags applied to all logs/assertions (in the defining file).
// Must be in format ```#define DLG_DEFAULT_TAGS "tag1", "tag2"```
// or just nothing (as defaulted here)
#define DLG_DEFAULT_TAGS

// The function used for formatting. Can have any signature, but must be callable with
// the arguments the log/assertions macros are called with. Must return a const char*
// that will not be freed by dlg, the formatting function must keep track of it.
// The formatting function might use dlg_thread_buffer or a custom owned buffer.
// The returned const char* has to be valid until the dlg log/assertion ends.
// Usually a c function with ... (i.e. using va_list) or a variadic c++ template do
// allow formatting.
#define DLG_FMT_FUNC <function with printf-like semantics>
```

## Core macros:

```c
// Tagged/Untagged logging with variable level
// Tags must always be in the format `("tag1", "tag2")` (including brackets)
#define dlg_log(level, ...)
#define dlg_logt(level, tags, ...)

// Dynamic level assert macros in various versions for additional arguments
#define dlg_assertl(level, expr) // assert without tags/message
#define dlg_assertlt(level, tags, expr) // assert with tags
#define dlg_assertlm(level, expr, ...) // assert with message
#define dlg_assertltm(level, tags, expr, ...) // assert with tags & message

// Static leveled logging
#define dlg_trace(...)
#define dlg_debug(...)
#define dlg_info(...)
#define dlg_warn(...)
#define dlg_error(...)
#define dlg_fatal(...)

// Tagged leveled logging
#define dlg_tracet(tags, ...)
#define dlg_debugt(tags, ...)
#define dlg_infot(tags, ...)
#define dlg_warnt(tags, ...)
#define dlg_errort(tags, ...)
#define dlg_fatalt(tags, ...)

// Assert macros useing DLG_DEFAULT_ASSERT as level
#define dlg_assert(expr)
#define dlg_assertt(tags, expr)
#define dlg_assertm(expr, ...)
#define dlg_asserttm(tags, expr, ...)
```

## Other

The tag order in which they are stored in origin.tags (all of them preserved in order):

- DLG_DEFAULT_TAGS
- current tags set via dlg_add_tag
- tags used in the specific call

```c
// Represents the importance of a log/assertion call.
enum dlg_level {
	dlg_level_trace = 0, // temporary used debug, e.g. to check if control reaches function
	dlg_level_debug, // general debugging, prints e.g. all major events
	dlg_level_info, // general useful information
	dlg_level_warn, // warning, something went wrong but might have no (really bad) side effect
	dlg_level_error, // something really went wrong; expect serious issues
	dlg_level_fatal // critical error; application is likely to crash/exit
};

// Holds various information associated with a log/assertion call.
// Forwarded to the output handler.
struct dlg_origin {
	const char* file;
	unsigned int line;
	const char* func;
	enum dlg_level level;
	const char** tags; // null-terminated
	const char* expr; // assertion expression, otherwise null
};

// Type of the output handler, see dlg_set_handler.
typedef void(*dlg_handler)(const struct dlg_origin* origin, const char* string, void* data);

// Sets the handler that is responsible for formatting and outputting log calls.
// This function is not thread safe and the handler is set globally.
// The handler itself must not change dlg tags or call a dlg macro.
// The handler can also be used for various other things such as dealing
// with failed assertions or filtering calls based on the passed tags.
// The default handler is dlg_default_output (see its doc for more info).
// If using c++ make sure the registered handler cannot throw e.g. by
// wrapping everything into a try-catch blog.
void dlg_set_handler(dlg_handler handler, void* data);

// Returns the currently active dlg handler and sets `data` to
// its user data pointer. `data` must not be NULL.
// Useful to create handler chains.
// This function is not threadsafe, i.e. retrieving the handler while
// changing it from another thread is unsafe.
// See `dlg_set_handler`.
dlg_handler dlg_get_handler(void** data);

// The default output handler. Pass a valid FILE* as stream or NULL to use stderr/stdout.
// Simply calls dlg_generic_output from dlg/output.h with the file_line feature enabled,
// the style feature enabled if the stream is a console (and if on windows ansi mode could
// be set) and dlg_default_output_styles as styles.
// It also flushes the stream used.
void dlg_default_output(const struct dlg_origin* origin, const char* string, void* stream);

// Adds the given tag associated with the given function to the thread specific list.
// If func is not NULL the tag will only applied to calls from the same function.
// Remove the tag again calling dlg_remove_tag (with exactly the same pointers!).
// Does not check if the tag is already present.
void dlg_add_tag(const char* tag, const char* func);

// Removes a tag added with dlg_add_tag (has no effect for tags no present).
// The pointers must be exactly the same pointers that were supplied to dlg_add_tag,
// this function will not check using strcmp. When the same tag/func combination
// is added multiple times, this function remove exactly one candidate, it is
// undefined which. Returns whether a tag was found (and removed).
bool dlg_remove_tag(const char* tag, const char* func);

// Returns the thread-specific buffer and its size for dlg.
// The buffer should only be used by formatting functions.
// The buffer can be reallocated and the size changed, just make sure
// to update both values correctly.
char** dlg_thread_buffer(size_t** size);
```

# Synopsis of output.h


```c
// Text style
enum dlg_text_style {
	dlg_text_style_reset     = 0,
	dlg_text_style_bold      = 1,
	dlg_text_style_dim       = 2,
	dlg_text_style_italic    = 3,
	dlg_text_style_underline = 4,
	dlg_text_style_blink     = 5,
	dlg_text_style_rblink    = 6,
	dlg_text_style_reversed  = 7,
	dlg_text_style_conceal   = 8,
	dlg_text_style_crossed   = 9,
	dlg_text_style_none,
};

// Text color
enum dlg_color {
	dlg_color_black = 0,
	dlg_color_red,
	dlg_color_green,
	dlg_color_yellow,
	dlg_color_blue,
	dlg_color_magenta,
	dlg_color_cyan,
	dlg_color_gray,
	dlg_color_reset = 9,

	dlg_color_black2 = 60,
	dlg_color_red2,
	dlg_color_green2,
	dlg_color_yellow2,
	dlg_color_blue2,
	dlg_color_magenta2,
	dlg_color_cyan2,
	dlg_color_gray2,

	dlg_color_none = 69,
};

struct dlg_style {
	enum dlg_text_style style;
	enum dlg_color fg;
	enum dlg_color bg;
};

// Like fprintf but fixes utf-8 output to console on windows.
// On non-windows sytems just uses the corresponding standard library
// functions. On windows, if dlg was compiled with the win_console option,
// will first try to output it in a way that allows the default console
// to display utf-8. If that fails, will fall back to the standard
// library functions.
int dlg_fprintf(FILE* stream, const char* format, ...);
int dlg_vfprintf(FILE* stream, const char* format, va_list list);

// Like dlg_printf, but also applies the given style to this output.
// The style will always be applied (using escape sequences), independent of the given stream.
// On windows escape sequences don't work out of the box, see dlg_win_init_ansi().
int dlg_styled_fprintf(FILE* stream, const struct dlg_style style,
	const char* format, ...);

// Features to output from the generic output handler.
// Some features might have only an effect in the specializations.
enum dlg_output_feature {
	dlg_output_tags = 1, // output tags list
	dlg_output_time = 2, // output time of log call (hour:minute:second)
	dlg_output_style = 4, // whether to use the supplied styles
	dlg_output_func = 8, // output function
	dlg_output_file_line = 16, // output file:line,
	dlg_output_newline = 32, // output a newline at the end
	dlg_output_threadsafe = 64, // locks stream before printing
	dlg_output_time_msecs = 128 // output micro seconds (ms on windows)
};

// The default level-dependent output styles. The array values represent the styles
// to be used for the associated level (i.e. [0] for trace level).
const struct dlg_style dlg_default_output_styles[6];

// Generic output function. Used by the default output handler and might be useful
// for custom output handlers (that don't want to manually format the output).
// Will call the given output func with the given data (and format + args to print)
// for everything it has to print in printf format.
// See also the *_stream and *_buf specializations for common usage.
// The given output function must not be NULL.
typedef void(*dlg_generic_output_handler)(void* data, const char* format, ...);
void dlg_generic_output(dlg_generic_output_handler output, void* data,
		unsigned int features, const struct dlg_origin* origin, const char* string,
		const struct dlg_style styles[6]);

// Generic output function. Used by the default output handler and might be useful
// for custom output handlers (that don't want to manually format the output).
// If stream is NULL uses stdout.
// Automatically uses dlg_fprintf to assure correct utf-8 even on windows consoles.
// Locks the stream (i.e. assures threadsafe access) when the associated feature
// is passed (note that stdout/stderr might still mix from multiple threads).
void dlg_generic_output_stream(FILE* stream, unsigned int features,
	const struct dlg_origin* origin, const char* string,
	const struct dlg_style styles[6]);

// Generic output function (see dlg_generic_output) that uses a buffer instead of
// a stream. buf must at least point to *size bytes. Will set *size to the number
// of bytes written (capped to the given size), if buf == NULL will set *size
// to the needed size. The size parameter must not be NULL.
void dlg_generic_output_buf(char* buf, size_t* size, unsigned int features,
	const struct dlg_origin* origin, const char* string,
	const struct dlg_style styles[6]);

// Returns if the given stream is a tty. Useful for custom output handlers
// e.g. to determine whether to use color.
// NOTE: Due to windows limitations currently returns false for wsl ttys.
bool dlg_is_tty(FILE* stream);

// Returns the null-terminated escape sequence for the given style into buf.
// Undefined behvaiour if any member of style has a value outside its enum range (will
// probably result in a buffer overflow or garbage being printed).
// If all member of style are 'none' will simply nullterminate the first buf char.
void dlg_escape_sequence(const struct dlg_style style, char buf[12]);

// The reset style escape sequence.
const char* dlg_reset_sequence;

// Just returns true without other effect on non-windows systems or if dlg
// was compiled without the win_console option.
// On windows tries to set the console mode to ansi to make escape sequences work.
// This works only on newer windows 10 versions. Returns false on error.
// Only the first call to it will have an effect, following calls just return the result.
// The function is threadsafe. Automatically called by the default output handler.
// This will only be able to set the mode for the stdout and stderr consoles, so
// other streams to consoles will still not work.
bool dlg_win_init_ansi(void);
```

# Synopsis of dlg.hpp

```cpp
// By default this header automatically uses a different, typesafe formatting
// function. Make sure to never include dlg.h in your translation unit before
// including dlg.hpp to make this work.
// The new formatting function works like a type-safe version of printf, see dlg::format.
// It can also be called with only an object (e.g. dlg_info(42)) which will
// then simply output the object.
#ifndef DLG_FMT_FUNC
	#define DLG_FMT_FUNC <format function with dlg::format like semantics>
#endif

// The default string to replace by the dlg::*format functions.
// Used as default by tlformat (set as new DLG_FMT_FUNC) or dlg::format.
// If a custom replace string is required in certain situations without
// overriding this macro, use dlg::rformat or dlg::gformat.
#ifndef DLG_FORMAT_DEFAULT_REPLACE
	#define DLG_FORMAT_DEFAULT_REPLACE "{}"
#endif

namespace dlg {

// Sets dlg tags on its construction and removes them on its destruction.
// Instead of explicitly constructing an object, just use the dlg_tags and
// dlg_tags_global macros which will construct one in the current scope.
// Just forwards the arguments on construction to dlg_add_tag, so if func
// is nullptr the tags will be applied even to called functions from the current
// scope, otherwise only to calls coming directly from the current function.
class TagsGuard {
public:
	TagsGuard(const char** tags, const char* func);
	~TagsGuard();

protected:
	const char** tags_;
	const char* func_;
};

// Constructs a dlg::TagsGuard in the current scope, passing correctly the
// current function, i.e. only dlg calls made from other functions
// that are called in the current scope will not use the given tags.
// Expects the tags to be set as parameters like this:
// ```dlg_tags("tag1", "tag2")```.
#define dlg_tags(...)

// Constructs a dlg::TagsGuard in the current scope, passing nullptr as func.
// This means that even dlg calls made from other functions called in the current
// scope will use those tags.
// Expects the tags to be set as parameters like this:
// ```dlg_tags_global("tag1", "tag2")```.
#define dlg_tags_global(...)

// Executes the given block only if dlg checking is enabled
#define dlg_check(code)

// Executes the given blocks with the given tags only if dlg checking is enabled
// The tags must have the default `("tag1", "tag2")` format.
#define dlg_checkt(tags, code)

/// Alternative output handler that allows to e.g. set lambdas or member functions.
using Handler = std::function<void(const struct dlg_origin& origin, const char* str)>;

// Allows to set a std::function as dlg handler.
// The handler should not throw, all exceptions (and non-exceptions) are caught
// in a wrapper since they must not be passed through dlg (since it's c and dlg
// might be called from c code).
void set_handler(Handler handler);

/// Generic version of dlg::format, allows to set the special string sequence
/// to be replaced with arguments instead of using DLG_FORMAT_DEFAULT_REPLACE.
/// Simply replaces all occurrences of 'replace' in 'fmt' with the given
/// arguments (as printed using operator<< with an ostream) in order and
/// prints everything to the given ostream.
/// Throws std::invalid_argument if there are too few or too many arguments.
/// If you want to print the replace string without being replaced, wrap
/// it into backslashes (\\). If you want to print your own types, simply
/// overload operator<< for ostream correctly. The replace string itself
/// must not be a backslash character.
///  - gformat("%", "number: '%', string: '%'", 42, "aye"); -> "number: '42', string: 'aye'"
///  - gformat("{}", "{} replaced: \\{}\\", "not"); -> "not replaced: {}"
///  - gformat("@", "@ @", 1); -> std::invalid_argument, too few arguments
///  - gformat("#", "#", 1, 2); -> std::invalid_argument, too many arguments
///  - gformat("$", "$ $", std::setw(5), 2); -> "     2"
template<typename... Args>
void gformat(std::ostream& os, const char* replace, const char* fmt, Args&&... args);

/// Simply calls gformat with a local stringstream and returns the stringstreams
/// contents.
template<typename Arg, typename... Args>
std::string rformat(const char* replace, const char* fmt, Args&&... args);

/// Simply calls rformat with DLG_FORMAT_DEFAULT_REPLACE (defaulted to '{}') as
/// replace string.
template<typename... Args>
std::string format(const char* fmt, Args&&... args);

/// Specialization of dlg_generic_output that returns a std::string.
std::string generic_output(unsigned int features,
	const struct dlg_origin& origin, const char* string,
	const struct dlg_style styles[6] = dlg_default_output_styles);

} // namespace dlg
```

