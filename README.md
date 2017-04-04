# Keylogger

Keylogger is simple Windows program that demonstrate how to do DLL injection and keylogging of another process. This program also do periodec screen capture upon starting of certain another application/process.

Keylogger that take two parameters(or one if the two params are supposed to be the same). The first parameter is name of process(for e.g 'notepad.exe') where the keylogger process should attach and intercept all the keystrokes directed to that process/window. The second parameter is the name of process(exe filename) to which the keylogger will monitor and start the screencapture as soon as the process is started.

Keylogger will terminate when the two process already activated and then terminated.


To run keylogger, simply use:
```cmd
C:\binary\keylogger notepad.exe chrome.exe
```

By default, the screen will be captured every 3 seconds and saved to `C:\src\YYYYMMDD_HHMMSS.bmp` and the logged key will be saved to `C:\src\keydump.txt`

# Project Internal
Keylogger consists of two parts, the first one is the main project itself(build to .exe), and the second one is the DLL part(build to .Dll). The .Dll will be loaded on the runtime, when the target process is activated and then injected to target process's memory space. The key hook will be then activated, using Win32 API: SetWindowsHookEx().

The project is done using Code::Blocks IDE with GCC/MingW compiler. C++11 support is required here.
