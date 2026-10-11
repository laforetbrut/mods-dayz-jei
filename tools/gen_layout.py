# Generates JEI/gui/layouts/jei_menu.layout and jei_tile.layout.
# Author: vyrriox
#
# Usage: python tools/gen_layout.py JEI/gui/layouts

import sys
from pathlib import Path

OUT = Path(sys.argv[1])

BG = "0.055 0.06 0.07 0.96"
CARD = "0.1 0.105 0.12 1"
BTN = "0.17 0.18 0.2 1"
ACCENT = "0.86 0.55 0.18 1"
TEXT = "0.92 0.92 0.92 1"
MUTED = "0.6 0.62 0.66 1"


def rel(x, y, w, h):
    return [f"position {x} {y}", f"size {w} {h}", "hexactpos 0", "vexactpos 0", "hexactsize 0", "vexactsize 0"]


def block(cls, name, props, children=None, ind=0):
    pad = " " * ind
    lines = [f"{pad}{cls} {name} {{"]
    lines += [f"{pad} {p}" for p in props]
    if children:
        lines.append(f"{pad} {{")
        for c in children:
            lines.append(c(ind + 2))
        lines.append(f"{pad} }}")
    lines.append(f"{pad}}}")
    return "\n".join(lines)


def W(cls, name, props, children=None):
    return lambda ind: block(cls, name, props, children, ind)


def panel(name, x, y, w, h, color, children=None, extra=()):
    return W("PanelWidgetClass", name, [*rel(x, y, w, h), f"color {color}", "style rover_sim_colorable", *extra], children)


def frame(name, x, y, w, h, children=None, extra=()):
    return W("FrameWidgetClass", name, [*rel(x, y, w, h), *extra], children)


def text(name, x, y, w, h, value="", size=16, color=TEXT, halign="left", extra=()):
    props = [*rel(x, y, w, h), "ignorepointer 1", f"color {color}", f'text "{value}"',
             'font "gui/fonts/metron"', '"exact text" 1', f'"exact text size" {size}',
             '"text valign" center', f'"text halign" {halign}', *extra]
    return W("TextWidgetClass", name, props)


def button(name, x, y, w, h, label, size=15, color=BTN):
    return panel(name, x, y, w, h, color, [text(name + "Text", 0, 0, 1, 1, label, size, halign="center")])


def editbox(name, x, y, w, h):
    return W("EditBoxWidgetClass", name, [*rel(x, y, w, h), "style DefaultBorder", 'font "gui/fonts/metron"',
                                          '"exact text" 1', '"exact text size" 16'])


def slot(name, x, y, w, h):
    return frame(name, x, y, w, h)


main_children = [
    panel("HeaderBar", 0, 0, 1, 0.055, "0.09 0.095 0.11 1", extra=["ignorepointer 1"]),
    panel("HeaderLine", 0, 0.055, 1, 0.003, ACCENT, extra=["ignorepointer 1"]),
    text("Title", 0.012, 0.008, 0.3, 0.04, "JEI", 24, ACCENT),
    text("Subtitle", 0.045, 0.008, 0.3, 0.04, "Just Enough Items", 16, MUTED),
    button("BtnClose", 0.968, 0.01, 0.026, 0.037, "X", 16),

    # Left: item grid
    editbox("Search", 0.012, 0.068, 0.25, 0.045),
    text("SearchPlaceholder", 0.02, 0.068, 0.24, 0.045, "Search...", 15, MUTED),
    button("BtnSort", 0.268, 0.068, 0.105, 0.045, "Sort: name", 13),
    button("BtnModFilter", 0.378, 0.068, 0.104, 0.045, "All mods", 13),
    text("SearchHint", 0.012, 0.112, 0.47, 0.025, "Left click: how to craft    Right click: used in    Wheel: pages    @mod  =Class", 12, MUTED),
    panel("Grid", 0.012, 0.14, 0.47, 0.76, CARD),
    button("BtnPrev", 0.012, 0.91, 0.05, 0.045, "<", 18),
    text("PageText", 0.065, 0.91, 0.12, 0.045, "1 / 1", 15, TEXT, "center"),
    button("BtnNext", 0.188, 0.91, 0.05, 0.045, ">", 18),
    text("ItemCount", 0.25, 0.91, 0.232, 0.045, "", 13, MUTED, "right"),

    # Right: selected item
    panel("PreviewBg", 0.5, 0.068, 0.13, 0.2, CARD, [
        W("ItemPreviewWidgetClass", "Preview", [*rel(0.05, 0.05, 0.9, 0.9), "ignorepointer 1"]),
    ]),
    text("ItemName", 0.645, 0.068, 0.345, 0.05, "Select an item", 22),
    text("ItemInfo", 0.645, 0.115, 0.345, 0.035, "", 13, MUTED),
    text("ItemInfo2", 0.645, 0.15, 0.345, 0.035, "", 13, MUTED),
    button("BtnRecipes", 0.645, 0.21, 0.17, 0.05, "How to craft", 15),
    button("BtnUses", 0.82, 0.21, 0.17, 0.05, "Used in", 15),

    # Recipe card
    panel("RecipeCard", 0.5, 0.28, 0.49, 0.43, CARD, [
        panel("RecipeAccent", 0, 0, 0.006, 1, ACCENT, extra=["ignorepointer 1"]),
        panel("RecipeDivider", 0.03, 0.235, 0.94, 0.004, "0.2 0.21 0.24 1", extra=["ignorepointer 1"]),
        text("RecipeTitle", 0.03, 0.03, 0.62, 0.12, "", 19, ACCENT),
        button("BtnRecPrev", 0.68, 0.035, 0.08, 0.11, "<", 18),
        text("RecPageText", 0.765, 0.035, 0.13, 0.11, "", 15, TEXT, "center"),
        button("BtnRecNext", 0.9, 0.035, 0.08, 0.11, ">", 18),
        text("RecipeTag", 0.03, 0.15, 0.94, 0.08, "", 13, MUTED),
        slot("SlotA", 0.04, 0.3, 0.17, 0.4),
        text("PlusText", 0.21, 0.3, 0.06, 0.4, "+", 30, MUTED, "center"),
        slot("SlotB", 0.27, 0.3, 0.17, 0.4),
        text("ArrowText", 0.45, 0.3, 0.08, 0.4, ">", 32, ACCENT, "center"),
        slot("SlotR0", 0.54, 0.3, 0.14, 0.33),
        slot("SlotR1", 0.69, 0.3, 0.14, 0.33),
        slot("SlotR2", 0.84, 0.3, 0.14, 0.33),
        text("SlotAName", 0.0, 0.72, 0.25, 0.1, "", 13, TEXT, "center"),
        text("SlotBName", 0.23, 0.72, 0.25, 0.1, "", 13, TEXT, "center"),
        text("ResultName", 0.53, 0.66, 0.46, 0.1, "", 13, TEXT, "center"),
        text("SlotAState", 0.0, 0.82, 0.25, 0.08, "", 12, MUTED, "center"),
        text("SlotBState", 0.23, 0.82, 0.25, 0.08, "", 12, MUTED, "center"),
        text("NoRecipe", 0.03, 0.3, 0.94, 0.4, "", 16, MUTED, "center"),
    ]),

    button("BtnCraft1", 0.5, 0.725, 0.15, 0.055, "Craft x1", 16, "0.2 0.42 0.24 1"),
    button("BtnCraftMax", 0.655, 0.725, 0.15, 0.055, "Craft x10", 16, "0.2 0.42 0.24 1"),
    text("Status", 0.5, 0.79, 0.49, 0.04, "", 14, MUTED),

    frame("AdminPanel", 0.5, 0.85, 0.49, 0.105, [
        text("AdminLabel", 0, 0, 1, 0.35, "ADMIN", 12, "0.9 0.4 0.4 1"),
        button("BtnGive1", 0.0, 0.42, 0.15, 0.55, "Give x1", 14),
        button("BtnGive10", 0.16, 0.42, 0.15, 0.55, "Give x10", 14),
        button("BtnNewRecipe", 0.32, 0.42, 0.22, 0.55, "New recipe", 14),
        button("BtnEditRecipe", 0.55, 0.42, 0.22, 0.55, "Edit recipe", 14),
        button("BtnToggleRecipe", 0.78, 0.42, 0.22, 0.55, "Disable", 14),
    ], extra=["visible 0"]),

    panel("Editor", 0.5, 0.065, 0.49, 0.89, "0.08 0.085 0.1 1", [
        text("EditorTitle", 0.03, 0.02, 0.94, 0.06, "Recipe editor", 20, ACCENT),
        text("EditorHint", 0.03, 0.08, 0.94, 0.05, "Class names separated by commas. Pick an item in the grid, then press + to add it.", 13, MUTED),
        text("LblName", 0.03, 0.15, 0.25, 0.05, "Name", 15),
        editbox("EdName", 0.28, 0.15, 0.69, 0.05),
        text("LblIngA", 0.03, 0.25, 0.25, 0.05, "Ingredient A", 15),
        editbox("EdIngA", 0.28, 0.25, 0.6, 0.05),
        button("BtnAddA", 0.9, 0.25, 0.07, 0.05, "+", 18),
        button("BtnConsumeA", 0.28, 0.31, 0.35, 0.045, "Consume A: yes", 13),
        text("LblIngB", 0.03, 0.4, 0.25, 0.05, "Ingredient B (optional)", 15),
        editbox("EdIngB", 0.28, 0.4, 0.6, 0.05),
        button("BtnAddB", 0.9, 0.4, 0.07, 0.05, "+", 18),
        button("BtnConsumeB", 0.28, 0.46, 0.35, 0.045, "Consume B: yes", 13),
        text("LblResults", 0.03, 0.55, 0.25, 0.05, "Results", 15),
        editbox("EdResults", 0.28, 0.55, 0.6, 0.05),
        button("BtnAddResult", 0.9, 0.55, 0.07, 0.05, "+", 18),
        button("BtnSave", 0.28, 0.88, 0.21, 0.06, "Save", 16, "0.2 0.42 0.24 1"),
        button("BtnDelete", 0.52, 0.88, 0.21, 0.06, "Delete", 16, "0.5 0.18 0.18 1"),
        button("BtnCancel", 0.76, 0.88, 0.21, 0.06, "Cancel", 16),
    ], extra=["visible 0"]),
]

root = W("FrameWidgetClass", "JEIRoot", rel(0, 0, 1, 1), [
    panel("Dim", 0, 0, 1, 1, "0 0 0 0.6", extra=["ignorepointer 1"]),
    panel("Main", 0.06, 0.06, 0.88, 0.88, BG, main_children),
    W("PanelWidgetClass", "Tooltip", ["position 0 0", "size 460 68", "hexactpos 1", "vexactpos 1", "hexactsize 1",
                                      "vexactsize 1", "ignorepointer 1", "visible 0", "color 0.03 0.03 0.04 0.97",
                                      "style rover_sim_colorable"], [
        text("TooltipName", 0.03, 0.05, 0.94, 0.45, "", 16),
        text("TooltipInfo", 0.03, 0.5, 0.94, 0.45, "", 12, MUTED),
    ]),
])

(OUT / "jei_menu.layout").write_text(root(0) + "\n", encoding="utf-8")

tile = W("PanelWidgetClass", "Tile", [*rel(0, 0, 1, 1), f"color {BTN}", "style rover_sim_colorable"], [
    W("ItemPreviewWidgetClass", "TilePreview", [*rel(0.08, 0.08, 0.84, 0.84), "ignorepointer 1"]),
    text("TileLabel", 0.04, 0.04, 0.92, 0.92, "", 11, TEXT, "center", extra=["visible 0"]),
])
(OUT / "jei_tile.layout").write_text(tile(0) + "\n", encoding="utf-8")
print("ok")
