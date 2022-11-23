#include <dlg/dlg.h>
#include <string.h>

static dlg_handler old_handler;
static void* old_data;

void pre_handler(const struct dlg_origin* origin, const char* string, void* data)
{
    (void) data;
    if (origin->expr && strcmp(origin->expr, "10 + 10 == 100") == 0) {
        return;
    }

    old_handler(origin, string, old_data);
}

int main()
{
    old_handler = dlg_get_handler(&old_data);
    dlg_set_handler(pre_handler, NULL);
    dlg_assertm(false, "Obviously false");
    dlg_assertm(10 + 10 == 100, "This won't be printed due to pre_handler");
}
