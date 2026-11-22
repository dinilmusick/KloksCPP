# KloksCPP

KloksCPP is a portable, header-only C++17 reusable-services foundation. It provides caller-driven transport/meter/time conversion, MIDI packet codecs and routing, Open Sound Control (OSC) message and bundle codecs, host-injected device adapters, lifecycle registry, RAII event subscriptions and a small renderer-independent state graph.

## Build and test

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build --prefix "$PWD/installed"
cmake -S consumer -B consumer-build -DCMAKE_PREFIX_PATH="$PWD/installed"
cmake --build consumer-build
./consumer-build/consumer
```

Consumers use `find_package(KloksCPP 0.1 CONFIG REQUIRED)` and link `KloksCPP::klokscpp`. C++17 and the standard library are the only dependencies. No background workers or device sockets are created; caller-supplied `MidiHost` and `OscHost` control all transport side effects.

## Cross-language parity boundaries

| TypeScript Kloks contract | Native counterpart | Notes |
| --- | --- | --- |
| `@ultrst-armada/kloks/clock` | `TransportClock`, `Meter`, `TransportPosition`, `TimingMath` | Caller-driven transport, no browser scheduling, clock ticks and basic tempo changes; full scheduler/transport automation unsupported |
| `/midi` | `MidiMessage`, `MidiDecoder`, `MidiRouter`, `MidiHost` | Complete single-packet channel/system/SysEx codec with running status; no OS MIDI enumeration or streaming input decoder |
| `/osc` | `OscMessage`, `OscBundle`, `OscRouter`, `OscHost` | OSC 1.0 `i,f,s,b,T,F,N` and flat message bundles; nested bundle/advanced tags omitted |
| `/services` | `ServiceRegistry`, `ServiceDefinition`, `ServiceError` | Synchronous operations, dependency ordering, callbacks, host injection; async/network protocol adapters are host-owned |
| `/state-graph` | `StateGraph`, `EventBus<Value>` | Key/value nodes, cycle-checked edges and observer events; graphics/predictive/spatial domain UI excluded |
| `/backend`, `/ai-inference`, `/wasm-runtime` | Intentionally unsupported | Platform integrations and AI/toolchain-specific features require independent native adapters; do not claim TS parity |

Pinned source comparison (2026-11-22): Kloks `286377a03a8c5973c592b61ccaaa3864d060e8cc`, GutCPP `443ad9397c081dd596e784c50cff14c1ecad95c6`, KrnlCPP `b2b9814e2fb7034a72d5f9c3b5fd6a3b005ed883`, FrnlCPP `536a46228a8fde46fddc95447ca264fa2ed5fd6b`.

## Behavioral and ownership contracts

- `TransportClock` never reads a wall clock; the caller supplies elapsed seconds. Positive finite BPM/PPQN are validated, overflow throws; near-integer fractional ticks normalize within 32 double ULP for consistent common decimal increments. This is an explicitly documented quantization policy, **not** sample-accurate audio timing.
- `Meter` requires representable beat resolution and produces canonical zero-based bar/beat/tick positions. Strict inverse and overflow checks are provided.
- `ServiceRegistry` is a single-threaded owner of definitions. Starting a service starts dependencies; startup failures roll back newly started services. Reentrant and multi-threaded concurrent service mutation are not supported. `stop_all` and destructor dispose services in reverse start order.
- `EventBus` subscriptions have move-only RAII lifetime, and dispatch safely skips callbacks disposed during emission. Application-owned hosts must outlive their routers.
- MIDI data and OSC packets are validated before encoding/decoding. `MidiDecoder` consumes complete packets; host stream framing is external. OSC parser limits decoded packets to 1 MiB and supports messages/flat bundles with aligned big-endian framing.

Remaining unverified areas: actual MIDI hardware, real UDP networking, TypeScript differential fixtures and wider platform services. MSVC and the external CMake consumer are certified by the core workflow; native adapters have independent CI. See GitHub CI for exact commit evidence; no full parity is claimed.

## Optional pinned GutCPP/KrnlCPP interoperability

Include `<kloks/native_adapters.hpp>` when actual native GutCPP and KrnlCPP headers are supplied. `call_gut` and `call_krnl` return each foundation's native structured results; `bind_krnl_service` borrows an existing native `krnl::Service` with lifecycle and operation forwarding. **The external service must outlive the KloksCPP registry**. No duplicate kernel or hidden worker is introduced. Standalone KloksCPP remains dependency-free.

For opt-in CMake testing, use `-DKLOKSCPP_TEST_NATIVE_FOUNDATIONS=ON` and provide the pinned headers under `.deps/GutCPP/include`, `.deps/KrnlCPP/include`, or set the matching CMake path options. The dedicated native-foundations workflow verifies the exact pinned source and strict C++17 builds on Linux, macOS and Windows. It is not full TypeScript parity.
