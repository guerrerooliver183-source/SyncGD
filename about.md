# SyncGD

**SyncGD** is a comprehensive cloud synchronization mod for Geometry Dash that keeps your save data in sync across all your devices.

## Features

- **Auto-Save**: Automatically backs up your save data to RobTop's cloud servers at configurable intervals (default: every 5 minutes)
- **Auto-Load**: Detects and downloads new progress when you open the game on any device
- **Cloud Change Detection**: Smart detection of account changes across devices for seamless multi-device gaming
- **Integrated Save Retry**: Replaces `sorkopiko.saveretry` with configurable retry logic that continuously attempts to upload your save until it succeeds
- **Custom Notifications**: Beautiful notification system powered by `miskaa.notif` to keep you informed about sync operations
- **Fully Configurable**: All features can be toggled and customized in the Geode mod settings

## Multi-Device Sync

Play on your phone, tablet, or PC - SyncGD ensures your progress is always up to date:
1. Play and complete levels on any device
2. Your progress is automatically saved to the cloud
3. When you open GD on another device, SyncGD detects the new progress
4. Your save is automatically downloaded and synced

## Save Retry

No more spamming the save button! When a save fails:
- The mod automatically retries the upload
- Configurable maximum retry attempts (or unlimited)
- Visual attempt counter in the Account menu
- Cancel button to stop retrying
- Smart detection of oversized saves

## Settings

All settings are available in **Geode > Mods > SyncGD > Settings**:

| Setting | Description | Default |
|---------|-------------|---------|
| Enabled | Master toggle for the mod | ON |
| Auto-Save Interval | Minutes between auto-saves | 5 |
| Auto-Load on Startup | Download cloud saves on game start | ON |
| Detect Cloud Changes | Detect progress from other devices | ON |
| Show Notifications | Display sync notifications | ON |
| Save Retry | Enable automatic save retrying | ON |
| Max Retry Attempts | Maximum retries (0 = unlimited) | 10 |
| Show Retry Attempts | Display attempt counter | ON |
| Show Cancel Button | Add cancel button during retries | ON |
| Debug Logging | Verbose console logging | OFF |
