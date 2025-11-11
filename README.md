# QR Code Generator

A lightweight, C-based QR code generator that creates QR codes from text input with support for different error correction levels.

## Features

- Generate QR codes from text input
- Support for multiple error correction levels (L, M, Q, H)
- Pure C implementation with no external dependencies
- Simple command-line interface
- Unit tests for core functionality

## Prerequisites

- C compiler (GCC or Clang)
- Make

## Building the Project

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd qr
   ```

2. Build the project:
   ```bash
   make
   ```

   This will create the `qr` executable in the `build/main` directory.

## Usage

```bash
./build/main/qr "Your text here" [error_correction]
```

### Error Correction Levels

- `L` - Low (7% of codewords can be restored)
- `M` - Medium (15% of codewords can be restored) - **Default**
- `Q` - Quartile (25% of codewords can be restored)
- `H` - High (30% of codewords can be restored)

### Examples

Generate a QR code with default error correction (L):
```bash
./build/release/qr "Hello, World!"
```

Generate a QR code with high error correction:
```bash
./build/release/qr "Important Data" H
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

## License

[Specify License Here]

## References

- [ISO/IEC 18004:2015](refs/ISO_IEC_18004_2015_Standard.pdf) - QR Code specification

## Contributing

[Add contribution guidelines here]
