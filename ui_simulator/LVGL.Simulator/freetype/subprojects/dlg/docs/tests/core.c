#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define DLG_DEFAULT_ASSERT dlg_level_fatal
#include <dlg/dlg.h>
#include <dlg/output.h>
#include <string.h>
#include <stdio.h>

// TODO: custom format functions (using dlg_thread_buffer)
//  other custom output handlers
//  some more output.h testing

enum check {
    check_line = 1,
    check_tags = 2,
    check_expr = 4,
    check_string = 8,
    check_level = 16,
    check_fire = 32
};

struct {
    enum check check;
    unsigned int line;
    const char** tags;
    const char* expr;
    const char* string;
    enum dlg_level level;
    bool fire;
    bool fired;
} gdata = {
    .check = check_line,
    .fired = false
};

unsigned int gerror = 0;
FILE* check_file;
FILE* check_formatted_file;

void custom_handler(const struct dlg_origin* origin, const char* string, void* data);

#define EXPECT(a) if(!(a)) { \
    printf("$$$ Expect '" #a "' failed [%d]\n", __LINE__); \
    ++gerror; \
}

void foo_log();
void foo_assert();

int main()
{
    printf("__FILE__: %s\n", __FILE__);

    dlg_log(dlg_level_trace, "trace %d", 1);
    dlg_log(dlg_level_debug, "debug %d", 1);
    dlg_log(dlg_level_info, "info %d", 1);
    dlg_log(dlg_level_warn, "warn %d", 1);
    dlg_log(dlg_level_error, "error %d", 1);
    dlg_log(dlg_level_fatal, "fatal %d", 1);
    printf("----\n"); // should have no color/style

    dlg_trace("%s %d", "trace", 2);
    dlg_debug("%s %d", "debug", 2);
    dlg_info("%s %d", "info", 2);
    dlg_warn("%s %d", "warn", 2);
    dlg_error("%s %d", "error", 2);
    dlg_fatal("%s %d", "fatal", 2);
    printf("----\n\n");

    dlg_assertl(dlg_level_trace, false);
    dlg_assertl(dlg_level_debug, false);
    dlg_assertl(dlg_level_info, false);
    dlg_assertl(dlg_level_warn, false);
    dlg_assertl(dlg_level_error, false);
    dlg_assertl(dlg_level_fatal, false);
    printf("----\n\n");

    dlg_assertlm(dlg_level_trace, false, "Should %s, %s", "fire", "trace");
    dlg_assertlm(dlg_level_debug, false, "Should %s, %s", "fire", "debug");
    dlg_assertlm(dlg_level_info, false, "Should %s, %s", "fire", "info");
    dlg_assertlm(dlg_level_warn, false, "Should %s, %s", "fire", "warn");
    dlg_assertlm(dlg_level_error, false, "Should %s, %s", "fire", "error");
    dlg_assertlm(dlg_level_fatal, false, "Should %s, %s", "fire", "fatal");
    printf("----\n\n");

    dlg_assertlm(dlg_level_trace, true, "Should %s, %s", "not fire", "trace");
    dlg_assertlm(dlg_level_debug, true, "Should %s, %s", "not fire", "debug");
    dlg_assertlm(dlg_level_info, true, "Should %s, %s", "not fire", "info");
    dlg_assertlm(dlg_level_warn, true, "Should %s, %s", "not fire", "warn");
    dlg_assertlm(dlg_level_error, true, "Should %s, %s", "not fire", "error");
    dlg_assertlm(dlg_level_fatal, true, "Should %s, %s", "not fire", "fatal");
    printf("----\n\n");

    dlg_set_handler(&custom_handler, &gdata);
    check_file = fopen("dlg_test_output.txt", "w");
    check_formatted_file = fopen("dlg_test_formatted_output.txt", "w");
    EXPECT(!dlg_is_tty(check_file));

    dlg_fprintf(check_file, u8"beginning of (some utf-8: äüß) %s", "test output file with default layout of dlg_features\n");
    dlg_fprintf(check_formatted_file, u8"beginning of (some utf-8: äüß) %s", "test output file with custom layout of dlg_features\n");

    // checks
    // logging
    const char* t1[] = {NULL};
    gdata.check = check_level | check_line | check_string | check_expr | check_tags | check_fire;
    gdata.expr = NULL;
    gdata.fire = true;
    gdata.tags = t1;

    gdata.fired = false;
    gdata.string = "trace 3";
    gdata.level = dlg_level_trace;
    gdata.line = __LINE__ + 1;
    dlg_trace("%s %d", "trace", 3);
    EXPECT(gdata.fired);

    gdata.fired = false;
    gdata.string = "debug 3";
    gdata.level = dlg_level_debug;
    gdata.line = __LINE__ + 1;
    dlg_debug("%s %d", "debug", 3);
    EXPECT(gdata.fired);

    gdata.fired = false;
    gdata.string = "info 3";
    gdata.level = dlg_level_info;
    gdata.line = __LINE__ + 1;
    dlg_info("%s %d", "info", 3);
    EXPECT(gdata.fired);

    gdata.fired = false;
    gdata.string = "warn 3";
    gdata.level = dlg_level_warn;
    gdata.line = __LINE__ + 1;
    dlg_warn("%s %d", "warn", 3);
    EXPECT(gdata.fired);

    gdata.fired = false;
    gdata.string = "error 3";
    gdata.level = dlg_level_error;
    gdata.line = __LINE__ + 1;
    dlg_error("%s %d", "error", 3);
    EXPECT(gdata.fired);

    gdata.fired = false;
    gdata.string = "fatal 3";
    gdata.level = dlg_level_fatal;
    gdata.line = __LINE__ + 1;
    dlg_fatal("%s %d", "fatal", 3);
    EXPECT(gdata.fired);
    printf("----\n\n");


    // assertion
    gdata.check &= ~check_line;
    gdata.string = NULL;
    gdata.level = DLG_DEFAULT_ASSERT;
    gdata.fire = true;
    gdata.fired = false;
    gdata.expr = "0 && 1";
    dlg_assert(0 && 1);
    EXPECT(gdata.fired);

    gdata.string = "assertion 1";
    gdata.expr = "1 && 0";
    dlg_assertm(1 && 0, "%s %d", "assertion", 1);

    gdata.fire = false;
    dlg_assert(true || false);

    gdata.string = "assertion failed";
    gdata.fire = true;
    gdata.level = dlg_level_warn;
    gdata.expr = "false && true";
    gdata.fired = false;
    dlg_assertlm(dlg_level_warn, false && true, "%s%s %s", "assert", "ion", "failed");
    EXPECT(gdata.fired);

    // tags
    // local, single
    const char* t2[] = {"tag1", NULL};
    gdata.check = check_tags;
    gdata.tags = t2;
    gdata.fired = false;
    dlg_infot(("tag1"), ".");
    EXPECT(gdata.fired);
    dlg_logt(dlg_level_warn, ("tag1"), ".");
    dlg_assertt(("tag1"), false);
    dlg_asserttm(("tag1"), false, ".");
    dlg_assertltm(dlg_level_warn, ("tag1"), false, ".");

    // local, multiple
    const char* t3[] = {"tag1", "tag2", NULL};
    gdata.tags = t3;
    gdata.fired = false;
    dlg_errort(("tag1", "tag2"), ".");
    EXPECT(gdata.fired);
    dlg_logt(dlg_level_trace, ("tag1", "tag2"), ".");
    dlg_assertt(("tag1", "tag2"), false);
    dlg_asserttm(("tag1", "tag2"), false, ".");
    dlg_assertltm(dlg_level_fatal, ("tag1", "tag2"), false, ".");

    // current
    const char* t4[] = {"lt", "gt", NULL};
    gdata.tags = t4;
    gdata.fired = false;
    const char* lt = "lt";
    const char* gt = "gt";
    const char* func = __func__;
    dlg_add_tag(lt, func);
    dlg_add_tag(gt, NULL);
    dlg_info(".");
    EXPECT(gdata.fired);
    dlg_assert(false);

    EXPECT(dlg_remove_tag(lt, func));
    EXPECT(dlg_remove_tag(gt, NULL));
    EXPECT(!dlg_remove_tag(lt, func));
    EXPECT(!dlg_remove_tag(gt, NULL));

    gdata.tags = t1;
    gdata.fired = false;
    dlg_warn(".");
    dlg_assert(false);
    EXPECT(gdata.fired);

    // correct scope of added tags
    const char* lt3 = "lt3";
    const char* gt3 = "gt3";
    const char* t7[] = {gt3, NULL};
    dlg_add_tag(lt3, func);
    dlg_add_tag(gt3, NULL);
    gdata.fired = false;
    gdata.tags = t7;
    foo_log();
    EXPECT(gdata.fired);
    foo_assert();
    EXPECT(dlg_remove_tag(lt3, func));
    EXPECT(dlg_remove_tag(gt3, NULL));
    EXPECT(!dlg_remove_tag(lt3, func));
    EXPECT(!dlg_remove_tag("non-existent", NULL));

    // reset handler
    fclose(check_file);
    fclose(check_formatted_file);

    dlg_set_handler(dlg_default_output, NULL);
    gdata.fired = false;
    dlg_info("Hai!");
    EXPECT(!gdata.fired);

    // fprintf
    if (!dlg_win_init_ansi())
        printf("$$$ dlg init ansi console, the following might get weird\n");

    printf(" - There should follow some utf-8 chars\n");

    struct dlg_style mstyle = { .style = dlg_text_style_bold, .fg = dlg_color_red, .bg = dlg_color_none };
    dlg_fprintf(stdout, u8"Ŝǿмẽ śạოрłё ẶŠČÌĬ-ŧē×ť (%s, אָǒť %s ãşçĩị...): %d\n", "ẃέłĺ", "all", 42);

    printf(" - The following line should be bold red, using utf-8 chars\n");

    dlg_styled_fprintf(stdout, mstyle, u8"ầŝƒđĵšҝďƒĵqשׂęрốґμĝĺ (<%s> in dingus-evlish)\n", "it's some kind of evlish");

    // return count of total errors
    return gerror;
}

void foo_log()
{
    dlg_info("log call from foo");
}

void foo_assert()
{
    dlg_assert(false);
}

void custom_handler(const struct dlg_origin* origin, const char* string, void* data)
{
    gdata.fired = true;
    if (data != &gdata) {
        printf("$$$ handler: invalid data %p [%d]\n", data, origin->line);
        ++gerror;
    }

    if (strcmp(origin->file, "docs/tests/core.c") != 0) {
        printf("$$$ handler: invalid file %s [%d]\n", origin->file, origin->line);
        ++gerror;
    }

    if (gdata.check & check_string) {
        if ((string == NULL) != (gdata.string == NULL)) {
            printf("$$$ handler: Invalid string (validness) %d, expected %d [%d]\n",
                   (string != NULL), (gdata.string != NULL), origin->line);
        } else if (string && strcmp(string, gdata.string)) {
            printf("$$$ handler: invalid string '%s' [%d]\n", string, origin->line);
        }
    }

    if (gdata.check & check_expr) {
        if ((origin->expr == NULL) != (gdata.expr == NULL)) {
            printf("$$$ handler: Invalid expr (ptr) %d, expected %d [%d]\n",
                   (origin->expr != NULL), (gdata.expr != NULL), origin->line);
        } else if (origin->expr && strcmp(origin->expr, gdata.expr)) {
            printf("$$$ handler: invalid expr '%s' [%d]\n", origin->expr, origin->line);
        }
    }

    if (gdata.check & check_line && origin->line != gdata.line) {
        printf("$$$ handler: invalid line: %d, expected %d\n", origin->line, gdata.line);
        ++gerror;
    }

    if (gdata.check & check_level && origin->level != gdata.level) {
        printf("$$$ handler: invalid level: %d, expected %d [%d]\n", origin->level, gdata.level, origin->line);
        ++gerror;
    }

    if (gdata.check & check_fire && !gdata.fire) {
        printf("$$$ handler: fired although it should not [%d]\n", origin->line);
        ++gerror;
    }

    if (gdata.check & check_tags) {
        const char** tags = origin->tags;
        const char** etags = gdata.tags;
        while (*tags && *etags) {
            if (strcmp(*tags, *etags) != 0) {
                printf("$$$ handler: invalid tag %s, expected %s [%d]\n", *tags, *etags, origin->line);
                ++gerror;
            }
            ++tags;
            ++etags;
        }

        if (*tags && !*etags) {
            printf("$$$ handler: more tags than expected [%d]\n", origin->line);
            ++gerror;
        }

        if (!*tags && *etags) {
            printf("$$$ handler: fewer tags than expected [%d]\n", origin->line);
            ++gerror;
        }
    }

    unsigned int features = dlg_output_tags | dlg_output_time | dlg_output_file_line | dlg_output_newline ;


    dlg_generic_output_stream(check_file, features, origin, string, dlg_default_output_styles);

    // unsigned int features_o = dlg_output_file_line | dlg_output_func | dlg_output_tags | dlg_output_newline | dlg_output_time | dlg_output_time_msecs | dlg_output_threadsafe;
    // dlg_set_layout( "[ $ tags: {%t} $ time: %s%ms file: %l func: %F]    " );
    // dlg_generic_output_stream(check_formatted_file, features_o, origin, string, dlg_default_output_styles);
    // dlg_set_default_layout();
}
