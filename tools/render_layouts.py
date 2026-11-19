#!/usr/bin/env python3
"""Render map layouts to a labelled contact-sheet PNG (needs Pillow).

Usage: python tools/render_layouts.py OUT.png LAYOUT_ID [LAYOUT_ID ...]
       python tools/render_layouts.py OUT.png 'LAYOUT_HOUSE*'     (fnmatch patterns work)

Draws each layout with its own primary and secondary tileset: bottom layer, then top layer with
palette index 0 transparent. Approximate (ignores layer-type rules and animated tiles), but good
enough to see where furniture and doors are. Metatile ids are printed in a grid with --ids.
"""
import fnmatch, json, os, struct, sys
from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NUM_TILES_PRIMARY, NUM_PALS_PRIMARY, NUM_METATILES_PRIMARY = 512, 6, 512
SCALE = 2


def tileset_dir(sym, layouts_primary):
    # gTileset_GenericBuilding -> data/tilesets/{primary,secondary}/generic_building
    name = sym[len("gTileset_"):]
    snake = "".join("_" + c.lower() if c.isupper() and i else c.lower() for i, c in enumerate(name))
    kind = "primary" if layouts_primary else "secondary"
    for cand in (snake, snake.replace("_", "")):
        p = os.path.join(ROOT, "data/tilesets", kind, cand)
        if os.path.isdir(p):
            return p
    raise SystemExit(f"no tileset dir for {sym} (tried {snake})")


class Tileset:
    def __init__(self, path):
        self.img = Image.open(os.path.join(path, "tiles.png")).convert("P")
        self.w = self.img.width // 8
        self.metatiles = open(os.path.join(path, "metatiles.bin"), "rb").read()
        self.pals = {}
        pdir = os.path.join(path, "palettes")
        for f in sorted(os.listdir(pdir)):
            if f.endswith(".pal"):
                lines = open(os.path.join(pdir, f)).read().split()[3:]
                nums = list(map(int, lines))
                self.pals[int(f[:2])] = [tuple(nums[i:i + 3]) for i in range(0, 48, 3)]

    def tile_pixels(self, idx):
        tx, ty = idx % self.w, idx // self.w
        return self.img.crop((tx * 8, ty * 8, tx * 8 + 8, ty * 8 + 8))


def render_metatile(prim, sec, mid):
    ts, local = (prim, mid) if mid < NUM_METATILES_PRIMARY else (sec, mid - NUM_METATILES_PRIMARY)
    out = Image.new("RGB", (16, 16), (255, 0, 255))
    if (local + 1) * 16 > len(ts.metatiles):
        return out
    entries = struct.unpack_from("<8H", ts.metatiles, local * 16)
    for layer in (0, 1):
        for q in range(4):
            e = entries[layer * 4 + q]
            tile, hf, vf, pal = e & 0x3FF, e >> 10 & 1, e >> 11 & 1, e >> 12
            tset = prim if tile < NUM_TILES_PRIMARY else sec
            t = tset.tile_pixels(tile if tile < NUM_TILES_PRIMARY else tile - NUM_TILES_PRIMARY)
            palset = prim if pal < NUM_PALS_PRIMARY else sec
            colors = palset.pals.get(pal, [(255, 0, 255)] * 16)
            if hf:
                t = t.transpose(Image.FLIP_LEFT_RIGHT)
            if vf:
                t = t.transpose(Image.FLIP_TOP_BOTTOM)
            px = list(t.tobytes())
            for i, v in enumerate(px):
                if layer == 1 and v == 0:
                    continue
                out.putpixel(((q % 2) * 8 + i % 8, (q // 2) * 8 + i // 8), colors[v])
    return out


def render_layout(lay):
    prim = Tileset(tileset_dir(lay["primary_tileset"], True))
    sec = Tileset(tileset_dir(lay["secondary_tileset"], False))
    w, h = lay["width"], lay["height"]
    data = open(os.path.join(ROOT, lay["blockdata_filepath"]), "rb").read()
    cells = struct.unpack(f"<{w * h}H", data[: w * h * 2])
    img = Image.new("RGB", (w * 16, h * 16))
    cache = {}
    for i, c in enumerate(cells):
        mid = c & 0x3FF
        if mid not in cache:
            cache[mid] = render_metatile(prim, sec, mid)
        img.paste(cache[mid], ((i % w) * 16, (i // w) * 16))
    return img.resize((w * 16 * SCALE, h * 16 * SCALE), Image.NEAREST)


def main():
    out, pats = sys.argv[1], sys.argv[2:]
    layouts = json.load(open(os.path.join(ROOT, "data/layouts/layouts.json"), encoding="utf-8"))["layouts"]
    chosen = [l for l in layouts if any(fnmatch.fnmatch(l["id"], p) for p in pats)]
    if not chosen:
        raise SystemExit("no layouts match " + " ".join(pats))
    tiles = [(l, render_layout(l)) for l in chosen]
    cols = 4
    cell_w = max(t.width for _, t in tiles) + 16
    cell_h = max(t.height for _, t in tiles) + 32
    rows = (len(tiles) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * cell_w, rows * cell_h), (40, 40, 40))
    d = ImageDraw.Draw(sheet)
    for i, (l, t) in enumerate(tiles):
        x, y = (i % cols) * cell_w + 8, (i // cols) * cell_h + 4
        d.text((x, y), f"{l['id'][7:]} {l['width']}x{l['height']}", fill=(255, 255, 255))
        sheet.paste(t, (x, y + 18))
    sheet.save(out)
    print(f"{len(tiles)} layouts -> {out}")
    for l in chosen:
        print(l["id"], l["width"], "x", l["height"])


if __name__ == "__main__":
    main()
