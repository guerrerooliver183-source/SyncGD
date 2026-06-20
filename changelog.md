# SyncGD Changelog

## v1.0.0 (2026-06-20)

### Initial Release
- **Auto-Save**: Automatic cloud backups at configurable intervals
- **Auto-Load**: Automatic download of cloud saves on startup
- **Cloud Change Detection**: Detect new progress from other devices
- **Save Retry Integration**: Built-in save retry functionality (replaces sorkopiko.saveretry)
- **Custom Notifications**: Beautiful notifications via miskaa.notif API
- **Full Settings Customization**: All features can be toggled and configured
- **Multi-Device Support**: Works across Windows, macOS, Linux (Wine), Android, and iOS
- **Incompatibility**: Conflicts with sorkopiko.saveretry (built-in replacement)

### Changes
- Max Retry Attempts defaults to 0 (unlimited)
- Retry attempts UI (counter + cancel button) only shows on manual Save/Load clicks
- Auto-save operations run silently without on-screen retry UI
- Added GitHub Actions multi-platform build workflow

### Technical
- Built for Geode SDK v5.7.0
- Supports Geometry Dash 2.2081 on all platforms
- Uses miskaa.notif for notification display
- Hooks: MenuLayer, AccountLayer, GJAccountManager, GameManager
- CI/CD: `.github/workflows/multi-platform.yml` for automated builds
