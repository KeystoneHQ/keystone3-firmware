#define DLG_LOG_LEVEL dlg_level_warn
#define DLG_ASSERT_LEVEL dlg_level_debug
#include <dlg/dlg.h>

struct {
    bool fired;
} gdata = {
    .fired = false
};

#define EXPECT(a) if(!(a)) { \
    printf("$$$ Expect '" #a "' failed [%d]\n", __LINE__); \
    ++gerror; \
}

unsigned int gerror = 0;

void custom_handler(const struct dlg_origin* origin, const char* string, void* data)
{
    (void) origin;
    (void) string;
    (void) data;
    gdata.fired = true;
}

int main()
{
    dlg_set_handler(custom_handler, NULL);

    // log
    dlg_trace("well, this is not printed");
    dlg_debug("well, this is not printed");
    dlg_info("well, this is not printed");
    EXPECT(!gdata.fired);

    dlg_warn("well, this is printed");
    EXPECT(gdata.fired);
    gdata.fired = false;

    dlg_error("well, this is printed");
    EXPECT(gdata.fired);
    gdata.fired = false;

    dlg_fatal("well, this is printed");
    EXPECT(gdata.fired);
    gdata.fired = false;

    // assert
    dlg_assertlm(dlg_level_trace, false, "well, this is not printed");
    EXPECT(!gdata.fired);

    dlg_assertlm(dlg_level_debug, false, "well, this is printed");
    EXPECT(gdata.fired);
    gdata.fired = false;

    dlg_assertlm(dlg_level_warn, false, "well, this is printed");
    EXPECT(gdata.fired);
    gdata.fired = false;

    dlg_assertlm(dlg_level_error, false, "well, this is printed");
    EXPECT(gdata.fired);
    gdata.fired = false;

    dlg_assertlm(dlg_level_fatal, false, "well, this is printed");
    EXPECT(gdata.fired);
    gdata.fired = false;

    return gerror;
}
