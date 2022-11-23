#define DLG_DEFAULT_TAGS "dlg"

#include <dlg/dlg.hpp>
#include <dlg/output.h>
#include <iomanip>
#include <iostream>
#include <cstdio>

unsigned int gerror = 0;

// TODO: pretty much todo... test all of the header correctly

#define EXPECT(a) if(!(a)) { \
    printf("$$$ Expect '" #a "' failed [%d]\n", __LINE__); \
    ++gerror; \
}

int main()
{
    // utility functions
    EXPECT(dlg::rformat("$", "\\$\\") == "$");
    EXPECT(dlg::rformat("$", "$\\", 0) == "0\\");
    EXPECT(dlg::rformat("$", "\\$", 1) == "\\1");
    EXPECT(dlg::rformat("$", "\\$\\$\\", 2) == "$2\\");
    EXPECT(dlg::rformat("@", "@", "ayyy") == "ayyy");
    EXPECT(dlg::format("{{}}", 2) == "{2}");
    EXPECT(dlg::format("\\{}\\") == "{}");
    EXPECT(dlg::format("\\{{}}\\", 2) == "\\{2}\\");

    std::string a;
    for (auto i = 0; i < 1000; ++i) {
        a += std::to_string(i);
    }

    EXPECT(dlg::detail::tlformat("{}", a) == a);
    EXPECT(dlg::detail::tlformat(42) == std::string("42"));
    EXPECT(dlg::detail::tlformat("ỦŤ₣8 ťéŝť ŝťяïאָğ")
           == std::string("ỦŤ₣8 ťéŝť ŝťяïאָğ"));

    // TODO: more output.h testing
    {
        dlg_origin origin {};

        size_t size;
        dlg_generic_output_buf(nullptr, &size, 0, &origin,
                               "test", dlg_default_output_styles);
        EXPECT(size == 4u);

        auto str = dlg::generic_output(0, origin, "test string");
        EXPECT(str == "test string");
    }

    std::string str;
    str += 'a';
    str += 'b';
    str += 'c';
    str += '\0';
    str += 'd';
    str += 'e';
    str += 'f';
    dlg_info("{} xyz", str);

    // check output
    enum check {
        check_line = 1,
        check_tags = 2,
        check_expr = 4,
        check_string = 8,
        check_level = 16,
        check_fire = 32
    };

    struct {
        unsigned int check;
        const char* str;
        bool fired;
    } expected {};

    dlg::set_handler([&](const struct dlg_origin & origin, const char* str) {
        expected.fired = true;
        if (expected.check & check_string) {
            if ((str == nullptr) != (expected.str == nullptr) ||
                    (str && std::strcmp(str, expected.str) != 0)) {
                std::printf("$$$ handler: invalid string [%d]\n", origin.line);
                ++gerror;
            }
        }

        // output
        dlg_win_init_ansi();
        dlg_generic_output_stream(nullptr, ~0u, &origin, str, dlg_default_output_styles);
    });

    {
        dlg_tags("a", "b");
        expected.check &= check_string;
        expected.str = "Just some formatted info";
        dlg_infot(("tag1", "tag2"), "Just some {} info", "formatted");
    }

    expected = {};
    dlg_warnt(("tag2", "tag3"), "Just some {} warning: {} {}", "sick", std::setw(10), 69);
    dlg_assertm(true, "eeeehhh... {}", "wtf");

    dlg_info("We can also just log objects");
    dlg_info(42);

    str = "should fire... {} {}";
    dlg_assertm(false, str, "!", 24);

    auto entered = false;
    dlg_checkt(("checked"), {
        entered = true;
        dlg_info("from inside the check block");
        EXPECT(expected.fired);
    });
    EXPECT(entered);

    return gerror;
}
