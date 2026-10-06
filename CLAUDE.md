# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

TRAODLE is a Windows console tool (Visual C++) that unpacks and converts Tomb Raider game assets into Autodesk formats: **FBX (ASCII)** and **Maya ASCII (.ma)**, plus textures (DDS/TGA/BMP). Code comments and commit messages are in Italian.

At startup it asks for a mode on stdin:
- `0` – Tomb Raider Anniversary Edition: `CLUSTER` → LVC (geometry), TEXSET (textures), MDC (models)
- `1` – Tomb Raider: The Angel of Darkness: `.GMX` or `.GMX.CLZ` → RMX (rooms), ZONE, CAM, CLN (collisions), SCX (scripts: each contains Small 2.x AMX bytecode, exported as `.amx` + disassembly `.asm` + decompiled pseudo-Pawn `.p` by `TRAOD/SCX/`), CHR (characters: skeleton + skinned meshes + materials/textures + face blend shapes from the level's TMT files + facial animations from the TMS/".3" files, including those of other TMTs of the same face whose targets are the same in a different order (`CHR_Morph.remap`): FBX takes in the character FBX, one MA per animation in `\Animations` referencing `..\Characters\<CHR>.MA`, `TRAOD/CHR/`; CHR files starting with "NODE" are animated objects in a different, unsupported format), CAL (skeletal animations, `TRAOD/CAL/Export_CAL.cpp`: one MA per animation referencing `..\..\Characters\<CHR>.MA` plus a single `<CALNAME>.FBX` (skeleton + one take per animation) in `\Animations\<CALNAME>`; root motion is combined into the root joint (HIP), never on the character group, which parents the skinned meshes). AoD file layout: in levels `<CHAR>.CAL` holds the game animations shared by all models of the character (LARA.CAL for LARAD, LARAC1, LARAC2, ...) and `<CHAR>_DLG.CAL` the dialog animations, which start the facial sequences of the `.3` files through FIRE_MORPH_TRIGGER keys on the base TMT (`<CHAR>.TMT`); cutscenes (`CS_*` levels and `IG_*` in-game cutscenes inside normal levels) use per-actor `<CHAR>_<CUTSCENE>.CAL/.TMT/.TMS` plus `<CUTSCENE>.POS` (world position of every actor's HIP per frame, `TRAOD/CAL/POS_Struct.h`, matched by `CAL_ANIMATION.animID`), always applied to the reference CHR (LARAD, LARAC1, ...). The CHR is matched by name and animated-bone count; the cutscene MA replaces the blend shape targets of the referenced character with the cutscene TMT through `setAttr` reference edits. Attached-mesh tracks, other events and FX nodes are not exported. Also asks for target renderer (`TargetRenderer`: 1 = Maya Hardware 2.0, 2 = Arnold), which changes how materials are exported.
- `2` – AoD Remastered: `.MSH` → mesh

The input file path is `argv[1]` (typically drag-and-drop onto the exe). Output is written into a folder named after the level next to the input file (`\LEVELNAME\Rooms`, `\Zones`, `\Cameras`, `\Collisions`, ...), plus a log file.

## Build

Single-project solution `TRAODLE.sln` → `TRAODLE/TRAODLE.vcxproj`. There are no tests or linters.

```
msbuild TRAODLE.sln /p:Configuration=Release /p:Platform=x86
```

- Only the **Win32** configurations are fully set up (precompiled header creation via `TRAODLE.cpp`, `_CRT_SECURE_NO_WARNINGS`, absolute include dir `D:\AoD\C++\VISUAL C++\TRAODLE\TRAODLE`). The x64 configs lack these and likely won't build as-is. Release|Win32 uses toolset v141; the others use v143.
- Every `.cpp` must `#include "stdafx.h"` first (PCH). The bundled `ZLIB/*.c` files are set to `NotUsing`.
- New source files must be added to both `TRAODLE.vcxproj` and `TRAODLE.vcxproj.filters`.
- `Release/` (gitignored) is also used as a scratch directory for test input files (GMX/MSH) and their outputs.
- `main.cpp` and `old_Zone_Exporter.h` are legacy files that aren't part of the build; the real entry point is `TRAODLE.cpp`. `TRAOD/CHR/Reference_TRAODAE/` is reference material only, git-ignored and never added to the project: the old Animation Exporter's CHR/CAL code (its bone transforms are wrong, see `CHR_Struct.h`), the original Maya animation scenes (`ANIMS/`, `Kurtis/`) and, in `Tools/`, the original developers' tool sources. `Tools/` is the authoritative source for format details: `Libs/AnimLib/AnimAPI.h`/`.c` (CAL/CHR runtime structures: ANIMATION, BONE, FXNODE, ...), `AnimEdit/EditorToGame.cpp` (CAL export), `ScriptProducer/` (SCX), `GMX Masher/` (GMX directory), `CollisionProducer/` + `worldedit/Sys/collision.csc` (CLN attributes), `Db2Game/` (ACTOR.DB). The `[SRC]` tags in the `*_Struct.h` files refer to it.
- Encoding: many sources are ISO-8859-1 (Latin-1) with Italian accents in comments, all with CRLF. Editing tools that write UTF-8 corrupt the accents into U+FFFD; check `file <path>` before/after editing and keep Latin-1. New files should be plain ASCII + CRLF.
- Output testing: Maya 2016 and 2025 are installed; `"C:/Program Files/Autodesk/Maya2025/bin/mayapy.exe"` can open the exported MA/FBX headless to validate them (FBX import: `mel.eval('FBXImport -f "path" -t 1')`, take index is 1-based). The FBX writer must keep the `; FBX 7.5.0` comment consistent with `FBXVersion: 7500` and the GlobalSettings/PropertyTemplate entries, or recent FBX plugins reject the file or ignore animations. The FBX files are Z-up: import them into a Z-up Maya scene (`cmds.upAxis(axis='z')`) when comparing values, otherwise the importer's axis conversion changes the root node's curves. All MA files use `currentUnit -t ntsc` (30 fps, the AoD animation rate) and animations start at frame 0, so cameras (CAM) and CAL animations line up frame by frame. Joints use segment scale compensate (FBX `InheritType` 2, Maya default), as the TRAOD runtime does for tracks with `HASSSC`. With several skeletal takes in one FBX, Maya gives channels that have no curves in the imported take the values of another take, so `FBX_Write_SkeletalAnimation` writes a one-key curve for every T/R/S group that is animated in any other take of the file.

## Architecture

**Flow:** `TRAODLE.cpp::main` → `*_IO_Init(argv)` (fills a global IO object with paths and level name) → unpack container (`Decompress_CLZ`, `Export_GMX`, `Export_CLUSTER`) → iterate the extracted file list and dispatch by file type to `Export_<TYPE>(filename)`.

**Globals** (declared `extern` in `stdafx.h`): `AOD_IO`, `AE_IO`, `AODRemastered_IO` (classes in `Classes.h`: folder paths as both `string` and `LPWSTR`, the extracted file list, and `SearchFileIn*List` helpers), `TargetRenderer`, `msg_file_stream`, `mutex mu`. Code relies heavily on `SetCurrentDirectory` to the right output folder before writing files.

**Directory layout by game/format:**
- `TRAOD/<FMT>/` – AoD formats. Each has `<FMT>_Struct.h` (binary on-disk layouts read directly with `ifstream::read`), `<FMT>_Functions.h`, `<FMT>_Read*.cpp`, and `Export_<FMT>.cpp`.
- `TRAE/<FMT>/` – Anniversary Edition formats, same pattern.
- `TRAOD Remastered/MESH/` – Remastered mesh.
- `FBX/` and `MA/` – format-agnostic writers.
- `MATH/` – matrix/quaternion/rotation helpers (`math.h`).
- Root: shared helpers (texture decoders `Texture_*.cpp`, primitive builders `Draw*.cpp`, hashing, `Misc_Functions.h`).

**Export pattern (important):** readers don't write files. They fill two scene containers, `FBX_EXPORT` (`FBX/FBX_Classes.h`) and `MA_EXPORT` (`MA/MA_Classes.h`), with the shared scene classes from `Classes.h` (`Mesh`, `Material`, `Texture`, `Transform`, `Joint`, `Camera`, `Light`, `Locator`, ...). Then `FBX_Export(name, FBX)` and `MA_Export(name, MA)` serialize them. See `TRAOD/ZONE/Export_ZONE.cpp` for the canonical example. MA supports some Maya-only primitives (`PolyCube`, `PolyPlane`, `NurbsSurface`, `BossWave`, `Layer`). When adding a new scene feature, it usually has to be added to both writers. `Guida alle connections per file FBX.xlsx` documents FBX object connections.

**Logging:** use the `msg` macro from `OutMsgInterface.h` instead of `cout`:
```cpp
msg(msg::TGT::FILE_CONS, msg::TYP::WARN) << "text " << value;
```
`TGT` = FILE / CONS / FILE_CONS; `TYP` = OVR, LOG, DBG, WARN, ERR, FATAL. The macro captures `__FUNCTION__` for the log file. Export functions return `bool`. `false` gets logged as an error by the caller, and a fatal init failure goes through `Fatal_Error_Terminate()`.

**Resources:** `Resources/ColTex_*.tga` are collision-attribute textures embedded via `Resource.rc`/`resource.h` and written out with `SaveTGAResource` (used by the CLN exporter through `CLN_GetResource`).

**Version string** is `version` in `TRAODLE.cpp` (format `0.YYMMDD`).
