#include "system/includes.h"
#include "generic/log.h"
#include "app_config.h"

extern char __VERSION_BEGIN[];
extern char __VERSION_END[];

const char *sdk_version(void)
{
    return "AC79NN SDK on branch [release/AC79NN_SDK_V1.2.0] tag [AC79NN_SDK_V1.2.11_2025-12-24]";
}

static int app_version_check()
{
    char *version;

    printf("================= SDK Version    %s     ===============\n", sdk_version());
    for (version = __VERSION_BEGIN; version < __VERSION_END;) {
        printf("%s\n", version);
        version += strlen(version) + 1;
    }
    puts("=======================================\n");

    return 0;
}
early_initcall(app_version_check);

