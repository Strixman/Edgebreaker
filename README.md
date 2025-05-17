**EdgeBreaker Compressor/Decompressor**

This project provides a robust implementation of the **EdgeBreaker** compression algorithm for manifold and non-manifold triangle meshes. With support for OBJ, OFF, and our intermediate OVX format, it enables high compression ratios and fast encoding/decoding workflows.

---

## Table of Contents

1. [Features](#features)
2. [Prerequisites](#prerequisites)
3. [Building](#building)
4. [Usage](#usage)

   * [Modes](#modes)
   * [File Types](#file-types)
   * [Examples](#examples)
5. [License](#license)

---

## Features

* **EdgeBreaker compression**: Implements the classic topological crawler with `C`, `L`, `E`, `R`, `S` symbols for optimal connectivity encoding.
* **Multi-format support**: Read/write OBJ, OFF, OVX (uncompressed), BCO (binary compressed), and CO (ASCII compressed).
* **Three modes**:

  * `compress` – encode mesh connectivity
  * `decompress` – decode into a standardized mesh
  * `ovx` – convert OBJ/OFF into OVX for unified pre-processing
* **Command-line interface** with clear error handling and usage hints.

## Prerequisites

* C++17-compatible compiler (e.g. `g++`, `clang++`)
* CMake 3.10 or higher

## Building

```bash
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The above generates an executable named `edgebreaker` in the `build/` directory.

## Usage

```text
Usage: edgebreaker <compress|decompress|ovx> <input_file> <output_file>
```

### Modes

* **compress**: Encode an input mesh (OBJ, OFF, or OVX) to a compressed stream (BCO or CO).
* **decompress**: Decode a compressed mesh (BCO or CO) back into a standard mesh (OBJ, OFF, or OVX).
* **ovx**: Convert OBJ/OFF into OVX (uncompressed binary) for downstream processing.

### File Types

| Extension | Type                                     |
| --------- | ---------------------------------------- |
| `.obj`    | Wavefront OBJ (ASCII triangles + coords) |
| `.off`    | Object File Format                       |
| `.ovx`    | Extended opposite corner table format    |
| `.bco`    | Binary compressed EdgeBreaker stream     |
| `.co`     | ASCII compressed EdgeBreaker stream      |

### Examples

* **Compress an OFF to binary**

  ```bash
  edgebreaker compress model.off model.bco
  ```

* **Compress an OBJ to ASCII**

  ```bash
  edgebreaker compress model.obj model.co
  ```

* **Decompress to OVX**

  ```bash
  edgebreaker decompress model.bco model.ovx
  ```

* **Convert OBJ to OVX**

  ```bash
  edgebreaker ovx model.obj model.ovx
  ```

If invalid arguments or file types are passed, the tool will print a usage hint and exit.

## License

This project is released under the MIT License. See [LICENSE](LICENSE) for details.
