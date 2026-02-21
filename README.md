# SpotifyEnhancer

> Automatically skip Spotify ads – never wait for the next track again.

SpotifyEnhancer runs silently in the background and monitors the Spotify window title.
As soon as an advertisement is detected, it closes Spotify and immediately restarts it,
resuming your music without interaction.

---

## Features

- **Automatic ad skipping** – detects ads via window-title monitoring and restarts Spotify instantly
- **Ad counter** – tracks the total number of skipped advertisements across sessions
- **System tray integration** – lives in the notification area; accessible at any time
- **Autostart with Windows** – optional registry entry so SpotifyEnhancer starts with your PC
- **Configurable check rate** – choose how aggressively the background thread polls Spotify
- **Transition delay** – optional pause before the restart to avoid rapid restart loops
- **Dark / Light theme** – follows the Windows color scheme automatically, or set it manually
- **Colored background** – optional accent-color background
- **Update checker** – built-in online updater via the Qt Installer Framework maintenance tool
- **Single-instance guard** – if already running, the existing window is brought to the foreground

---

## Download

| Installer | Platform |
|-----------|----------|
| [SpotifyEnhancer Online Installer](https://code.obermui.de/Markus/SpotifyEnhancer/releases) | Windows x64 |

The online installer fetches the latest package components at install time and installs a
`SpotifyEnhancerMaintenanceTool.exe` for future updates.

---

## Requirements

| Requirement | Notes |
|-------------|-------|
| **Windows 10 / 11** | Win32 API (`EnumWindows`, `Shell_NotifyIcon`, …) |
| **Spotify desktop app** | Must be installed via a real installer (not the Microsoft Store version) |
| **Qt 6 runtime** | Bundled with the installer |

> **Important:** Spotify must be installed through its official installer into a real folder
> (e.g. `%AppData%\Spotify`). The Microsoft Store variant uses a different process model and
> is not supported.

---

## Build from Source

### Prerequisites

- Qt 6.6 or later (MinGW 64-bit toolchain)
- Qt Installer Framework (optional, for building the installer)

### Steps

```bash
# Clone including submodules
git clone --recurse-submodules https://code.obermui.de/markus/SpotifyEnhancer.git
cd SpotifyEnhancer

# Build the MUpdaterLib dependency first
cd libs/MUpdaterLib
qmake MUpdater.pro CONFIG+=release
mingw32-make -j$(nproc)
cd ../..

# Build SpotifyEnhancer
qmake SpotifyEnhancer.pro CONFIG+=release
mingw32-make -j$(nproc)
```

The resulting binary is placed in the `release/` folder.

---

## How It Works

1. A `QThread`-based background loop (`SpotifyManager`) polls the Spotify window every
   ~800 ms (configurable).
2. The window title is inspected: Spotify displays `Artist – Track` during music and a
   plain title (e.g. `Spotify Free`) during ads.
3. When an ad title is detected, the loop:
   - Moves the mouse focus back after a configurable transition delay
   - Sends `WM_CLOSE` to the Spotify window and waits up to 5 seconds for it to exit
     (falls back to `TerminateProcess` if needed)
   - Launches Spotify again via `CreateProcess`
   - Waits for the new window, then sends a `VK_MEDIA_PLAY_PAUSE` keystroke to resume
4. A rate-limiter prevents the loop from restarting more than 4 times per minute to
   guard against runaway restart cycles.

---

## Known Limitations

- **Fullscreen applications** – after a restart, Windows may not automatically return
  focus to your fullscreen app; you may need to Alt+Tab once.
- **Rare start failures** – on some systems `CreateProcess` can fail to bring up the
  Spotify window in time; increasing the *delay time* setting usually helps.
- **Microsoft Store Spotify** – not supported (no accessible `.exe` path).

---

## Project Structure

```
SpotifyEnhancer/
├── src/
│   ├── main.cpp              # Entry point
│   ├── mainwindow.{cpp,h,ui} # Main UI, system tray, settings
│   ├── spotifymanager.{cpp,h}# Background ad-detection thread
│   └── menue.{cpp,h}         # Custom rounded context menu widget
├── libs/
│   └── MUpdaterLib/          # Submodule: update dialog, theme, switch widget
├── installer/                # Qt IFW installer configuration & packages
├── icons/                    # Application icons
└── SpotifyEnhancer.pro       # qmake project file
```

---

## Settings Migration

Settings are stored in the Windows registry under
`HKCU\Software\M4RKUS\SpotifyEnhancer`.  
On first launch after an upgrade from versions prior to 1.10.2, existing settings
(previously stored under `HKCU\Software\SpotifyEnhancer\SpotifyEnhancer`) are
automatically migrated and the old keys are removed.

---

## License

See [LICENSE.txt](LICENSE.txt).

---

## Repository

<https://code.obermui.de/markus/SpotifyEnhancer>