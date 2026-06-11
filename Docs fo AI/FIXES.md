# Cobalt Strike Linux Beacon - Fixes Applied

## Issues Fixed

### 1. `sonnt.profile` metadata decryption failure (`transform.c`)

**Problem**: The beacon worked with `quan.profile` but not `sonnt.profile`. CS teamserver logged:
```
[-] decrypt of metadata failed
[-] A Malleable C2 attempt to recover data from a '.http-get.client.metadata' transaction failed
```

**Root Cause**: In `implant/src/transform.c`, both `TRANSFORM_NETBIOS` and `TRANSFORM_NETBIOSU` were encoding identically using uppercase `'A' + nibble`. In real Cobalt Strike Malleable C2:
- `netbios` encodes using **lowercase** `a-p` (`'a' + nibble`)
- `netbiosu` encodes using **uppercase** `A-P` (`'A' + nibble`)

`sonnt.profile` uses `netbios` (lowercase), so the teamserver expected lowercase-encoded metadata but the beacon sent uppercase, causing RSA decryption to fail. `quan.profile` uses `netbiosu` (uppercase), which matched the buggy implementation.

**Fix**:
- Changed `TRANSFORM_NETBIOS` encode to use `'a' + nibble` (lowercase)
- Split the decode paths so `netbios` decodes from lowercase and `netbiosu` from uppercase

**Files Modified**:
- `implant/src/transform.c`

---

### 2. "Dropped responses from session. Didn't expect N prior to first task" error

**Problem**: On first checkin, the teamserver logged many warnings:
```
[-] Dropped responses from session. Didn't expect 6 prior to first task
```

**Root Cause**: The beacon's first GET response from CS contains initial tasks (PWD, SLEEP, etc.). When the beacon processes them, it calls `send_output_to_server` immediately for each task result, triggering multiple POST responses in rapid succession. The teamserver hasn't finished registering the session when it receives these POSTs, so it drops them and logs warnings.

**Fix**: Deferred task processing on the first checkin. The beacon now receives the initial tasks but doesn't process them until the second checkin, giving the teamserver time to register the session.

Also enforced a 200ms minimum sleep (even when profile says sleep 0) to prevent flooding.

**Files Modified**:
- `implant/src/beacon.c` — added first-checkin detection to skip `commands_parse_tasks`
- `implant/src/main.c` — enforced 200ms minimum sleep before entering the main loop
- `implant/src/commands.c` — added 100ms delay between consecutive output POSTs (as a secondary measure)

---

## Files Changed Summary

| File | Change |
|------|--------|
| `implant/src/transform.c` | Fixed `TRANSFORM_NETBIOS` to use lowercase encoding; split decode paths |
| `implant/src/beacon.c` | Skip task processing on first checkin to avoid "Dropped responses" error |
| `implant/src/main.c` | Enforce 200ms minimum sleep before first checkin |
| `implant/src/commands.c` | Added 100ms delay between consecutive output POSTs |

---

## Build Instructions

```bash
cd implant
make clean && make
```

Output: `implant/bin/beacon`

To generate a new profile header from a `.profile` file:
```bash
python3 generate-payload/InsertProfile.py path/to/profile.profile implant/headers/profile_generated.h
```

---

## Known Limitations

- The initial PWD and SLEEP task results are dropped (not delivered to operator) due to the deferred processing. The operator will still see the session as active and can issue new commands immediately.
- The `commands.c` 100ms delay between POSTs is a band-aid; the root fix is the deferred processing in `beacon.c`.
