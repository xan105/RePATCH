<p align="center">
  <img src="https://github.com/xan105/RePATCH/raw/main/logo.png">
  <br /><em>Patching process memory at runtime with just a config file!</em>
</p>

This project is an in-memory binary patcher. Instead of altering file(s) on disk, it modifies the memory of a running process.

It scans memory for byte patterns and when a pattern is found, it applies the specified byte modifications at a given offset.
All changes are applied directly to memory at runtime and are not persistent.
Patches are defined in a JSON config file and can be enabled or disabled individually.

🐧 This software has an emphasis on being compatible with Linux/Proton.
  
💻 This software is for my own personal use but feel free to use it.

Config
======

Patches are defined as an array of objects in a JSON config file:

_Example:_

```json
[
  {
    "name": "Skip Intro",
    "enable": true,
    "pattern": "74 10 C7 ?? 0C 00 00 00",
    "offset": "0x00",
    "value": "90 90",
    "match": "first",
    "module": "UnityPlayer.dll"
  }
]
```

By default the config file is `patch.json` but you can specify the config file path with the env. var `REPATCH_FILEPATH`

Each _"patch object"_ has the following properties:

- `name?: string`: Describe the patch.
- `enable?: bool`: Enable (default) or disable the patch.
- `pattern: string`: Hexadecimal string pattern to search for in memory. Use `?` for wildcard, whitespace are ignored. 
- `offset?: number | string`: Address offset (default to 0).
  
  Because hex numbers are not valid in JSON. Offset can either be a number or a string:
  
  + If `number` then offset is **in decimal**
  + If `string` then offset is expected to be a **hex number represented as a string** and must start with `0x`
                                         
- `value: string`: hexadecimal string representing the sequence of bytes to be written. Whitespace are ignored.
- `match?: "first" | "last" | "all"`: Return the first match (default), all matches, or the last match.
- `module?: string`: When specified, scan module memory region instead of process.

> [!CAUTION]
> Incorrect patterns or values may cause crashes.

Usage
======

Set up the config file with a list of patch(es) to apply then inject RePatch into the target process.

You will need a DLL injector. A quick google search will find you plenty on GitHub.<br />
🐧 On Linux the classic combo `createRemoteThread()` + `LoadLibrary()` from _Kernel32_ works under Wine/Proton.

Alternatively, here are some of my own:

- [xan105/Mini-Launcher](https://github.com/xan105/Mini-Launcher):

  > CLI launcher with DLL Injection, Lua Scripting, Splash screen, and other goodies.

  <details><summary>Example (xan105/Mini-Launcher)</summary>
  
  ```json
  {
    "bin": "Binaries/NMS.exe",
    "env": {
      "REPATCH_FILEPATH": "%CURRENTDIR%\\Addons\\patch.json"
    },
    "addons": [
      { "path": "Addons/RePATCH.dll", "required": true }
    ]
  }
  ```
  </details>

- [xan105/node-remote-thread](https://github.com/xan105/node-remote-thread):

  > Node.js NAPI Native addon for Windows DLL injection with support for Wow64 and Unicode path.

  <details><summary>Example (xan105/node-remote-thread)</summary>

  ```js
  import { env } from "node:process";
  import { spawn } from "node:child_process";
  import { dirname, resolve } from "node:path";
  import { createRemoteThread } from "@xan105/remote-thread";

  const EXECUTABLE = "G:\\P5R\\P5R.exe";
  const ADDON = "G:\\P5R\\RePATCH.dll";
  const ARGS = [];

  const binary = spawn(EXECUTABLE, ARGS, {
    cwd: dirname(EXECUTABLE),
    stdio:[ "ignore", "ignore", "ignore" ], 
    detached: true,
    env: {
      ...env,
      "REPATCH_FILEPATH": resolve("patch.json")
    }
  });

  binary.once("spawn", () => {
    binary.unref();
    createRemoteThread(binary.pid, ADDON);
  });
  ```
  </details>

> [!TIP]
> Consider changing the file extension from .dll to .asi to help prevent false positive with Windows Defender.

Build
=====

🆚 Visual Studio 2022

Solution: `./vc/repatch.sln`
Output: `./build/output/${platform}/${config}`

Github Actions
==============

`./.github/workflows/vs-build-on-windows.yaml`

Build all targets (debug and release) and create a release.
