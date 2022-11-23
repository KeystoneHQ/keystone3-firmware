tests = [
	['core', 'core.c', []],
	['level', 'level.c', []],
	['cpp', 'cpp.cpp', []],
	['disabledc', 'disabled.c', []],
	['disabledcpp', 'disabled.cpp', []],
	['threads', 'threads.cpp', [dep_threads]],
	['outputf', 'outputf.cpp', []],
]

foreach test : tests
	test(test[0],
		executable(test[0], test[1],
			c_args: common_args,
			cpp_args: common_args,
			dependencies: test[2] + [dlg_dep]))
endforeach
