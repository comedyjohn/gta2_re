# Undefined behaviour that stays, and why

UndefinedBehaviorSanitizer (`build_port/ubsan64`, `cmake --preset ubsan64`) over the replay corpus
and the unit tests. Every report is either fixed (a `Fix:` commit) or listed here with its reason.
Re-run after changes in these places:

```bash
for r in ~/Games/gta2/corpus/replays/*.rep; do
    xvfb-run -a build_port/ubsan64/gta2 --replay "$r" --game-dir ~/Games/gta2/game --hash-log /dev/null
done 2>&1 | grep 'runtime error' | sort | uniq -c
```

## Left as they are

| Where | Report | Why it stays |
|---|---|---|
| `InputRecorder_67F8B0::get_input_bits_4CEAC0` | `field_8_input_masks[3 * idx + 8]`: index out of bounds of `s32[12]` | The original reads past the masks into the record buffer the same way. The value only feeds `bLog_input` logging. Phase 3 (arrays and spans) |
| `gtx_0x106C::get_car_remap_5AA3D0`, `Car_BC.cpp` (`SetRemap`) | `car_info::remap[1]` indexed past 1 | A variable-length array at the end of a structure read from the style file, declared `[1]` as in the original. Becomes a span over the file data in phase 3 (file readers) |
| `Frontend::intro_bik_exists_4B5FF0` | member call on a null `Frontend*` | A static function calls `intro_bik_4B5E50` through the global, which is null before the front end exists; the method reads no member. Phase 2/6 make it a free function |
| `InputDevices_1::make_input_devices_498800` | reference to a misaligned `GUID` at address 1 | `DIPROP_BUFFERSIZE` is DirectInput's `MAKEDIPROP(1)`, a GUID reference that is really an integer, as in the real `dinput.h`; the port's DirectInput compares the address. Goes with the platform interfaces in phase 6 |
| `Draw.cpp` (`pConvertColourBank`) | call through a pointer to another function type | The game's type takes `BYTE*`, GTA2Hax's `d3ddll.hpp` (the real DLL's interface) declares `s32`. The port's renderer ignores the call. Phase 6 replaces the `gbh_*` table with an interface |

## Fixed

| Commit | What |
|---|---|
| `Fix: tan(90) and tan(270) in the tangent table were undefined behaviour` | an out-of-range double to int conversion, which ARM would have done differently from x86 |
| `Fix: police crews read an uninitialised counter; a roadblock freed with delete[]` | AddressSanitizer: `PoliceCrew_38::field_35` read before being set (the original's bug too), and a `new`/`delete[]` mismatch |

Found by other means (different results between GCC and Clang builds), the static initialisation
order: `Fix: police crews read kFpEight_6FED48 as 0 in GCC builds`, `Fix: gDummyH_678584 was 0
instead of 0.2`, `Fix: the registry could be used before it was constructed`.
`port/tools/refactor/init_order.py` keeps globals from being initialised from another file's.
