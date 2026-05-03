# This module provides a layered interface for external flash memory operations and logging with the W25N04ZEIR flash chip

**High-level structure:**
- `W25N04KVZEIR`: low-level QSPI flash driver
- `FlashTask`: high level flash interface (implements flash driver)

---

### W25N04KVZEIR Driver (Low-Level)
Handles direct communication with the flash chip via QSPI.

**Core functions:**
- `W25N_reset()`
- `W25N_read()`
- `W25N_program_data()`
- `W25N_block_erase()`
- `W25N_status()`

---

### FlashTask (High-Level)
`FlashTask` handles:
- Writing (`AppendFlash`)
- Reading (`ReadFlash`)
- Erasing (`EraseBlocks`)
- Command handling

It abstracts flash complexity such as:
- Page boundaries
- Block erase requirements
- Chunked read/write operations

- flash dumps are handled by the configured LoggingService which formats the raw flash contents into human readable format, based on what
sensor(s) are configured for the system

---