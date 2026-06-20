#include <Geode/modify/GJAccountManager.hpp>
#include "../manager/SyncManager.hpp"

using namespace geode::prelude;

/**
 * GJAccountManager hook - Monitors account operations for
 * change detection and sync status updates
 */
class $modify(SyncGDAccountManager, GJAccountManager) {
    
    void onBackupAccountCompleted(cocos2d::extension::CCHttpClient* client, cocos2d::extension::CCHttpResponse* response) {
        auto manager = SyncManager::get();
        
        if (manager->isDebugLoggingEnabled()) {
            log::debug("[SyncGD] Backup account HTTP response received");
        }
        
        // Call original
        GJAccountManager::onBackupAccountCompleted(client, response);
        
        // Parse response for any useful info
        if (response && response->isSucceed()) {
            std::vector<char>* data = response->getResponseData();
            if (data && !data->empty()) {
                std::string responseStr(data->begin(), data->end());
                
                if (manager->isDebugLoggingEnabled()) {
                    log::debug("[SyncGD] Backup response: {}", responseStr);
                }
            }
        }
    }
    
    void onSyncAccountCompleted(cocos2d::extension::CCHttpClient* client, cocos2d::extension::CCHttpResponse* response) {
        auto manager = SyncManager::get();
        
        if (manager->isDebugLoggingEnabled()) {
            log::debug("[SyncGD] Sync account HTTP response received");
        }
        
        // Call original
        GJAccountManager::onSyncAccountCompleted(client, response);
        
        // After successful sync, we can check for differences
        if (response && response->isSucceed()) {
            std::vector<char>* data = response->getResponseData();
            if (data && !data->empty()) {
                std::string responseStr(data->begin(), data->end());
                
                if (manager->isDebugLoggingEnabled()) {
                    log::debug("[SyncGD] Sync response received, length: {}", data->size());
                }
            }
        }
    }
    
    void onLoginAccountCompleted(cocos2d::extension::CCHttpClient* client, cocos2d::extension::CCHttpResponse* response) {
        auto manager = SyncManager::get();
        
        // Call original first
        GJAccountManager::onLoginAccountCompleted(client, response);
        
        // After login, check if we need to sync
        if (response && response->isSucceed() && manager->isEnabled()) {
            if (manager->isDebugLoggingEnabled()) {
                log::info("[SyncGD] Login completed - will check for cloud changes");
            }
            
            // Small delay then check for changes
            CCDelayTime* delay = CCDelayTime::create(2.0f);
            CCCallFunc* checkChanges = CCCallFunc::create(
                manager,
                callfunc_selector(SyncManager::checkForCloudChanges)
            );
            
            CCDirector::sharedDirector()->getRunningScene()->runAction(
                CCSequence::create(delay, checkChanges, nullptr)
            );
        }
    }
};
