// Copyright (c) 2019 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef INC_DLG_DLG_HPP_
#define INC_DLG_DLG_HPP_


// By default this header automatically uses a different, typesafe formatting
// function. Make sure to never include dlg.h in your translation unit before
// including dlg.hpp to make this work.
// The new formatting function works like a type-safe version of printf, see dlg::format.
// TODO: override the default (via undef) if a dlg.h was included before this file?
#ifndef DLG_FMT_FUNC
	#define DLG_FMT_FUNC ::dlg::detail::tlformat
#endif

#include <dlg/dlg.h>
#include <dlg/output.h>

#include <algorithm>
#include <string>
#include <streambuf>
#include <ostream>
#include <functional>
#include <cstring>
#include <string>
#include <sstream>
#include <type_traits>

// Define this macro as 1 to disable all dlg_check* blocks.
// Note that checking is defined separately from DLG_DISABLE.
// By default this is defined to 1 if NDEBUG is defined OR DLG_DISABLe is 1.
#ifndef DLG_DISABLE_CHECK
	#if defined(NDEBUG) || defined(DLG_DISABLE)
		#define DLG_DISABLE_CHECK 1
	#else
		#define DLG_DISABLE_CHECK 0
	#endif
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
	TagsGuard(const char** tags, const char* func) : tags_(tags), func_(func) {
		while(*tags) {
			dlg_add_tag(*tags, func);
			++tags;
		}
	}

	~TagsGuard() {
		while(*tags_) {
			dlg_remove_tag(*tags_, func_);
			++tags_;
		}
	}

protected:
	const char** tags_;
	const char* func_;
};

#ifdef DLG_DISABLE
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
#else
	#define dlg_tags(...) \
		const char* _dlgtags_[] = {__VA_ARGS__, nullptr}; \
		::dlg::TagsGuard _dlgltg_(_dlgtags_, __func__)
	#define dlg_tags_global(...) \
		const char* _dlgtags_[] = {__VA_ARGS__, nullptr}; \
		::dlg::TagsGuard _dlggtg_(_dlgtags_.begin(), nullptr)
#endif

// TODO: move this to the c api? together with (a scoped version of) dlg_tags?
#if DLG_DISABLE_CHECK
	// Executes the given block only if dlg checking is enabled
	#define dlg_check(code)

	// Executes the given blocks with the given tags only if dlg checking is enabled
	// The tags must have the default `("tag1", "tag2")` format.
	#define dlg_checkt(tags, code)
#else
	#define dlg_check(code) do code while(0)
	#define dlg_checkt(tags, code) do { dlg_tags(tags); code } while(0)
#endif

/// Alternative output handler that allows to e.g. set lambdas or member functions.
using Handler = std::function<void(const struct dlg_origin& origin, const char* str)>;

/// Small std::string_view replacement that allows taking std::string
/// and const char* arguments
struct StringParam {
	StringParam(const std::string& s) : str(s.c_str()) {}
	StringParam(const char* s) : str(s) {}

	const char* str;
};

/// Allows to set a std::function as dlg handler.
/// The handler should not throw, all exceptions (and non-exceptions) are caught
/// in a wrapper since they must not be passed through dlg (since it's c and dlg
/// might be called from c code).
inline void set_handler(Handler handler);

// TODO: maybe don't use exceptions for wrong formats?

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
///  - gformat("{}", "not replaced: \\{}\\"); -> "not replaced: {}"
///  - gformat("$", "{} {}", 1); -> std::invalid_argument, too few arguments
///  - gformat("$", "{}", 1, 2); -> std::invalid_argument, too many arguments
///  - gformat("$", "{} {}", std::setw(5), 2); -> "     2"
template<typename Arg, typename... Args>
void gformat(std::ostream& os, StringParam replace, StringParam fmt, Arg&& arg, Args&&... args);
inline void gformat(std::ostream& os, StringParam replace, StringParam fmt);

/// Simply calls gformat with a local stringstream and returns the stringstreams
/// contents.
template<typename... Args>
std::string rformat(StringParam replace, StringParam fmt, Args&&... args) {
	std::stringstream sstream;
	gformat(sstream, replace, fmt, std::forward<Args>(args)...);
	return sstream.str();
}

/// Simply calls rformat with DLG_FORMAT_DEFAULT_REPLACE (defaulted to '{}') as
/// replace string.
template<typename... Args>
std::string format(StringParam fmt, Args&&... args) {
	return rformat(DLG_FORMAT_DEFAULT_REPLACE, fmt, std::forward<Args>(args)...);
}

/// Specialization of dlg_generic_output that returns a std::string.
inline std::string generic_output(unsigned int features,
	const struct dlg_origin& origin, StringParam string,
	const struct dlg_style styles[6] = dlg_default_output_styles);


// - Private interface & implementation -
namespace detail {

inline void handler_wrapper(const struct dlg_origin* origin, const char* str, void* data) {
	auto& handler = *static_cast<Handler*>(data);
	try {
		handler(*origin, str);
	} catch(const std::exception& err) {
		fprintf(stderr, "dlg.hpp: handler has thrown exception: '%s'\n", err.what());
	} catch(...) {
		fprintf(stderr, "dlg.hpp: handler has thrown something else than std::exception");
	}
}

// Implements std::basic_streambuf into the dlg_thread_buffer.
class StreamBuffer : public std::basic_streambuf<char> {
public:
	StreamBuffer(char*& buf, std::size_t& size) : buf_(buf), size_(size) {
		setp(buf, buf + size); // we will only read from it
	}

	~StreamBuffer() {
		// make sure only the end has a null terminator
		setp(std::remove(buf_, pptr(), '\0'), buf_ + size_);
		sputc('\0');
	}

	int_type overflow(int_type ch = traits_type::eof()) override {
		if(pptr() >= epptr()) {
			auto off = pptr() - buf_;
			size_ = size_ * 2 + 1;
			buf_ = static_cast<char*>(std::realloc(buf_, size_));
			if(!buf_) {
				size_ = 0;
				setp(nullptr, nullptr);
				return traits_type::eof();
			}

			setp(buf_ + off, buf_ + size_);
		}

		if(!traits_type::eq_int_type(ch, traits_type::eof())) {
			*pptr() = (char_type) ch;
			pbump(1);
		}

		return 0;
	}

protected:
	char*& buf_;
	size_t& size_;
};

// Like std::strstr but only matches if target is not wrapped in backslashes,
// otherwise prints the target.
inline const char* find_next(std::ostream& os, const char*& src, const char* target) {
	auto len = std::strlen(target);
	const char* next = std::strstr(src, target);
	while(next && next > src && *(next - 1) == '\\' && *(next + len) == '\\') {
		os.write(src, next - src - 1);
		os.write(target, len);
		src = next + len + 1;
		next = std::strstr(next + 1, target);
	}

	return next;
}

// Used as DLG_FMT_FUNC, uses a threadlocal stringstream to not allocate
// a new buffer on every call
template<typename... Args>
const char* tlformat(StringParam fmt, Args&&... args) {
	{
		std::size_t* size;
		char** dbuf = dlg_thread_buffer(&size);
		detail::StreamBuffer buf(*dbuf, *size);
		std::ostream output(&buf);
		gformat(output, DLG_FORMAT_DEFAULT_REPLACE, fmt, std::forward<Args>(args)...);
	}

	return *dlg_thread_buffer(nullptr);
}

template<typename Arg, typename = typename std::enable_if<
	!std::is_convertible<Arg, StringParam>::value>::type>
const char* tlformat(Arg arg) {
	{
		std::size_t* size;
		char** dbuf = dlg_thread_buffer(&size);
		detail::StreamBuffer buf(*dbuf, *size);
		std::ostream output(&buf);
		output << arg;
	}

	return *dlg_thread_buffer(nullptr);
}

} // namespace detail

void gformat(std::ostream& os, StringParam replace, StringParam fmt) {
	if(detail::find_next(os, fmt.str, replace.str)) {
		throw std::invalid_argument("Too few arguments given to format");
	}

	os << fmt.str;
}

template<typename Arg, typename... Args>
void gformat(std::ostream& os, StringParam replace, StringParam fmt, Arg&& arg, Args&&... args) {
	const char* next = detail::find_next(os, fmt.str, replace.str);
	if(!next) {
		throw std::invalid_argument("Too many arguments to format supplied");
	}

	// XXX: any drawback compared to formatted?
	// this allows to use temporary ostream modifiers only to real arguments
	os.write(fmt.str, next - fmt.str);
	os << std::forward<Arg>(arg);
	auto len = std::strlen(replace.str);
	return gformat(os, replace, next + len, std::forward<Args>(args)...);
}

void set_handler(Handler handler) {
	static Handler handler_;
	handler_ = std::move(handler);
	dlg_set_handler(&detail::handler_wrapper, &handler_);
}

std::string generic_output(unsigned int features,
		const struct dlg_origin& origin, StringParam string,
		const struct dlg_style styles[6]) {
	std::size_t size;
	dlg_generic_output_buf(nullptr, &size, features, &origin,
		string.str, styles);
	std::string ret(++size, ' ');

	// NOTE: this might (theoretically) cause problems before C++17
	//  but this function should return a string and all compilers
	//  make it work. If you want to go sure, just use C++17
	//  where it is guaranteed that this works (everybody is
	//  using it anyways so shit will blow up if this fails)
	dlg_generic_output_buf(&ret[0], &size, features, &origin,
		string.str, styles);
	ret.pop_back(); // terminating null-char
	return ret;
}

} // namespace dlg

#endif // header guard
