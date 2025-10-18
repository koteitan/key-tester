# Keyboard Dropout & Chatter Tester

A Linux command-line tool to detect keyboard hardware issues.

## tests

### (1) Hold Test
- Press and hold a key.
- If dropout (short release) is detected, A warning message will be displayed.
```syslog
[00:33:03.489] 'a' repeat #1 (interval: 35.6ms)
[00:33:03.523] 'a' repeat #2 (interval: 34.1ms)
[00:33:03.558] 'a' repeat #3 (interval: 35.2ms)
[00:33:03.593] 'a' repeat #4 (interval: 34.7ms)
[00:33:04.027] ⚠️  DROPOUT DETECTED! 'a' repeat stopped for 106ms
[00:33:04.027] 'a' pressed
[00:33:04.290] 'a' repeat #1 (interval: 263.1ms)
[00:33:04.323] 'a' repeat #2 (interval: 32.6ms)
[00:33:04.358] 'a' repeat #3 (interval: 35.3ms)
[00:33:04.393] 'a' repeat #4 (interval: 34.5ms)
```

### (2) Chatter Test
- Type two keys alternately.
- If chatter (multiple registrations) is detected, A warning message will be displayed.
```syslog
[00:32:50.793] 'f' pressed
[00:32:50.900] 'j' pressed
[00:32:50.984] 'f' pressed
[00:32:51.107] 'j' pressed
[00:32:51.127] 'j' pressed
[00:32:51.636] ⚠️  CHATTER DETECTED! 'j' pressed again after 146.5ms
```

### Build

```bash
make
./key-tester
```
## License

[MIT](LICENSE)
