#include "volc_errno.h"
#include <errno.h>

int volc_errno(int fd)
{
    return errno;
}
