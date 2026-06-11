#include <sys/types.h>
#include "beacon.h"

// https://github.com/outflanknl/nix_bof_template
// ID BOF from Outflank

uid_t libc$getuid();
uid_t libc$geteuid();

void go(char* args, int alen) {
    uid_t uid = libc$getuid();
    uid_t euid = libc$geteuid();

    BeaconPrintf(CALLBACK_OUTPUT, "UID: %d EUID: %d\n", uid, euid);
}
