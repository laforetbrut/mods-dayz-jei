# Generates workshop/preview.jpg, the Steam Workshop preview image.
# Author: vyrriox
#
# Usage: python tools/gen_preview.py

from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "workshop" / "preview.jpg"
SIZE = 512
BG = (14, 15, 18)
CARD = (26, 27, 31)
TILE = (43, 46, 51)
TILE_OK = (35, 65, 42)
ACCENT = (219, 140, 46)
TEXT = (235, 235, 235)
MUTED = (158, 160, 168)


def font(size, bold=False):
    names = ["segoeuib.ttf" if bold else "segoeui.ttf", "arialbd.ttf" if bold else "arial.ttf"]
    for name in names:
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def main():
    img = Image.new("RGB", (SIZE, SIZE), BG)
    d = ImageDraw.Draw(img)

    # Item grid in the background, a few tiles lit like craftable items.
    cell = 52
    for row in range(4):
        for col in range(9):
            x = 14 + col * cell
            y = 250 + row * cell
            color = TILE_OK if (row * 9 + col) % 7 == 3 else TILE
            d.rounded_rectangle([x, y, x + cell - 8, y + cell - 8], radius=6, fill=color)

    d.rectangle([0, 0, SIZE, 8], fill=ACCENT)
    d.text((SIZE // 2, 95), "JEI", font=font(150, True), fill=ACCENT, anchor="mm")
    d.text((SIZE // 2, 185), "Just Enough Items", font=font(38, True), fill=TEXT, anchor="mm")
    d.text((SIZE // 2, 225), "Recipe browser and quick craft for DayZ", font=font(20), fill=MUTED, anchor="mm")

    # Recipe strip: A + B > result
    y = 470
    d.rounded_rectangle([60, y - 26, SIZE - 60, y + 26], radius=10, fill=CARD)
    for x, label in ((120, "A"), (220, "B"), (380, "")):
        d.rounded_rectangle([x - 20, y - 18, x + 20, y + 18], radius=5, fill=TILE_OK if label else TILE)
        if label:
            d.text((x, y), label, font=font(20, True), fill=TEXT, anchor="mm")
    d.text((170, y), "+", font=font(28, True), fill=MUTED, anchor="mm")
    d.text((300, y), ">", font=font(30, True), fill=ACCENT, anchor="mm")
    d.text((380, y), "OK", font=font(16, True), fill=ACCENT, anchor="mm")

    OUT.parent.mkdir(parents=True, exist_ok=True)
    img.save(OUT, "JPEG", quality=92)
    print(OUT)


if __name__ == "__main__":
    main()
