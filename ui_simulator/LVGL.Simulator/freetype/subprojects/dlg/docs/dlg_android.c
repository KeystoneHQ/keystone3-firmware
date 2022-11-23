// Simple dlg output handler that uses the android ndk logging library.
// Include <dlg/dlg.hpp> and <android/log.h> and link its library.
// You probably want to rename the "<DLG ANDROID TAG>" with the name
// of your application/library.
//
// To use this in C simply use dlg_generic_output with the
// default compute-size-then-malloc pattern.

void output_handler(const dlg_origin& origin, const char* str)
{
    auto features = dlg_output_file_line;
    auto output = dlg::generic_output(features, origin, str);

    auto prio = ANDROID_LOG_DEFAULT;
    switch (origin.level) {
    case dlg_level_trace:
        [[fallthrough]]
    case dlg_level_debug:
        prio = ANDROID_LOG_DEBUG;
        break;
    case dlg_level_info:
        prio = ANDROID_LOG_INFO;
        break;
    case dlg_level_warn:
        prio = ANDROID_LOG_WARN;
        break;
    case dlg_level_error:
        prio = ANDROID_LOG_ERROR;
        break;
    case dlg_level_fatal:
        prio = ANDROID_LOG_FATAL;
        break;
    default:
        break;
    }

    __android_log_write(prio, "<DLG ANDRDOID TAG>", output.c_str());
}
