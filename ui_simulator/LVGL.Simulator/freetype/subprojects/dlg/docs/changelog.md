2019-12-07
	- Add dlg_get_handler to allow handler chaining for debugging
	  [api addition]
	- dlg.h previously required DLG_DISABLE to be defined to 1, this
	  was already documented differently in api.md and examples.
	  Now it's enough if DLG_DISABLE is defined at all
	  [breaking change]
	- rework dlg__strip_root_path to actually check for prefix.
	  If the base path isn't a prefix of the file, we won't strip it.
	- change naming of header guard to not start with underscore

=== Release of v0.2.2 ===

2018-5-20
	Rework the tags array building macro due to new errors in gcc8.
	Seems like the C++ way was undefined behavior before, now uses
	initializer_list.

2018-3-20
	Fix default_output_handler output on wsl, add option to disable special
	windows console handling. Also add options to always use color in
	default output handler (to work around wsl color limitation).

2018-3-19
	dlg.hpp: Allow tlformat (default dlg.hpp formatter) to be called with just a
	non-string object (which will then just be printed)

2018-01-17
	output.h: add dlg_output_msecs for more time output precision

=== Release of v0.2.1 ===
