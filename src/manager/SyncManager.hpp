#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

/**
 * SyncManager - Core manager for cloud synchronization
 * Handles auto-save, auto-load, and change detection across devices
 */
class SyncManager {
public:
    static SyncManager* get();
    
    void init();
    
    // Auto-save control
    void startAutoSave();
    void stopAutoSave();
    void doBackup();
    void doSync();
    
    // Interval management
    void setInterval(int minutes);
    int getInterval() const;
    
    // Cloud change detection
    void checkForCloudChanges();
    bool hasDetectedChanges() const;
    
    // Notification helpers
    void sendNotification(const std::string& message, const std::string& type = "info");
    void sendSuccessNotification(const std::string& message);
    void sendErrorNotification(const std::string& message);
    void sendWarningNotification(const std::string& message);
    
    // Save retry helpers
    bool isSaveRetryEnabled() const;
    bool shouldShowRetryAttempts() const;
    bool shouldShowCancelButton() const;
    int getMaxRetryAttempts() const;
    
    // Settings helpers
    bool isEnabled() const;
    bool isAutoLoadEnabled() const;
    bool isChangeDetectionEnabled() const;
    bool shouldShowNotifications() const;
    bool isDebugLoggingEnabled() const;

private:
    SyncManager() = default;
    ~SyncManager() = default;
    
    SyncManager(const SyncManager&) = delete;
    SyncManager& operator=(const SyncManager&) = delete;
    
    void scheduleNextSave();
    void onAutoSaveTick();
    
    bool m_initialized = false;
    bool m_isSaving = false;
    bool m_isSyncing = false;
    int m_intervalMinutes = 5;
    int m_currentAttempts = 0;
    bool m_changesDetected = false;
    
    // Store last known save state for change detection
    std::string m_lastKnownHash;
};
