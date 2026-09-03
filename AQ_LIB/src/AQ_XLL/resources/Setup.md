# AQ_ADDIN — xlOil Static XLL Setup

Build configuration notes for the `AQ_ADDIN` project in the AQ solution.
Records the settings required to link xlOil as a **static XLL**, and the
upstream gap that has to be worked around.

Written after migrating from XLL Plus / VS2017 to xlOil / VS2022.

---

## 1. Route chosen

xlOil offers three consumption routes:

| Route | Description |
|---|---|
| Plugin DLL | A DLL loaded by an existing xlOil.xll install |
| Dynamic XLL | An XLL that links xlOil.dll at runtime |
| **Static XLL** | **A standalone XLL with xlOil linked in. This is what AQ uses.** |

Static was chosen to match the previous XLL Plus arrangement: one
self-contained `.xll` with no DLL search-path dependencies. The trade-off is
that xlOil's own object cache functions (`xloRef`, `xloVal`) are not
available — AQ has its own object cache, so this costs nothing.

Reference: https://xloil.readthedocs.io/en/stable/xlOil_Cpp/GettingStarted.html

---

## 2. xlOil version and location

- xlOil **0.19.0**
- Root: `AQ_EXTERNAL_LIBS\library\xlOil-0.19.0`
- Built from source in VS2022 (the binary releases ship only the headers and
  the dynamic-link import library; the static libs must be built yourself)

Static libraries consumed, from `build\x64\ReleaseStatic` and
`build\x64\DebugStatic`:

- `xlOil-COM.lib`
- `xlOil-Dynamic.lib`
- `xlOil-External.lib`
- `xlOil-Funcs.lib`
- `xlOil-XLL.lib`
- `xlOilHelpers.lib`  — see section 5, this one needs manual work

---

## 3. Required preprocessor definitions

Set on **AQ_ADDIN**, for every configuration (Release x64, Debug x64):

**C/C++ → Preprocessor → Preprocessor Definitions**

```
XLOIL_STATIC_LIB
SPDLOG_COMPILED_LIB
```

Keep *Inherit from parent or project defaults* ticked.

### Why these matter

**`XLOIL_STATIC_LIB`** — xlOil's headers wrap public functions in an
`XLOIL_EXPORT` macro. Undefined, it expands to `__declspec(dllimport)`, so the
compiler emits DLL-style import calls. Defined, it expands to nothing and calls
go directly into the static libs.

Symptom if missing: unresolved externals with an `__imp_` prefix, e.g.

```
unresolved external symbol __imp_?CalcCancelled@Event@xloil@@...
```

The `__imp_` prefix is the diagnostic — it means the compiler is generating a
DLL import for something that lives in a static library.

**`SPDLOG_COMPILED_LIB`** — spdlog's headers carry full function bodies. Without
this define every translation unit compiles its own copy, colliding with the
copy already compiled into the xlOil libs.

Symptom if missing: a block of ~24 `LNK2005` duplicate-symbol errors, e.g.

```
xlOil-External.lib(spdlog.obj) : error LNK2005:
  "...spdlog::logger::flush_(void)" already defined in main.obj
```

Both defines are set by xlOil's own `libs\xlOilStaticLib.props`. AQ sets them
directly in the project instead, to keep all settings in the `.vcxproj`.

---

## 4. Required linker inputs

Set on **AQ_ADDIN**, for every configuration:

**Linker → Input → Additional Dependencies**

```
xlOilHelpers.lib
Oleacc.lib
```

- `xlOilHelpers.lib` provides `writeWindowsError`, `getEnvironmentVar`,
  `stableGuidFromString`, `guidToWString`, `getWindowsRegistryValue` — pulled in
  by `xlOil-XLL` and `xlOil-COM`.
- `Oleacc.lib` is the Windows accessibility library, providing
  `AccessibleObjectFromWindow`, required by `xlOil-COM.lib(Connect.obj)`.

**Linker → General → Additional Library Directories** must include the matching
xlOil static output folder:

```
$(xlOilRoot)\build\x64\ReleaseStatic     (Release)
$(xlOilRoot)\build\x64\DebugStatic       (Debug)
```

---

## 5. Upstream gap: xlOilHelpers has no static configurations

**This is the non-obvious one. Expect to redo it after any xlOil upgrade.**

In xlOil 0.19.0, the `xlOilHelpers` project defines only `Debug` and `Release`
configurations. It has no `DebugStatic` or `ReleaseStatic`, even though
`xlOil-COM` and `xlOil-XLL` — which do have them — depend on its symbols.

The result is that `xlOilHelpers.lib` is simply never produced in the static
output folders, and AQ_ADDIN fails to link with five unresolved `xloil::`
symbols.

### Fix

In the **xlOil solution**, once per configuration:

1. Build → Configuration Manager
2. Set active solution configuration to `ReleaseStatic` (then repeat for
   `DebugStatic`), platform `x64`
3. On the **xlOilHelpers** row, Configuration dropdown → `<New...>`
4. Name: `ReleaseStatic`, Copy settings from: `Release`
5. Leave *Create new solution configurations* **unticked**
6. Confirm the Build checkbox is ticked
7. Right-click `xlOilHelpers` → Build

The output directory is inherited and expressed via `$(Configuration)`, so the
`.lib` lands in the correct static folder automatically. No path edits needed.

---

## 6. Verifying the add-in works

A clean link does not prove a working add-in. Test the round trip:

```cpp
#include <xloil/xlOil.h>
#include <xloil/XllEntryPoint.h>
using namespace xloil;
struct AlgoQuantLib {
    static std::wstring addInManagerInfo() { return L"Algo Quant Lib"; }
};
XLO_DECLARE_ADDIN(AlgoQuantLib);
// Test Function
XLO_FUNC_START(aqEcho(const ExcelObj* arg))
{
    return returnValue(arg->toString());
}
XLO_FUNC_END(aqEcho).threadsafe()
.help(L"Returns the argument provided")
.arg(L"Value", L"Any value");
```

Then:

1. Close Excel completely (check Task Manager for a lingering `EXCEL.EXE`, or
   the previous `.xll` stays locked)
2. Reopen Excel, drag `AQ_ADDIN.xll` onto the window
3. In a cell: `=aqEcho("hello")` → returns `hello`

### API note

`returnValue` is a **free function in the `xloil` namespace**
(`include\xloil\StaticRegister.h`), not a static member of `ExcelObj`. Some
xlOil documentation shows `ExcelObj::returnValue(...)`, which does not compile
against 0.19.0. Likewise `toString()` takes no default-value argument in this
version. When in doubt, grep the headers rather than trusting the docs:

```
findstr /s /n /C:"returnValue" include\*.h
```

---

## 7. Add-in registration in Excel

Dragging an `.xll` onto a running Excel loads it **temporarily**. It will not
appear under *File → Options → Add-ins → Manage: Excel Add-ins → Go…*, because
that list shows only persisted registrations. Absence from that list after a
drag-and-drop load is expected and is not a fault.

To register permanently: *Manage: Excel Add-ins → Go… → Browse…* and select the
`.xll`. The description shown is the string returned by `addInManagerInfo()`.

If a second drag-and-drop reports the add-in is already loaded, that confirms
the first load succeeded.

---

## 8. Known risk for Debug builds

Debug is where a **C runtime mismatch** shows up. If the xlOil `DebugStatic`
libraries were built with `/MDd` and AQ_ADDIN is on `/MTd` (or vice versa), the
result is a fresh set of duplicate-symbol errors that superficially resemble the
spdlog problem in section 3 but have an unrelated cause.

Check **C/C++ → Code Generation → Runtime Library** on both sides and make them
agree.

---

## 9. Diagnostic note

`dumpbin /symbols` returns an empty symbol table for the xlOil static libs —
only the header and `File Type: LIBRARY`. This is because they are built with
whole program optimisation (`/GL`), and LTCG object files carry no readable COFF
symbol table. It is not a sign of a corrupt library. To locate a symbol's home,
search the source or the build outputs instead.