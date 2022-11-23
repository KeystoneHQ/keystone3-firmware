#include <dlg/dlg.hpp>
#include <thread>
#include <chrono>
#include <iostream>

// mainly used to test that everything works multithreaded and that
// there will be no leaks
void test()
{
    dlg_info("main");
    dlg_info("initing thread 2");

    std::thread t([&]() {
        dlg_info("hello world from thread 2");
        dlg_warn("Just a small {}", "warning");
        dlg_info("Goodbye from thread 2");
        dlg_info("thread 2 id: {}", std::this_thread::get_id());

        for (auto i = 0u; i < 10; ++i) {
            dlg_warn("Some nice yellow warning (hopefully)");
            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        }
    });

    dlg_error("Just some messages from the main thread");
    dlg_info("thread 1 id: {}", std::this_thread::get_id());
    dlg_info("Waiting on thread 2 to finally spawn...");
    dlg_info("Hurry ffs!");
    dlg_info("Well i guess i go to sleep for now...");
    dlg_info("Wake me up then!");

    for (auto i = 0u; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::nanoseconds(i * 10));
        dlg_info("Did he say something already?");
    }

    dlg_info("Hopefully he said something by now...");
    dlg_info("Goodbye from main thread");

    t.join();
}

int main()
{
    // default handler (threadsafe)
    std::cout << " =================== 1 (threadsafe) ==================== \n";
    test();

    // custom handler (threadsafe)
    std::cout << " =================== 2 (threadsafe) ==================== \n";
    auto tid1 = std::this_thread::get_id();
    dlg::set_handler([&](const struct dlg_origin & origin, const char* str) {
        auto t = (std::this_thread::get_id() == tid1) ? "thread-1: " : "thread-2: ";
        dlg_win_init_ansi();
        dlg_fprintf(stdout, "%s %s", t, dlg::generic_output(~0u, origin, str).c_str());
    });
    test();

    // custom handler (not threadsafe)
    std::cout << " =================== 3 (not threasafe) ==================== \n";
    dlg::set_handler([&](const struct dlg_origin & origin, const char* str) {
        unsigned int features = dlg_output_file_line | dlg_output_newline |
                                dlg_output_style;
        dlg_generic_output_stream(stdout, features, &origin, str, dlg_default_output_styles);
    });
    test();
}
