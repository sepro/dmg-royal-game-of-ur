# FIXME - Link Cable Issues

## 1. Bump sync timeouts (trivial, do first)

**Files:** `include/link/link_connect.h:20`, `include/link/link_profile.h:71`

Change `CONNECT_SYNC_TIMEOUT` and `LPROFILE_SYNC_TIMEOUT` from 180 to 360 frames (3s → 6s).

The side-reveal screen in link_connect.c allows A-skip, meaning one player can enter SYNCING up to 120 frames before the other. With a 180-frame timeout, the effective sync window shrinks to just 60 frames (1 second) in the worst case. Doubling the timeout gives plenty of margin.

**Risk:** Near zero. Only effect is longer wait on genuine disconnections.

## 2. Investigate post-profile-select link failure (separate root cause)

**Symptom:** Sometimes initiating a link game fails after both players have selected a profile.

**Why it's NOT the timeout issue above:** The profile exchange in `link_profile.c` naturally synchronizes both sides — `try_exchange_profiles()` requires both to send AND receive before either enters SYNCING, so both enter within a few frames of each other. The 180-frame timeout should be adequate there.

**Where to investigate:**
- `link_profile.c` `try_exchange_profiles()` — is the exchange itself dropping bytes or getting stale data?
- `link.c` `link_ready_sync()` — is it reliably completing on hardware?
- `link.c` `link_exchange()` / `link_exchange_slave()` — race conditions in the transfer?
- Could the link state be dirty entering link_profile if the earlier link_connect sync barely succeeded?

**Testing approach:** Add visual debug indicators (e.g. briefly flash a tile or increment a counter on screen) to show which phase is timing out.
