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

## [2026-10-11 17:32] — Mission module fails to compile on shift operator
**Context:** New JEI interface, hover color helper.
**Error:** `JEI/scripts/5_Mission/jei\jeimenu.c(478): Unknown operator '>>'`
**Root cause:** Enforce Script has no bit shift operators.
**Fix:** Replaced the computed color with a lookup of fixed hover colors.
**Prevention:** No `<<` / `>>` and no ternary operator in Enforce Script; use arithmetic or lookups.

## [2026-10-11 17:55] — Server crashes sending settings, client thinks crafting is disabled
**Context:** Opening JEI after a server restart (settings and custom_recipes.json already on disk).
**Error:** `NULL pointer to instance` in `JEI_SendSettings` on the server; client status "crafting disabled on this server".
**Root cause:** `JsonFileLoader.JsonLoadFile(path, this)` takes an `out` parameter and replaces its target; loading into `this` can leave the singleton null. The client never received settings and fell back to a wrong status.
**Fix:** Load into a temporary object and copy the fields; guard the send; show "loading" until settings are synced.
**Prevention:** Never pass `this` to `JsonLoadFile`; always load into a fresh local instance.
