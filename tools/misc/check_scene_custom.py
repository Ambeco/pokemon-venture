#!/usr/bin/env python3
"""Guard for scene `custom` steps (design_scenes.md, "Scene runner").

A scene is layout-independent: it names actors by slot and tiles relative to actors and anchors. A `custom` step
is the escape hatch for what the step vocabulary lacks, so its script must not bring the layout back in. This
check fails when

  * a SCENE_CUSTOM(...) in src/ or data/ names a script that is not defined in data/scripts/scene_custom.inc; or
  * a script in data/scripts/scene_custom.inc has a label not starting with `SceneCustom_`, moves or places an
    object (applymovement, setobjectxy, addobject, turnobject, ...), contains a movement list (`step_end`,
    `walk_*`, `face_*`), or names a local id (`LOCALID_*`).

Use a scene step instead (walk, face, place, hide, show). Run from the repo root; `--self-test` runs the
checker's own tests.
"""
import re
import sys
from pathlib import Path

CUSTOM_FILE = Path("data/scripts/scene_custom.inc")
LABEL_PREFIX = "SceneCustom_"
SOURCE_DIRS = ("src", "data")

FORBIDDEN_COMMANDS = {
    "applymovement", "applymovementat", "waitmovement", "waitmovementat", "setobjectxy", "setobjectxyperm",
    "copyobjectxytoperm", "addobject", "removeobject", "showobjectat", "hideobjectat", "turnobject",
    "moveobjectoffscreen", "setobjectmovementtype", "setobjectsubpriority", "resetobjectsubpriority",
    "createvobject", "setvobjectpriority", "opendoor", "closedoor", "faceplayer", "freezeobjects", "step_end",
}
MOVEMENT_COMMAND = re.compile(r"^(?:walk|face|jump|slide|run|player_run|player_walk|delay|set_invisible|set_visible|emote|lock_facing|unlock_facing)_\w+$")
LOCAL_ID = re.compile(r"\bLOCALID_\w+")


def split_scripts(text):
    """Maps each label (`Name::` or `Name:`) to its body as (line number, stripped line), comments removed."""
    scripts, name = {}, None
    for number, raw in enumerate(text.splitlines(), 1):
        line = raw.split("@", 1)[0].rstrip()
        label = re.match(r"^(\w+)::?\s*$", line)
        if label:
            name = label.group(1)
            scripts[name] = []
        elif name is not None and line.strip():
            scripts[name].append((number, line.strip()))
    return scripts


def find_violations(text, filename=str(CUSTOM_FILE)):
    problems = []
    for name, body in split_scripts(text).items():
        if not name.startswith(LABEL_PREFIX):
            problems.append(f"{filename}: label {name} does not start with {LABEL_PREFIX}; did you mean {LABEL_PREFIX}{name}?")
        for number, line in body:
            command = line.split()[0].rstrip(",")
            if command in FORBIDDEN_COMMANDS or MOVEMENT_COMMAND.match(command):
                problems.append(f"{filename}:{number}: custom script {name} uses `{command}`; custom scripts must not move or place objects, so use a scene step (walk, face, place, hide, show) to keep the scene layout-independent")
            if re.match(r"^(?:call|goto)(?:_if_\w+)?$", command):
                target = line.split(",")[-1].split()[-1]
                if not target.startswith(LABEL_PREFIX):
                    problems.append(f"{filename}:{number}: custom script {name} jumps to {target}, outside the checked {LABEL_PREFIX} scripts, where objects could move; did you mean to copy the part you need into {CUSTOM_FILE}?")
            found = LOCAL_ID.search(line)
            if found:
                problems.append(f"{filename}:{number}: custom script {name} names a local id (`{found.group(0)}`); scenes address actors by slot and tiles by anchor, never by local id")
    return problems


def find_unknown_customs(sources, defined):
    """`sources` maps a file name to its text; every SCENE_CUSTOM(x) must name a script defined in the custom file."""
    problems = []
    for filename, text in sources.items():
        for found in re.finditer(r"\bSCENE_CUSTOM\(\s*(\w+)\s*\)", text):
            name = found.group(1)
            line_start = text.rfind("\n", 0, found.start()) + 1
            if text.count('"', line_start, found.start()) % 2 == 1:
                continue  # inside a string literal, such as an error message
            if name not in defined:
                line = text.count("\n", 0, found.start()) + 1
                problems.append(f"{filename}:{line}: SCENE_CUSTOM({name}) is not a script in {CUSTOM_FILE}; did you mean to define `{LABEL_PREFIX}{name}::` there (the guard checks it for movement and local ids)?")
    return problems


def self_test():
    good = "SceneCustom_SwapParty::\n\tsetvar VAR_0x8004, 1 @ applymovement in a comment\n\tspecial SavePlayerParty\n\treturn\n"
    assert find_violations(good) == [], find_violations(good)
    chained = "SceneCustom_A::\n\tgoto_if_set FLAG_X, SceneCustom_B\n\treturn\n\nSceneCustom_B::\n\treturn\n"
    assert find_violations(chained) == [], find_violations(chained)
    for bad in (
        "SceneCustom_A::\n\tapplymovement 3, Move\n\treturn\n",
        "SceneCustom_A::\n\tsetobjectxy LOCALID_X, 1, 2\n\treturn\n",
        "SceneCustom_A::\n\twalk_up\n\tstep_end\n",
        "SceneCustom_A::\n\tgoto_if_eq VAR_0x8000, LOCALID_FOO, SceneCustom_B\n",
        "NotACustom::\n\treturn\n",
        "SceneCustom_A::\n\tcall Common_EventScript_MoveSomeone\n\treturn\n",
        "SceneCustom_A::\n\tgoto_if_set FLAG_X, Elsewhere\n",
    ):
        assert find_violations(bad), f"missed a violation in {bad!r}"
    assert find_unknown_customs({"x.h": "SCENE_CUSTOM(Foo)"}, {"SceneCustom_Foo"}) != []
    assert find_unknown_customs({"x.h": "SCENE_CUSTOM(SceneCustom_Foo)"}, {"SceneCustom_Foo"}) == []
    assert find_unknown_customs({"x.c": 'fail("did you mean SCENE_CUSTOM(Foo)?")'}, set()) == []
    print("check_scene_custom: self-test passed")


def main():
    if "--self-test" in sys.argv:
        self_test()
        return 0
    text = CUSTOM_FILE.read_text(encoding="utf-8") if CUSTOM_FILE.exists() else ""
    problems = find_violations(text)
    sources = {}
    for directory in SOURCE_DIRS:
        for path in Path(directory).rglob("*"):
            if path.suffix in (".h", ".c", ".inc") and path != CUSTOM_FILE and path.name != "scene.h":
                content = path.read_text(encoding="utf-8", errors="replace")
                if "SCENE_CUSTOM(" in content:
                    sources[str(path)] = content
    problems += find_unknown_customs(sources, set(split_scripts(text)))
    for problem in problems:
        print(problem, file=sys.stderr)
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
