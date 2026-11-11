# Handy C++ logger (`ict::logger`) - File rotation module

The `rotate` class in the `ict::logger` namespace provides functionality for logging to a file with automatic rotation when the file exceeds a specified size. It maintains a set number of backup copies, renaming older files as new ones are created.

## Usage

```cpp
#include "rotate.hpp"

ict::logger::rotate r("logfile.log", 1024, 3); // max_size = 1024 bytes, max_copies = 3
r << "Log line " << 42 << std::endl; // Full std::ostream interface is available.

```
