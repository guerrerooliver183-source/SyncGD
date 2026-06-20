#include "SyncManager.hpp"

#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/loader/Mod.hpp>

// Include miskaa.notif API
#include <notif_api.hpp>

using namespace geode::prelude;

SyncManager* SyncManager::get() {
    static SyncManager instance;
    return &instance;
}

void SyncManager::init() {
    if (m_initialized) return;
    
    m_intervalMinutes = Mod::get()->getSettingValue<int64_t>("auto-save-interval");
    if (m_intervalMinutes < 1) m_intervalMinutes = 5;
    
    m_initialized = true;
    
    if (isDebugLoggingEnabled()) {
        log::debug("[SyncGD] SyncManager initialized");
    }
    
    // Auto-save will start when MenuLayer loads (see MenuLayer hook)
}

void SyncManager::startAutoSave() {
    if (!isEnabled()) {
        if (isDebugLoggingEnabled()) {
            log::debug("[SyncGD] Auto-save not started - mod disabled");
        }
        return;
    }
    
    stopAutoSave(); // Clear any existing schedule
    scheduleNextSave();
    
    if (isDebugLoggingEnabled()) {
        log::info("[SyncGD] Auto-save started with {} minute interval", m_intervalMinutes);
    }
}

void SyncManager::stopAutoSave() {
    CCScheduler::get()->unscheduleSelector(
        schedule_selector(SyncManager::onAutoSaveTick),
        this
    );
    
    if (isDebugLoggingEnabled()) {
        log::debug("[SyncGD] Auto-save stopped");
    }
}

void SyncManager::scheduleNextSave() {
    if (!isEnabled()) return;
    
    // Convert minutes to seconds for the scheduler
    float intervalSeconds = static_cast<float>(m_intervalMinutes * 60.0f);
    
    CCScheduler::get()->scheduleSelector(
        schedule_selector(SyncManager::onAutoSaveTick),
        this,
        intervalSeconds,
        kCCRepeatForever,
        0.0f,
        false
    );
}

void SyncManager::onAutoSaveTick() {
    if (!isEnabled() || m_isSaving || m_isSyncing) return;
    
    if (isDebugLoggingEnabled()) {
        log::info("[SyncGD] Auto-save tick triggered");
    }
    
    doBackup();
}

void SyncManager::doBackup() {
    if (m_isSaving) return;
    
    m_isSaving = true;
    m_currentAttempts = 0;
    
    if (shouldShowNotifications()) {
        sendNotification("Syncing save data to cloud...", "info");
    }
    
    if (isDebugLoggingEnabled()) {
        log::info("[SyncGD] Starting cloud backup");
    }
    
    // Trigger the actual backup through GJAccountManager
    auto am = GJAccountManager::sharedState();
    if (am) {
        am->backupAccount("");
    }
}

void SyncManager::doSync() {
    if (m_isSyncing) return;
    
    m_isSyncing = true;
    m_currentAttempts = 0;
    
    if (shouldShowNotifications()) {
        sendNotification("Downloading save data from cloud...", "info");
    }
    
    if (isDebugLoggingEnabled()) {
        log::info("[SyncGD] Starting cloud sync (download)");
    }
    
    // Trigger the actual sync/download through GJAccountManager
    auto am = GJAccountManager::sharedState();
    if (am) {
        am->syncAccount("");
    }
}

void SyncManager::setInterval(int minutes) {
    if (minutes < 1) minutes = 1;
    if (minutes > 60) minutes = 60;
    m_intervalMinutes = minutes;
    
    // Restart with new interval if active
    if (isEnabled()) {
        startAutoSave();
    }
}

int SyncManager::getInterval() const {
    return m_intervalMinutes;
}

void SyncManager::checkForCloudChanges() {
    if (!isChangeDetectionEnabled() || !isEnabled()) return;
    
    if (isDebugLoggingEnabled()) {
        log::debug("[SyncGD] Checking for cloud changes...");
    }
    
    // Compare current local save with known state
    // If changes detected on server, prompt user or auto-sync
    auto am = GJAccountManager::sharedState();
    if (!am) return;
    
    // Request account info to check sync status
    // The response will be handled by our hooks
    
    // For now, trigger a sync if auto-load is enabled
    if (isAutoLoadEnabled()) {
        if (shouldShowNotifications()) {
            sendNotification("Checking for cloud updates...", "info");
        }
        doSync();
    }
}

bool SyncManager::hasDetectedChanges() const {
    return m_changesDetected;
}

// Notification helpers
void SyncManager::sendNotification(const std::string& message, const std::string& type) {
    if (!shouldShowNotifications()) return;
    
    try {
        notifapi::notify(message, type);
    } catch (...) {
        // Fallback to Geode's built-in notification
        Notification::create(message, NotificationIcon::Info)->show();
    }
}

void SyncManager::sendSuccessNotification(const std::string& message) {
    if (!shouldShowNotifications()) return;
    
    try {
        notifapi::success(message);
    } catch (...) {
        Notification::create(message, NotificationIcon::Success)->show();
    }
}

void SyncManager::sendErrorNotification(const std::string& message) {
    if (!shouldShowNotifications()) return;
    
    try {
        notifapi::error(message);
    } catch (...) {
        Notification::create(message, NotificationIcon::Error)->show();
    }
}

void SyncManager::sendWarningNotification(const std::string& message) {
    if (!shouldShowNotifications()) return;
    
    try {
        notifapi::warn(message);
    } catch (...) {
        Notification::create(message, NotificationIcon::Warning)->show();
    }
}

// Save retry settings
bool SyncManager::isSaveRetryEnabled() const {
    return Mod::get()->getSettingValue<bool>("save-retry-enabled");
}

bool SyncManager::shouldShowRetryAttempts() const {
    return Mod::get()->getSettingValue<bool>("show-retry-attempts");
}

bool SyncManager::shouldShowCancelButton() const {
    return Mod::get()->getSettingValue<bool>("cancel-retry-button");
}

int SyncManager::getMaxRetryAttempts() const {
    return static_cast<int>(Mod::get()->getSettingValue<int64_t>("max-retry-attempts"));
}

// General settings
bool SyncManager::isEnabled() const {
    return Mod::get()->getSettingValue<bool>("enabled");
}

bool SyncManager::isAutoLoadEnabled() const {
    return Mod::get()->getSettingValue<bool>("auto-load-on-startup");
}

bool SyncManager::isChangeDetectionEnabled() const {
    return Mod::get()->getSettingValue<bool>("detect-changes");
}

bool SyncManager::shouldShowNotifications() const {
    return Mod::get()->getSettingValue<bool>("show-notifications");
}

bool SyncManager::isDebugLoggingEnabled() const {
    return Mod::get()->getSettingValue<bool>("debug-logging");
}
