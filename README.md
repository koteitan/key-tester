# Keyboard Dropout & Chatter Tester

A Linux command-line tool to detect keyboard hardware issues.

## tests

### (1) Hold Test
- Press and hold a key.
- If dropout (short release) is detected, A warning message will be displayed.

### (2) Chatter Test
- Type two keys alternately.
- If chatter (multiple registrations) is detected, A warning message will be displayed.

### Build

```bash
make
./key-tester
```
## License

[MIT](LICENSE)
