# CobaltStrike-Linux-Beacon — Known Issues & Workarounds

This document tracks the current state of bugs, errors, and quirks discovered in this Linux beacon port, along with workarounds. It is not a fix list — it is a snapshot of what is broken and how to work around it.

---

## 1. CustomBeacon.cna — Generation Script

### 1.1 Hardcoded project folder path

**File:** `CustomBeacon.cna:1`

```sleep
$CustomBeaconProjectFolder = "/home/kali/CS/CobaltStrike-Linux-Beacon/";
```

**Status:** Hardcoded absolute path.

**Issue:** The script uses an absolute path to the project root. Moving the project to another machine requires editing line 1.

**Auto-detect attempt:** Tried `getFileDirectory(script_resource("CustomBeacon.cna"))`, but `script_resource()` returns inconsistent values when the script is loaded via the Cobalt Strike Script Manager (often just the filename with no directory). The auto-detect approach was reverted because it produced empty paths and broke the script.

**Workaround:** Manually edit line 1 to the new absolute path on every new machine.

---

### 1.2 `closef($null)` errors on `exec()` failure

**File:** `CustomBeacon.cna` (multiple locations)

**Status:** Fixed.

**Original error:**
```
[!] null value error at CustomBeacon.cna:75
[!] null value error at CustomBeacon.cna:102
```

**Cause:** `exec()` returns `$null` when the command cannot be launched (wrong working directory, missing script, etc). Passing `$null` to `closef()` throws a "null value error" in Sleep/Aggressor Script.

**Fix applied:** Added `if ($process ne $null)` guards before every `closef($process)` call. If `exec()` fails, the script logs an error and continues.

---

### 1.3 Ternary operator not supported

**File:** `CustomBeacon.cna:170` (original)

**Status:** Fixed.

**Original error:**
```
[!] Attempting to use non-existent operator: ':' at CustomBeacon.cna:170
[!] Attempting to use non-existent operator: '?' at CustomBeacon.cna:170
[!] attempted an invalid cast: class sleep.bridges.BasicStrings cannot be cast to class sleep.interfaces.Operator
```

**Cause:** Sleep/Aggressor Script does not support the ternary `condition ? a : b` operator.

**Fix applied:** Replaced the ternary with an explicit `if/else` block setting a `$profileDisplay` variable.

---

### 1.4 Profile path quoting bug

**File:** `CustomBeacon.cna:71`

**Status:** Fixed.

**Original error:**
```
[0/6] Inserting malleable profile: /home/kali/CS/CobaltStrike-Linux-Beacon/quan.profile
Error: Profile file not found: "/home/kali/CS/CobaltStrike-Linux-Beacon/quan.profile"
```

**Cause:** The original command wrapped the profile path in escaped quotes:
```sleep
$command = "bash gen_profile.sh \"" . $profilePath . "\" \"" . $profileHeader . "\"";
```

Aggressor Script's `exec()` does not run through a shell — it splits arguments directly. The literal double-quote characters were passed as part of the path, so `os.path.exists()` in `InsertProfile.py` received `"/home/kali/.../quan.profile"` (with embedded quotes) and failed to find the file.

**Fix applied:** Removed the quoting. The path is now passed bare:
```sleep
$command = "bash gen_profile.sh " . $profilePath . " " . $profileHeader;
```

Since project paths here do not contain spaces, this is safe. If a path contains spaces, manual quoting will need to be re-added with a shell wrapper.

---

### 1.5 Missing `libcurl4-openssl-dev` package

**Status:** Fixed (system package install).

**Original symptom:** Build failed with:
```
./headers/http.h:6:10: fatal error: curl/curl.h: No such file or directory
```

**Cause:** The C build requires `libcurl4-openssl-dev` (development headers for libcurl), which is not installed by default on Kali Linux.

**Fix:** `apt-get install -y libcurl4-openssl-dev`

---

### 1.6 Copy to output directory silently fails

**File:** `CustomBeacon.cna:159`

**Status:** Fixed.

**Original symptom:** Dialog reported "Path: /home/kali/Beacon_x64" but the file was not present at that path. Binary was only at `implant/bin/Beacon_x64`.

**Cause:** The original `exec()` call for the `cp` command used quoted paths but no working directory:
```sleep
$process = exec($command);
```

Aggressor's `exec()` does not always resolve relative paths from the script's directory.

**Fix applied:** Use the absolute path for `cp` and pass the implant directory as the working directory:
```sleep
$command = "/bin/cp " . $final . " " . $dest;
$process = exec($command, $null, $ImplantSrcDirectory);
```

Also added a post-copy existence check so the dialog reports the actual file location if the copy fails.

---

## 2. Implant — Unknown Command Handling

### 2.1 "Unknown error: 1131375981" flood after `screenshot`

**Status:** Partially fixed, residual cosmetic issue.

**Symptom:** After running `screenshot` (or any other Windows-only command like `keylogger`, `hashdump`, `screenshot`, etc), the teamserver floods the beacon console with:
```
[-] Unknown error: 1131375981
```
once per check-in, indefinitely.

**Decoded error code:** `1131375981 = 0x436F6E6E` (ASCII "Conn"). This is the Cobalt Strike teamserver's generic "no callback registered for this response" error.

**Cause:** Cobalt Strike's teamserver keeps a pending task queue per beacon. When `screenshot` is issued, the teamserver queues a `CALLBACK_SCREENSHOT` (0x33) task. The Linux implant falls into the `default` case of its command switch and discards it. Because the implant never returns a response with the expected callback type, the teamserver re-queues the task on every check-in. The result is an error flood.

**Fix attempt 1 (rejected):** Return `-1` with no output in the `default` case.
- Result: the teamserver keeps re-sending the task forever (infinite flood).

**Fix attempt 2 (rejected):** Return `0` with `CALLBACK_ERROR` (0x1f) and an error message.
- Result: the teamserver does not recognize this as a terminal failure for the screenshot task; it keeps re-queuing.

**Fix attempt 3 (rejected):** Return `0` with `CALLBACK_OUTPUT` and a "Unknown command!" string.
- Result: the teamserver routes the response to the beacon console ("received output: Unknown command!") but the task is still considered pending and re-queued. Worse than fix 1 because the flood is now visible in the console as text.

**Current state:** The `default` case returns `-1` with no output. The "Unknown error: 1131375981" flood occurs, but the teamserver's long-poll timeout (60-90s) eventually clears the stuck task. Subsequent commands (`ls`, `pwd`, `shell`, etc) work normally throughout the flood.

**Workaround for operator:**
1. Do not issue Windows-only commands (`screenshot`, `keylogger`, `hashdump`, `mimikatz`, `psinject`, `spawn`, etc) on Linux beacons.
2. If a Windows-only command is issued accidentally, run `sleep 60` to force the teamserver to drop the stuck task and clear the error flood.
3. After the flood clears, run `sleep 0` to return to 200ms check-ins.

**Real fix (not implemented):** Add a `CALLBACK_SCREENSHOT` handler to the implant that captures the Linux desktop and returns PNG bytes. Out of scope per project requirements.

---

## 3. Implant — Sleep Behavior

### 3.1 Sleep 2 takes 58 seconds to take effect

**Status:** Not a bug — protocol behavior.

**Symptom:**
```
[11:34:33] beacon> sleep 2
[11:34:33] [*] Tasked beacon to sleep for 2s
[11:35:31] [+] host called home, sent: 16 bytes
```

A `sleep 2` command took 58 seconds to get a response, suggesting the implant did not honor the new sleep value.

**Cause:** Cobalt Strike uses **long polling** on the implant's HTTP GET check-ins. The teamserver holds the GET connection open for up to 60-90 seconds, waiting for tasks to queue. If no task is queued, the teamserver's long-poll timeout fires and returns an empty response. The implant then enters its `beacon_sleep()` cycle.

**Timeline reconstruction:**
- `11:34:15` — User issues `sleep 2`. Implant sets `g_sleep_time_ms = 2000`.
- `11:34:16` — Implant checks in via GET. Teamserver holds the connection.
- `11:34:27` — User issues `sleep 60`. Teamserver queues the task, returns it on the held-open GET.
- `11:34:27` — Implant processes `sleep 60`, sets `g_sleep_time_ms = 60000`, responds.
- `11:34:33` — User issues `sleep 2`. Task is queued on teamserver.
- `11:35:27` — Implant's 60-second sleep ends, checks in, receives `sleep 2` task, responds.
- `11:35:31` — Response appears in the console.

The 58-second delay is the implant waiting for its 60-second sleep to expire before the next check-in. The `sleep` value is a *target interval* between check-in **attempts**, not a hard deadline. The teamserver's long-poll behavior overrides it.

**Workaround:** None on the implant side. This is a fundamental CS protocol design. Use `sleep 0` (200ms minimum) for fast interactive work and accept the long-poll overhead.

---

## 4. Implant — Command Output Display

### 4.1 Output sometimes not shown in GUI

**Status:** Cosmetic — data is received, display is the issue.

**Symptom:** Commands like `shell ls` show `host called home, sent: N bytes` but no `received output:` line. The data is in the beacon console log but not auto-rendered.

**Cause:** Cobalt Strike's beacon console only auto-renders output for the **focused beacon tab** in the graph. If the bottom pane is showing a different beacon's log, the Linux beacon's responses are queued and not displayed.

**Additionally:** Cobalt Strike's display layer collapses rapid duplicate-looking responses. Spamming `shell ls` 6 times in 2 seconds causes only the first response to be shown; the rest are scrolled past or deduplicated.

**Workarounds:**
1. Click the beacon in the graph before issuing commands to ensure the bottom pane shows that beacon's log.
2. Use `View → Beacons` for a unified log of all responses.
3. Wait 1-2 seconds between rapid commands.
4. Use `shell ls; echo DONE` to confirm the response arrived even if the `ls` content was missed.

---

## 5. Implant — Supported Commands

The following Cobalt Strike client commands are implemented in `implant/src/commands.c`:

| CS Command | Implant Command ID | Notes |
|------------|-------------------|-------|
| `sleep <ms> [jitter%]` | 4 | Clamps to 200ms minimum |
| `cd <path>` | 5 | |
| `pwd` | 39 | Uses `CALLBACK_PWD` (0x13) |
| `shell <cmd>` | 78 | Runs via `popen` with `2>&1` |
| `exit` / `die` | 3 | Sends `CALLBACK_DEAD` then exits |
| `ls <path>` | 53 | Plain text output, not file browser |
| `upload <src> <dst>` | 10 / 67 | |
| `download <path>` | 11 | Triggers file download in CS |
| `cancel` | 19 | |
| `socks <port>` | 17 | Start SOCKS proxy |
| `rportfwd` / SOCKS ops | 14 / 15 / 16 | Connect / send / close |
| `inline-execute <elf>` | 200 | Custom BOF/ELF runner |

**Not implemented (Windows-only):** `screenshot`, `keylogger`, `hashdump`, `mimikatz`, `psinject`, `spawn`, `execute-assembly`, `runas`, `getsystem`, `uac-*`, `powerpick`, `shspawn`, `shinject`, `dllinject`, `pth`.

These commands will fall into the `default` case and produce the "Unknown error: 1131375981" flood described in section 2.1.

---

## 6. Build Environment

**Required packages on Kali Linux:**
- `gcc`
- `make`
- `libssl-dev`
- `libcurl4-openssl-dev`

**Build command:**
```bash
cd /home/kali/CS/CobaltStrike-Linux-Beacon/implant
make clean
make OUTPUT=Beacon_x64
```

**Output binary:** `implant/bin/Beacon_x64`

---

## 7. Summary of Fixes Applied

| # | File | Issue | Fix |
|---|------|-------|-----|
| 1 | `CustomBeacon.cna:1` | Hardcoded path `/home/s3th/` | Changed to `/home/kali/CS/CobaltStrike-Linux-Beacon/` |
| 2 | `CustomBeacon.cna` multiple | `closef($null)` errors | Added `if ($process ne $null)` guards |
| 3 | `CustomBeacon.cna:170` | Ternary `? :` not supported | Replaced with `if/else` block |
| 4 | `CustomBeacon.cna:71` | Profile path embedded quotes | Removed quotes, use bare path |
| 5 | `CustomBeacon.cna:159` | `cp` silently failed | Use `/bin/cp` with absolute working dir |
| 6 | System | Missing `libcurl4-openssl-dev` | `apt-get install -y libcurl4-openssl-dev` |

## 8. Known Unfixed Issues

| # | Issue | Severity | Workaround |
|---|-------|----------|------------|
| 1 | "Unknown error" flood after Windows-only commands | Low (cosmetic) | Run `sleep 60` to clear, or avoid those commands |
| 2 | `sleep 2` appears to take 58s after a long sleep | None (protocol) | Accept long-poll behavior |
| 3 | Output sometimes not shown in GUI | Low (display) | Click beacon tab, use `View → Beacons` |
| 4 | Auto-detect of project folder not working in Aggressor | Low | Manually edit line 1 on each machine |
