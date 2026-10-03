# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

TRAODLE is a Windows console tool (Visual C++) that unpacks and converts Tomb Raider game assets into Autodesk formats: **FBX (ASCII)** and **Maya ASCII (.ma)**, plus textures (DDS/TGA/BMP). Code comments and commit messages are in Italian.

At startup it asks for a mode on stdin:
- `0` – Tomb Raider Anniversary Edition: `CLUSTER` → LVC (geometry), TEXSET (textures), MDC (models)
- `1` – Tomb Raider: The Angel of Darkness: `.GMX` or `.GMX.CLZ` → RMX (rooms), ZONE, CAM, CLN (collisions), SCX (scripts: each contains Small 2.x AMX bytecode, exported as `.amx` + disassembly `.asm` + decompiled pseudo-Pawn `.p` by `TRAOD/SCX/`). Also asks for target renderer (`TargetRenderer`: 1 = Maya Hardware 2.0, 2 = Arnold), which changes how materials are exported.
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
- `main.cpp` and `old_Zone_Exporter.h` are legacy files that aren't part of the build; the real entry point is `TRAODLE.cpp`.

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
