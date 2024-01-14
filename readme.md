# Chrome Audio Delay Patch

This software patches Chrome on Windows to fix audio delay over wireless (primarily, Bluetooth) headphones.

This introduces an artificial delay into video playback, so that audio and video are in sync.

The patch injects a DLL into Chrome's audio subsystems, and the DLL modifies values returned from `IAudioClock::GetPosition`.

This is only tested on Windows 10 and 11, x64.

# How to Use

First of all, connect the bluetooth device in question and measure the audio delay. You can do this by playing some "av sync test" video and recording your screen and audio from headphones on a smartphone (ideally, in slow mo). That should show by how much audio is delayed.

Download binaries, unpack somewhere.

Run `Configurator.exe`, add the audio device (by selecting it and pressing "->"), set the delay.
Press the "Run now" button, you can also set it to start with Windows.

You can select browsers to run this for (use "Select Browsers" button). This has been tested on Chrome, Edge, and Yandex.Browser, but should theoretically work for any Chromium-based browser.

Edge, Yandex, and Chrome are auto-detected, any other browser can be added manually.

**Restart the browser**, or at least its audio subsystem (by pressing Shift-Esc in Chrome and killing audio subsystem process).

Enjoy!

Demo: https://youtu.be/QfFo403fcgc
