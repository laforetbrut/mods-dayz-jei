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

## [2026-10-11 18:00] — Server cannot find recipe, NULL pointer in FindRecipe
**Context:** Craft request from the JEI window.
**Error:** `NULL pointer to instance` in `JEIRegistry.FindRecipe`, then "unknown or disabled recipe".
**Root cause:** Recipes were created by static factories (`JEIRecipe.FromVanilla`) and passed straight to `array.Insert`; the returned temporary was released, leaving null entries.
**Fix:** Build objects with a constructor (`new JEIRecipe(...)`), store function results in locals before using them, guard loops against null.
**Prevention:** In Enforce Script never pass a freshly returned object straight into a call; assign it to a local first, or construct it with `new` at the call site.

## [2026-10-11 18:08] — Craft accepted by JEI but "PerformRecipeServer - recipe not valid"
**Context:** Crafting a splint from the JEI window with ingredients in the inventory.
**Error:** `PerformRecipeServer - recipe not valid !!` from the vanilla recipe manager.
**Root cause:** Read in the vanilla scripts (dta/scripts.pbo): `RecipeBase.CheckRecipe` rejects any recipe not flagged "anywhere" when neither ingredient is in the player's hands. `PerformRecipeServer` also re-sorts ingredients through its cache and runs a sanity check.
**Fix:** `RecipeBase.JEI_Check` / `JEI_Perform` sort the items with `CheckIngredientMatch`, lift only the hands rule through `IsRecipeAnywhere` while JEI runs, then call `CheckRecipe` / `PerformRecipe` so every other vanilla condition still applies.
**Prevention:** Check the vanilla source (extract dta/scripts.pbo) before calling engine helpers; do not guess their preconditions.
