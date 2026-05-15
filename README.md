# SyncGD
Automatically saves your progress on mobile and performs backups on PC when changes are detected.

<img src="logo.png" width="150" alt="SyncGD logo" />

## Features
- **Mobile Auto-Save**: Automatically saves your game data every 5 minutes when playing on mobile devices.
- **PC Auto-Backup**: Automatically checks for new changes and performs a backup when you start the game on PC after playing on mobile.
- **Background Protection**: Saves your data immediately when the app enters the background to prevent progress loss.

## How it works
The mod uses the Geode SDK to hook into the game's lifecycle. It detects the platform (Mobile vs PC) and applies the corresponding logic to ensure your Geometry Dash data is always safe and synchronized.

## Build instructions
```sh
# Assuming you have the Geode CLI set up already
geode build
```

## Resources
* [Geode SDK Documentation](https://docs.geode-sdk.org/)
* [Geode SDK Source Code](https://github.com/geode-sdk/geode/)
