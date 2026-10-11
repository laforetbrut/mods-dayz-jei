# Error log

## [2026-10-11 17:15] — JEI menu does not open, NULL pointer in OnHide
**Context:** Pressing the JEI key in game (DayZ 1.29 Diag client).
**Error:** `SCRIPT (E): NULL pointer to instance, Class 'JEIMenu', Function 'OnHide'` from `JEI_HandleInput`.
**Root cause:** `UIManager.ShowScriptedMenu()` does not call `Init()` on a menu created with `new`, so the layout was never loaded and every widget member was null.
**Fix:** Call `menu.Init()` before `ShowScriptedMenu()`, make `Init()` idempotent and null-safe, guard `OnHide()`.
**Prevention:** Always call `Init()` yourself on a script-created `UIScriptedMenu`; check widget lookups before using them.

## [2026-10-11 17:16] — Repack fails with permission denied
**Context:** Rebuilding `@JEI/addons/JEI.pbo` while the test game was running.
**Error:** `PermissionError: [Errno 13] Permission denied: ...JEI.pbo`
**Root cause:** DayZ keeps loaded PBOs locked.
**Fix:** Close the Diag server and client before packing.
**Prevention:** `start-test.bat` packs before launching; close the game before relaunching it.
