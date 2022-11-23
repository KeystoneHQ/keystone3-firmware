# Ideas and todo

- [x] return int from dlg printf wrappers
- [x] windows utf-8 output (see ny)
- [x] windows text style support
- [x] assert without error message
- [x] add real example and screenshot
- [x] custom (changeable) base paths (for nested projects/header calls) __[DLG_FILE]__
- [x] possibility to get current scope (or more general: exception support)
- [x] make default scope signs customizable by macro
- [x] unit tests (at least some basic stuff) + ci (travis)
- [x] c example
	- [ ] Could be extended. Examples can generally be improved/reworked
- [x] extend testing (mainly tag setting scope)
	- test all macros (also disable and stuff)
- [x] fix c++ example
- [x] release version 0.2.0
- [x] add appveyor testing (for mingw as well as visual studio)
- [x] decide on whether to catch exceptions from assert expressions. Config variable?
	- Yeah, don't do it. We are c now
- [x] make dlg.c valid c++ (mainly casting issues atm)
- [x] update README picture
- [ ] Fix all todos in dlg.c (mainly error handling questions)
	- [ ] Check GetLastError with winapi functions?
- [x] contrib file that implements android log handler (using android liblog)
- [ ] make wsl output faster. It currently triggers a WriteConsole error on
      every output and then falls back to default, performing the formatting
	  twice
- [ ] get stable (or make a list of what to do for 1.0)

### Kinda trashed ideas

Just because you can something, doesn't mean you should, right?

- [ ] Default dummy platform (fallback if neither unix nor windows detected)
- [ ] rework/further strip fmt.hpp
	- [ ] since it is parsed to some type-erased list anyways, don't include the whole header
	- [ ] ~~constexpr string parsing~~ __[Not really worth it/fully possible i guess]__
		- [ ] ~~warn about format issues~~
		- [ ] ~~warn about unused but passed variables~~
- [ ] add field to Origin that determines whether the origin is inside a dlg::check block?
- [ ] add at least really simply pattern matching utility function for tags?
- [ ] make dlg_assert return false on failure
	- not that easy to accomplish actually since we use an if, if we would use ? : we would get
	  an unused expression warning if it is not used. Not worth it
- [ ] compile time format specifier validation (c++) instead of exceptions
	- really no way to implement it that is at least somewhat safe
- [ ] assert_failed function (maybe as c symbol) that can be easily used as breakpoint
	- [ ] Also custom assertion handler? that is called inline (macro) and might throw?
	- [ ] example for custom failed assertion handle, i.e. print backtrace/exception/abort
	- Decided not worth it since all of this can already be achieved in the log
	  handler. If you want an assert failed breakpoint, just check for expr
	  in the log handler and depending on level call a function to
	  set a breakpoint on.
	  See notes.md for some first concepts though
