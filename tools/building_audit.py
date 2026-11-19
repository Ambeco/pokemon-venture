#!/usr/bin/env python3
"""Inventory city buildings and the game state their scripts can change.

Usage: python3 tools/building_audit.py [--out documents/building_audit.md] [--csv buildings.csv]

The game has no building concept: a "building" is the set of indoor maps reachable from an outdoor
(city/town) map through warps without passing through another outdoor map. Scripts are scanned
per map (map header scripts, every label in the map's scripts.inc) and followed transitively through
every label they mention (goto/call/trainerbattle/etc.), including shared data/scripts/*.inc.
"""
import argparse, collections, glob, json, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUTDOOR = {"MAP_TYPE_CITY", "MAP_TYPE_TOWN", "MAP_TYPE_ROUTE", "MAP_TYPE_OCEAN_ROUTE", "MAP_TYPE_UNDERWATER"}
LABEL_RE = re.compile(r"^([A-Za-z_][A-Za-z0-9_]*)::?\s*(?:@.*)?$")
IDENT_RE = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
STATE_RE = re.compile(r"\b(FLAG|VAR)_[A-Z0-9_]+\b")
NOISE_VARS = re.compile(r"^VAR_(TEMP_|RESULT|0x|LAST_TALKED|FACING|PLAYER_FACING|OBJ_GFX_ID_|ITEM_ID|STARTER_MON|MAP_SCENE|SPECIAL)")
ARG_COMMANDS = {  # command -> (category, index of the state/item argument)
    "setflag": "flag", "clearflag": "flag", "setvar": "var", "addvar": "var", "subvar": "var",
    "giveitem": "item", "additem": "item", "givemon": "mon", "giveegg": "mon",
}


def key_items():
    """Items that can be story keys: the key-item pocket, every HM, and TM43 (Secret Power)."""
    keys, cur = {"ITEM_TM_SECRET_POWER"}, None
    for line in read(os.path.join(ROOT, "src/data/items.h")).splitlines():
        m = re.match(r"\s*\[(ITEM_[A-Z0-9_]+)\]\s*=", line)
        if m:
            cur = m.group(1)
            if cur.startswith("ITEM_HM_"):
                keys.add(cur)
        elif "POCKET_KEY_ITEMS" in line and cur:
            keys.add(cur)
    return keys


def read(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read()


def load_labels():
    """label -> body lines, and map folder -> its own labels."""
    bodies, by_map = {}, collections.defaultdict(list)
    files = glob.glob(os.path.join(ROOT, "data/scripts/*.inc")) + glob.glob(os.path.join(ROOT, "data/maps/*/scripts.inc"))
    for path in files:
        mapname = os.path.basename(os.path.dirname(path)) if "/maps/" in path.replace("\\", "/") else None
        cur = None
        for line in read(path).splitlines():
            m = LABEL_RE.match(line.strip())
            if m:
                cur = m.group(1)
                bodies[cur] = []
                if mapname:
                    by_map[mapname].append(cur)
            elif cur is not None:
                bodies[cur].append(line.split("@")[0].strip())
    return bodies, by_map


def effects_of(root_labels, bodies, cache):
    """Set of (category, name) effects reachable from the labels, plus the state names read."""
    effects, reads, seen, stack = set(), set(), set(), list(root_labels)
    while stack:
        lab = stack.pop()
        if lab in seen or lab not in bodies:
            continue
        seen.add(lab)
        for line in bodies[lab]:
            if not line or line.startswith("."):
                continue
            tokens = IDENT_RE.findall(line)
            cmd = tokens[0] if tokens else ""
            for t in tokens[1:]:
                if t in bodies:
                    stack.append(t)
            if cmd in ARG_COMMANDS and len(tokens) > 1:
                name = tokens[1]
                cat = ARG_COMMANDS[cmd]
                if name.startswith(("FLAG_TEMP_",)) or NOISE_VARS.match(name):
                    continue
                if cat == "flag" and name.startswith("FLAG_HIDE_"):
                    cat = "hide"
                effects.add((cat, name))
            elif cmd.startswith("trainerbattle"):
                trainer = next((t for t in tokens[1:] if t.startswith("TRAINER_")), "?")
                effects.add(("battle", trainer))
            else:
                for m in STATE_RE.finditer(line):
                    n = m.group(0)
                    if not (n.startswith("FLAG_TEMP_") or NOISE_VARS.match(n)):
                        reads.add(n)
    return effects, reads


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out")
    ap.add_argument("--csv")
    ap.add_argument("--all-items", action="store_true", help="count every giveitem and received-flag, not only key items, HMs and TM43")
    ap.add_argument("--frlg", action="store_true", help="include the Kanto (_Frlg) maps")
    ap.add_argument("--common", type=int, default=4, help="state set by this many buildings counts as common noise")
    args = ap.parse_args()

    maps = {}
    for path in glob.glob(os.path.join(ROOT, "data/maps/*/map.json")):
        d = json.loads(read(path))
        if "id" in d:
            d["_folder"] = os.path.basename(os.path.dirname(path))
            maps[d["id"]] = d
    layout_users = collections.defaultdict(list)
    for mid, d in maps.items():
        layout_users[d.get("layout")].append(d["_folder"])
    bodies, by_map = load_labels()

    def warps(mid):
        return [w["dest_map"] for w in maps[mid].get("warp_events", []) if w["dest_map"] in maps]

    cities = sorted(m for m, d in maps.items() if d.get("map_type") in ("MAP_TYPE_CITY", "MAP_TYPE_TOWN")
                    and (args.frlg or not d["_folder"].endswith("_Frlg")))
    buildings = []  # dicts
    for city in cities:
        first = collections.OrderedDict()
        for dest in warps(city):
            if maps[dest].get("map_type") in OUTDOOR:
                continue
            first.setdefault(dest, None)
        claimed = set()
        for entry in first:
            if entry in claimed:
                continue
            comp, exits, stack = [], set(), [entry]
            while stack:
                m = stack.pop()
                if m in comp:
                    continue
                comp.append(m)
                for w in warps(m):
                    if maps[w].get("map_type") in OUTDOOR:
                        if w != city:
                            exits.add(w)
                    else:
                        stack.append(w)
            claimed.update(comp)
            eff, rd = set(), set()
            for m in comp:
                e, r = effects_of(by_map.get(maps[m]["_folder"], []), bodies, None)
                eff |= e
                rd |= r
            entry_layout = maps[entry]["layout"]
            buildings.append(dict(city=city, entry=entry, maps=comp, exits=sorted(exits), effects=eff, reads=rd,
                                  layout=entry_layout, layout_uses=len(layout_users[entry_layout]),
                                  all_unique=all(len(layout_users[maps[m]["layout"]]) == 1 for m in comp)))

    def short(m):
        return maps[m]["_folder"]

    setter_count = collections.Counter(n for b in buildings for n in {e[1] for e in b["effects"] if e[0] in ("flag", "var", "item", "mon")})
    common = {n for n, c in setter_count.items() if c >= args.common}
    keys = key_items()

    def persistent(b):
        eff = {e for e in b["effects"] if e[0] in ("flag", "var", "item", "mon") and e[1] not in common}
        if args.all_items:
            return eff
        eff = {e for e in eff if e[0] != "item" or e[1] in keys}
        if not any(e[0] == "item" for e in eff):  # gift bookkeeping flags of ignored items
            eff = {e for e in eff if not e[1].startswith(("FLAG_RECEIVED_", "FLAG_GOT_"))}
        return eff
    out = []
    w = out.append
    w("# Building audit (generated by tools/building_audit.py)\n")
    w(f"{len(cities)} outdoor city/town maps, {len(buildings)} buildings (indoor map groups behind a city warp).\n")
    w("Columns: entry map; layout reuse (number of maps sharing the entry layout; 1 = unique); persistent state "
      "changed (flag/var/item/mon, temp and scratch state excluded); other effects (hide-flag toggles, trainer battles).\n")

    w("## Common state (set by %d+ buildings, excluded below)\n" % args.common)
    w(", ".join(sorted(common)) + "\n")
    cand = [b for b in buildings if b["layout_uses"] > 1 and persistent(b)]
    w("## Candidates: reused layout AND changes persistent state\n")
    w("These are where a `BuildingRole` is not obvious from the layout alone.\n")
    for b in sorted(cand, key=lambda b: (b["layout"], b["city"])):
        w(f"- {short(b['city'])} -> {short(b['entry'])} (layout {b['layout']}, x{b['layout_uses']}): "
          + ", ".join(sorted(n for _, n in persistent(b))))
    w("")

    w("## Layouts reused by multiple maps, with state vs without\n")
    by_layout = collections.defaultdict(list)
    for b in buildings:
        by_layout[b["layout"]].append(b)
    for lay, bs in sorted(by_layout.items(), key=lambda kv: -len(kv[1])):
        if layout_users[lay] and len(layout_users[lay]) > 1:
            n_state = sum(1 for b in bs if persistent(b))
            w(f"- {lay}: {len(layout_users[lay])} maps; {len(bs)} city buildings, {n_state} with persistent state")
    w("")

    w("## Per city\n")
    for city in cities:
        bs = [b for b in buildings if b["city"] == city]
        if not bs:
            continue
        w(f"### {short(city)} ({maps[city]['map_type'][9:]})\n")
        w("| entry | maps | layout reuse | persistent state | other |")
        w("|---|---|---|---|---|")
        for b in bs:
            p = sorted(n for _, n in persistent(b))
            o = sorted(f"{c}:{n}" for c, n in b["effects"] if c in ("hide", "battle"))
            tag = "unique" if b["layout_uses"] == 1 else f"x{b['layout_uses']}"
            extra = f" (+{len(b['maps']) - 1} inner)" if len(b["maps"]) > 1 else ""
            exits = f" EXITS->{','.join(short(x) for x in b['exits'])}" if b["exits"] else ""
            w(f"| {short(b['entry'])}{extra}{exits} | {len(b['maps'])} | {tag} | {', '.join(p)[:300]} | {', '.join(o)[:200]} |")
        w("")

    # State index and groups linked by shared state.
    state_keys = collections.defaultdict(lambda: ([], []))
    for i, b in enumerate(buildings):
        for _, n in persistent(b):
            state_keys[n][0].append(i)
        for n in b["reads"]:
            state_keys[n][1].append(i)
    parent = list(range(len(buildings)))

    def find(x):
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x

    shared = {k: v for k, v in state_keys.items() if len(set(v[0]) | set(v[1])) >= 2 and v[0]}
    for k, (setters, readers) in shared.items():
        ids = sorted(set(setters) | set(readers))
        for i in ids[1:]:
            parent[find(i)] = find(ids[0])
    w("## State shared between buildings\n")
    w("A key appears here if some building changes it and at least one other building (or the same one) reads or changes it. "
      "Common scripts (e.g. Pokemon Center healing) can link unrelated buildings; scan for surprises.\n")
    for k in sorted(shared):
        s, r = shared[k]
        w(f"- {k}: set in {', '.join(sorted({short(buildings[i]['entry']) for i in s}))}; "
          f"read in {', '.join(sorted({short(buildings[i]['entry']) for i in r if i not in s}))}")
    w("")
    groups = collections.defaultdict(list)
    for i in range(len(buildings)):
        groups[find(i)].append(i)
    w("## Building groups linked by shared state (size >= 2)\n")
    for g in sorted(groups.values(), key=len, reverse=True):
        if len(g) >= 2:
            w(f"- {len(g)}: " + ", ".join(f"{short(buildings[i]['city'])}/{short(buildings[i]['entry'])}" for i in g[:40])
              + (" ..." if len(g) > 40 else ""))
    w("")

    text = "\n".join(out)
    if args.out:
        with open(args.out, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
    else:
        sys.stdout.write(text)
    if args.csv:
        with open(args.csv, "w", encoding="utf-8", newline="\n") as f:
            f.write("city,entry,maps,layout,layout_uses,persistent_state,other\n")
            for b in buildings:
                p = " ".join(sorted(n for _, n in persistent(b)))
                o = " ".join(sorted(f"{c}:{n}" for c, n in b["effects"] if c in ("hide", "battle")))
                f.write(f"{short(b['city'])},{short(b['entry'])},{len(b['maps'])},{b['layout']},{b['layout_uses']},{p},{o}\n")


if __name__ == "__main__":
    main()
