Notes
=====

# General

- The default of DLG_FILE (stripping DLG_BASE_PATH (if existent) from the current file)
  will not really compare DLG_BASE_PATH with the file name but just skip its length
- Adding your project name as tag to all calls coming from your library is probably
  a good idea. It can either be done defining DLG_DEFAULT_TAGS e.g. from the
  build system (prefer this method).
  Or add macros that add the tags like this (usually not a good idea):

```c
#define MY_ADD_TAGS(tags, ...) (MY_EVAL tags, __VA_ARGS__)
#define MY_EVAL(...) __VA_ARGS__

#define my_warn(...) dlg_warnt(("my"), __VA_ARGS__)
#deinfe my_warnt(tags, ...) dlg_warnt(MY_ADD_TAGS(tags, "my"), __VA_ARGS__)
```

__TODO__: Add some examples on how tags could be used

# Windows/msvc troubleshooting

Windows and msvc are fully supported (and tested) by dlg.
We try to work around most of the... issues on windows by there are a few
things you have to keep in mind for dlg to work.

- msvc (at the state of 2017, still) does not allow/correctly handle utf-8 string literals
	- this means ```dlg_fprintf(stdout, u8"äü")``` will not work by default since the
	  string passed there is not utf-8 encoded
	- The above example can be made to work using the ```/utf-8``` switch on msvc
- note that dlg always passes filepaths in their native representation so with msvc
  the filepath will have backslashes (important if you e.g. want to handle logging
  calls from different files differently)
- meson on windows: if you want to define DLG_BASE_PATH using meson you will have
  to work around the backslashes (e.g. in the path returned from ```meson.source_root()```)
  sine those would be interpreted as invalid escape characters and there is not meson
  function to escape backslashes correctly (as of 2017).

  DLG itself handles it this way:

```meson
source_root = meson.source_root().split('\\')
add_project_arguments('-DDLG_BASE_PATH="' + '/'.join(source_root) + '/"', language: 'c')
```

## C-api tag guards

```c
	#define dlg_tag_base(global, tags, code) { \
		const char* _dlg_tag_tags[] = {DLG_EVAL tags}; \
		const char** _dlg_tag_ptr = _dlg_tag_tags;; \
		const char* _dlg_tag_func = global ? NULL : __FUNCTION__; \
		while(_dlg_tag_ptr)  \
			dlg_add_tag(_dlg_tag_ptr++, _dlg_tag_func); \
		code \
		_dlg_tag_ptr = _dlg_tag_tags; \
		while(_dlg_tag_ptr) \
			 dlg_remove_tag(_dlg_tag_ptr++, _dlg_tag_func); \
	}

#if DLG_CHECK
	#define dlg_check(code) { code }
	#define dlg_checkt(tags, code) dlg_tag(tags, code)
#else
	#define dlg_check(code)
	#define dlg_checkt(tags, code)
#endif // DLG_CHECK

#define dlg_tag(tags, code) dlg_tag_base(false, tags, code)
#define dlg_tag_global(tags, code) dlg_tag_base(true, tags, code)
```

## Idea: Custom assert macro handler

```c
// Can be specified to a custom assert handler/checker.
// Will be called for every failed assertion. If it returns true,
// the default failed assertion log will be done by dlg, otherwise
// no further action will be taken.
// Must have the signature 'bool (const char* expr)'.
// The default (dlg__assertion_failed) just always returns false.
// Useful as breakpoint.
#ifndef DLG_ASSERT_FILTER
	#define DLG_ASSERT_FILTER dlg__assertion_failed
#endif

bool dlg_assertion_failed(const char* expr);

//
#define DLG__CHECK_ASSERT(lvl, expr) \
	(level > DLG_ASSERT_LEVEL && !(expr) && DLG_ASSERT_FILER(#expr))

#define dlg_assertl(level, expr) if(DLG__CHECK_ASSERT(level, expr)) \
	dlg__do_log(level, DLG_CREATE_TAGS(NULL), DLG_FILE, __LINE__, __func__, NULL, #expr)

// ...
```
