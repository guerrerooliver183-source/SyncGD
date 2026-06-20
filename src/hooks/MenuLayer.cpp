#include <Geode/modify/MenuLayer.hpp>
#include "../manager/SyncManager.hpp"

using namespace geode::prelude;

/**
 * MenuLayer hook - Initializes auto-sync when the main menu loads
 * This ensures the auto-save timer starts when the player reaches the menu
 */
class $modify(SyncGDMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }
        
        auto manager = SyncManager::get();
        
        if (manager->isDebugLoggingEnabled()) {
            log::debug("[SyncGD] MenuLayer initialized - setting up auto-sync");
        }
        
        // Start auto-save timer
        manager->startAutoSave();
        
        // Check for cloud changes on startup (with small delay to ensure everything is loaded)
        if (manager->isAutoLoadEnabled() && manager->isEnabled()) {
            CCDelayTime* delay = CCDelayTime::create(3.0f);
            CCCallFunc* checkChanges = CCCallFunc::create(
                manager,
                callfunc_selector(SyncManager::checkForCloudChanges)
            );
            
            this->runAction(CCSequence::create(delay, checkChanges, nullptr));
        }
        
        return true;
    }
};
