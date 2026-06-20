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

### Technical
- Built for Geode SDK v5.7.0
- Supports Geometry Dash 2.2081 on all platforms
- Uses miskaa.notif for notification display
- Hooks: MenuLayer, AccountLayer, GJAccountManager, GameManager
