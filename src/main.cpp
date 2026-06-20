/**
 * SyncGD - Auto Cloud Sync for Geometry Dash
 * Automatically syncs your save data across all devices
 * with configurable intervals and integrated save retry.
 */

#include <Geode/Geode.hpp>
#include "manager/SyncManager.hpp"

using namespace geode::prelude;

$execute {
    // Log mod initialization
    log::info("SyncGD v1.0.0 loaded successfully!");
    log::info("Auto-Save Interval: {} minutes", Mod::get()->getSettingValue<int64_t>("auto-save-interval"));
    log::info("Save Retry: {}", Mod::get()->getSettingValue<bool>("save-retry-enabled") ? "Enabled" : "Disabled");

    // Initialize the sync manager
    SyncManager::get()->init();
}

// Listen for setting changes
$execute {
    listenForSettingChanges<bool>("enabled", [](bool value) {
        auto manager = SyncManager::get();
        if (value) {
            manager->startAutoSave();
            log::info("SyncGD: Auto-sync enabled");
        } else {
            manager->stopAutoSave();
            log::info("SyncGD: Auto-sync disabled");
        }
    });

    listenForSettingChanges<int64_t>("auto-save-interval", [](int64_t value) {
        auto manager = SyncManager::get();
        manager->setInterval(static_cast<int>(value));
        log::info("SyncGD: Auto-save interval changed to {} minutes", value);
    });

    listenForSettingChanges<bool>("save-retry-enabled", [](bool value) {
        log::info("SyncGD: Save Retry {}", value ? "enabled" : "disabled");
    });

    listenForSettingChanges<bool>("auto-load-on-startup", [](bool value) {
        log::info("SyncGD: Auto-load on startup {}", value ? "enabled" : "disabled");
    });

    listenForSettingChanges<bool>("detect-changes", [](bool value) {
        log::info("SyncGD: Cloud change detection {}", value ? "enabled" : "disabled");
    });
}
