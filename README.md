# Keyboard Dropout & Chatter Tester

A Linux command-line tool to detect keyboard hardware issues.

## Features

### Auto Detection Mode
Press any keys freely. The tool automatically detects:

**1. Dropout Detection**
- Detects when key repeat suddenly stops and then resumes
- 2-stage confirmation prevents false positives
  - Stage 1: Marks as potential dropout when repeat stops (100-1000ms)
  - Stage 2: Confirms dropout only when the same key is pressed again
- No warnings for intentional key releases

**2. Chatter Detection**
- Detects key bouncing when the same key is pressed consecutively
- Within 200ms threshold
- Suppressed for 1 second after dropout/hold end to avoid false warnings

### Display
- Real-time key press/repeat monitoring
- Red background warnings for detected issues
- Summary statistics on exit

## Installation

### Requirements
- GCC (C99 or later)
- Linux OS with terminal support
- WSL2 compatible (uses terminal input instead of /dev/input)

### Build

```bash
make
./key-tester
```
## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
