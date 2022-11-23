#define DLG_DISABLE 1
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
    dlg_info("well, this is not printed");
    EXPECT(!gdata.fired);
    return gerror;
}
