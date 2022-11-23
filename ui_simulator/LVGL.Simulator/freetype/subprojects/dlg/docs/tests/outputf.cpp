#include <dlg/dlg.hpp>

void sample()
{
    dlg_trace("This is a trace");
    dlg_debug("This is a debug info");
    dlg_info("This is an info");
    dlg_warn("This is a warning");
    dlg_error("Errors are red");
    dlg_assertm(1 == 2, "Well, this assertion will probably {}...", "fail");
    dlg_infot(("tag1", "tag2"), "We can tag our stuff. Can be used to filter/redirect messages");
    dlg_asserttm(("tag3"), 3 == 2, "The same goes for asserts");
    dlg_info("Another feature: Utf-8 printing works automatically, even for שׁǐŉďốẅś consoles");
    dlg_fatal("This one is printed bold. For more information, read the example already");
}

int main()
{
    dlg::set_handler([&](const struct dlg_origin & origin, const char* str) {
        dlg_generic_outputf_stream(stdout, "%s%c\n", &origin, str, dlg_default_output_styles, false);
    });

    dlg_info("Using output handler 1, pretty simple");
    dlg_info("It should just output the messages in the appropriate styles");
    sample();

    std::printf("-------------------------------------------\n");

    dlg::set_handler([&](const struct dlg_origin & origin, const char* str) {
        dlg_generic_outputf_stream(stdout, "[%o %h:%m {%t} %f] %s%c\n", &origin, str, dlg_default_output_styles, false);
    });

    dlg_info("outputting pretty much all information now. Only message is styled");
    sample();
    std::fprintf(stdout, "Normal, non-dlg printf message. Should not be effected by style\n");

    std::printf("-------------------------------------------\n");


    dlg::set_handler([&](const struct dlg_origin & origin, const char* str) {
        dlg_generic_outputf_stream(stdout, "%s[%o %h:%m %f]%r %c %s[%t]%r\n", &origin, str, dlg_default_output_styles, false);
    });

    dlg_info("This time, only the meta information is styled");
    sample();
    std::fprintf(stdout, "Normal, non-dlg printf message. Should not be effected by style\n");
}
