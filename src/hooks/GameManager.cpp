#include <Geode/modify/GameManager.hpp>
#include "../manager/SyncManager.hpp"

using namespace geode::prelude;

/**
 * GameManager hook - Detects progress changes that need to be synced
 * Triggers notifications and marks data as needing save
 */
class $modify(SyncGDGameManager, GameManager) {
    
    struct Fields {
        bool m_hasUnsavedProgress = false;
        int m_starsBefore = 0;
        int m_orbsBefore = 0;
        int m_diamondsBefore = 0;
    };
    
    bool init() {
        if (!GameManager::init()) {
            return false;
        }
        
        auto manager = SyncManager::get();
        
        if (manager->isDebugLoggingEnabled()) {
            log::debug("[SyncGD] GameManager initialized");
        }
        
        // Store initial currency values for change detection
        m_fields->m_starsBefore = m_playerUserID;
        
        return true;
    }
    
    // Hook reportPercentageForLevel to detect level completions
    void reportPercentageForLevel(int levelID, int percentage, bool isPractice) {
        GameManager::reportPercentageForLevel(levelID, percentage, isPractice);
        
        auto manager = SyncManager::get();
        if (!manager->isEnabled()) return;
        
        m_fields->m_hasUnsavedProgress = true;
        
        if (manager->isDebugLoggingEnabled()) {
            log::debug("[SyncGD] Level progress detected - Level {}: {}%", levelID, percentage);
        }
        
        // If significant progress (100% completion), notify
        if (percentage >= 100 && manager->shouldShowNotifications()) {
            manager->sendSuccessNotification("Level completed! Progress will be synced.");
        }
    }
    
    // Hook when stars are earned
    void reportAchievementWithID(const char* achievementID) {
        GameManager::reportAchievementWithID(achievementID);
        
        auto manager = SyncManager::get();
        if (!manager->isEnabled()) return;
        
        m_fields->m_hasUnsavedProgress = true;
        
        if (manager->isDebugLoggingEnabled()) {
            log::debug("[SyncGD] Achievement earned: {}", achievementID);
        }
    }
    
    // Hook when a level is rated (stars change)
    void addLevelToSavedGauntlet(int levelID) {
        GameManager::addLevelToSavedGauntlet(levelID);
        
        auto manager = SyncManager::get();
        if (!manager->isEnabled()) return;
        
        m_fields->m_hasUnsavedProgress = true;
        
        if (manager->isDebugLoggingEnabled()) {
            log::debug("[SyncGD] Gauntlet level saved: {}", levelID);
        }
    }
};
