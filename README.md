# QR Code Generator

A lightweight, C-based QR code generator that creates QR codes from text input with support for different error correction levels.

## Features

- Generate QR codes from text input
- Byte mode encoding only (ISO-8859-1/UTF-8 compatible)
- Support for multiple error correction levels (L, M, Q, H)
- Pure C implementation with no external dependencies
- Simple command-line interface
- Supports standard QR code versions only (no Micro QR support)

## Prerequisites

- C compiler (GCC or Clang)
- Make

## Building the Project

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd qr_gen
   ```

2. Build the project:
   ```bash
   make
   ```
   This will create the `qr-gen` executable in the `build/release` directory.

   For a release build without debug output:
   ```bash
   make NDEBUG=1
   ```

## Usage

```bash
./build/release/qr-gen "Your text here" [error_correction]
```

### Error Correction Levels

- `L` - Low (7% of codewords can be restored)
- `M` - Medium (15% of codewords can be restored) - **Default**
- `Q` - Quartile (25% of codewords can be restored)
- `H` - High (30% of codewords can be restored)

### Output Format

The program outputs the QR code in SVG (Scalable Vector Graphics) format to standard output (stdout). You can redirect the output to a file:

```bash
./build/release/qr-gen "Your text here" > qrcode.svg
```

### Examples

Generate a QR code with default error correction (M):
```bash
./build/release/qr-gen "Hello, World!"
```

Generate a QR code with high error correction:
```bash
./build/release/qr-gen "Important Data" H
```

## Running Tests

The project includes unit tests to verify the functionality of core components. To run the tests:

```bash
make test
```

## Project Structure

- `qr/` - Main source code
  - `ecc.[ch]` - Error correction coding
  - `enc.[ch]` - Data encoding
  - `mask.[ch]` - Mask pattern generation
  - `matrix.[ch]` - QR code matrix operations
  - `patterns.[ch]` - QR code patterns and alignment
  - `qr.[ch]` - Main QR code functionality
  - `types.h` - Common type definitions
  - `main.c` - Command-line interface
- `test/` - Unit tests
- `refs/` - Reference materials

## References

- [ISO/IEC 18004:2024](https://www.iso.org/standard/83389.html) - QR Code specification
