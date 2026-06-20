# SyncGD

<img src="logo.png" width="150" alt="SyncGD Logo" />

**SyncGD** is a Geometry Dash Geode mod that provides automatic cloud synchronization across all your devices, with integrated save retry functionality.

## Features

- **Auto-Save** - Backs up your progress to the cloud every 5 minutes (configurable)
- **Auto-Load** - Downloads cloud saves when starting the game
- **Cloud Change Detection** - Detects new progress from other devices automatically
- **Save Retry** - Replaces `sorkopiko.saveretry` with automatic retry on failed saves
- **Beautiful Notifications** - Custom notifications via `miskaa.notif`
- **Fully Configurable** - Toggle and customize every feature in Geode settings

## Installation

1. Install [Geode](https://geode-sdk.org/) if you haven't already
2. Install `miskaa.notif` from the Geode mod index (required dependency)
3. Download and install SyncGD
4. Configure settings in **Geode > Mods > SyncGD > Settings**

## Multi-Device Sync

SyncGD works seamlessly across all platforms:
- **Windows** - Native support
- **macOS** - Native support
- **Linux** - Works through Wine
- **Android** - Full support
- **iOS** - Full support

Your progress syncs automatically when you:
1. Complete levels on any device
2. Close and reopen the game on another device
3. SyncGD detects changes and downloads your updated save

## Save Retry

The integrated Save Retry feature:
- Automatically retries failed saves
- Shows attempt counter (optional)
- Cancel button to stop retrying (optional)
- Configurable max retry attempts (0 = unlimited)
- Smart handling of oversized saves

## Building from Source

```sh
# Requires Geode SDK and CLI
git clone https://github.com/guerrerooliver183-source/SyncGD.git
cd SyncGD
geode build
```

## Links

- **Source**: https://github.com/guerrerooliver183-source/SyncGD
- **Dependencies**: [miskaa.notif](https://github.com/miskkaaa/notif)
- **Replaces**: [sorkopiko.saveretry](https://github.com/SorkoPiko/SaveRetry)

## License

This project is open source. See the repository for license details.
