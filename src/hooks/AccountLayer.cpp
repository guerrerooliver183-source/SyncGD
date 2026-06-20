#include <Geode/modify/AccountLayer.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/binding/TextArea.hpp>
#include "../manager/SyncManager.hpp"

using namespace geode::prelude;

/**
 * AccountLayer hook - Integrates SaveRetry functionality
 * and handles backup/sync completion and failure events
 */
class $modify(SyncGDAccountLayer, AccountLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* m_cancelButton = nullptr;
        CCMenu* m_cancelMenu = nullptr;
        CCLabelBMFont* m_backupLabel = nullptr;
        int m_attempts = 1;
        bool m_cancelled = false;
        bool m_isRetrying = false;
    };

    void customSetup() {
        AccountLayer::customSetup();
        
        auto manager = SyncManager::get();
        
        // Only show retry UI if Save Retry is enabled
        if (!manager->isSaveRetryEnabled()) return;
        
        if (!manager->shouldShowRetryAttempts() && !manager->shouldShowCancelButton()) return;
        
        m_fields->m_cancelMenu = CCMenu::create();
        m_fields->m_cancelMenu->setPosition({30.f, 295.f});
        
        m_fields->m_backupLabel = CCLabelBMFont::create(
            fmt::format("Attempt {}", m_fields->m_attempts).c_str(),
            "goldFont.fnt"
        );
        
        m_fields->m_backupLabel->setScale(0.8f);
        m_fields->m_backupLabel->setPosition({
            m_linkedAccountTitle->getPositionX(),
            m_linkedAccountTitle->getPositionY() + 30.f
        });
        m_fields->m_backupLabel->setAnchorPoint({0.5f, 0.5f});
        
        // Cancel button
        if (manager->shouldShowCancelButton()) {
            const auto sprite = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
            m_fields->m_cancelButton = CCMenuItemSpriteExtra::create(
                sprite,
                this,
                menu_selector(SyncGDAccountLayer::cancelBackup)
            );
            m_fields->m_cancelButton->setID("syncgd-cancel-button"_spr);
            
            m_fields->m_cancelMenu->addChild(m_fields->m_cancelButton);
        }
        
        customHideLoadingUI();
        m_mainLayer->addChild(m_fields->m_cancelMenu, 106);
        m_mainLayer->addChild(m_fields->m_backupLabel, 106);
    }
    
    void cancelBackup(CCObject* sender) {
        auto manager = SyncManager::get();
        
        manager->sendWarningNotification("Backup Cancelled - Finishing Last Attempt");
        
        if (m_fields->m_cancelButton) {
            m_fields->m_cancelButton->setVisible(false);
        }
        m_fields->m_cancelMenu->setEnabled(false);
        m_fields->m_cancelled = true;
    }
    
    void customShowLoadingUI() {
        auto manager = SyncManager::get();
        if (!manager->isSaveRetryEnabled()) return;
        
        if (manager->shouldShowCancelButton() && m_fields->m_cancelMenu) {
            m_fields->m_cancelMenu->setEnabled(true);
        }
        
        if (manager->shouldShowRetryAttempts() && m_fields->m_backupLabel) {
            m_fields->m_backupLabel->setString(fmt::format("Attempt {}", m_fields->m_attempts).c_str());
            m_fields->m_backupLabel->setVisible(true);
        }
        
        if (m_fields->m_cancelButton) {
            m_fields->m_cancelButton->setVisible(true);
        }
    }
    
    void customHideLoadingUI() {
        m_fields->m_attempts = 1;
        m_fields->m_cancelled = false;
        m_fields->m_isRetrying = false;
        
        if (m_fields->m_cancelMenu) {
            m_fields->m_cancelMenu->setEnabled(false);
        }
        if (m_fields->m_backupLabel) {
            m_fields->m_backupLabel->setVisible(false);
        }
        if (m_fields->m_cancelButton) {
            m_fields->m_cancelButton->setVisible(false);
        }
    }
    
    void incrementAttempt() {
        m_fields->m_attempts++;
        if (m_fields->m_backupLabel) {
            m_fields->m_backupLabel->setString(fmt::format("Attempt {}", m_fields->m_attempts).c_str());
        }
    }
    
    void showAttempts() {
        auto manager = SyncManager::get();
        if (!manager->shouldShowRetryAttempts()) return;
        
        std::string label = fmt::format("Backup successful\n({} attempts)", m_fields->m_attempts);
        if (m_fields->m_attempts == 1) {
            label = "Backup successful\n(1 attempt)";
        }
        m_textArea->setString(label);
        m_textArea->colorAllCharactersTo({0, 255, 0});
    }
    
    void showSyncAttempts() {
        auto manager = SyncManager::get();
        if (!manager->shouldShowRetryAttempts()) return;
        
        std::string label = fmt::format("Sync successful\n({} attempts)", m_fields->m_attempts);
        if (m_fields->m_attempts == 1) {
            label = "Sync successful\n(1 attempt)";
        }
        m_textArea->setString(label);
        m_textArea->colorAllCharactersTo({0, 255, 0});
    }
    
    // ===== BACKUP HANDLING =====
    
    void backupAccountFailed(const BackupAccountError p0, const int p1) {
        auto manager = SyncManager::get();
        
        if (manager->isDebugLoggingEnabled()) {
            log::info("[SyncGD] Backup failed: {} {}", static_cast<int>(p0), p1);
        }
        
        // SaveRetry logic
        if (manager->isSaveRetryEnabled() && 
            static_cast<int>(p0) == -1 && 
            !m_fields->m_cancelled) {
            
            int maxAttempts = manager->getMaxRetryAttempts();
            if (maxAttempts > 0 && m_fields->m_attempts >= maxAttempts) {
                // Max attempts reached
                customHideLoadingUI();
                manager->sendErrorNotification(fmt::format("Backup failed after {} attempts", maxAttempts));
                AccountLayer::backupAccountFailed(p0, p1);
                return;
            }
            
            incrementAttempt();
            
            const auto gjam = GJAccountManager::sharedState();
            const float profile = gjam->m_gameManagerSize * 0.00000095367;
            const float levels = gjam->m_localLevelsSize * 0.00000095367;
            
            if (profile + levels > 32.0) {
                customHideLoadingUI();
                m_fields->m_attempts = 1;
                manager->sendErrorNotification("Save data too large to upload!");
                AccountLayer::backupAccountFailed(p0, p1);
                return;
            }
            
            manager->sendWarningNotification(fmt::format("Backup failed, retrying... (Attempt {})", m_fields->m_attempts));
            this->doBackup();
            return;
        }
        
        customHideLoadingUI();
        manager->sendErrorNotification("Backup failed!");
        AccountLayer::backupAccountFailed(p0, p1);
    }
    
    void backupAccountFinished() {
        auto manager = SyncManager::get();
        
        if (manager->isDebugLoggingEnabled()) {
            log::debug("[SyncGD] Backup completed successfully");
        }
        
        AccountLayer::backupAccountFinished();
        
        if (manager->isSaveRetryEnabled()) {
            showAttempts();
            customHideLoadingUI();
        }
        
        manager->sendSuccessNotification("Save data backed up to cloud!");
    }
    
    // ===== SYNC HANDLING =====
    
    void syncAccountFailed(const BackupAccountError p0, const int p1) {
        auto manager = SyncManager::get();
        
        if (manager->isDebugLoggingEnabled()) {
            log::info("[SyncGD] Sync failed: {} {}", static_cast<int>(p0), p1);
        }
        
        // SaveRetry logic for sync
        if (manager->isSaveRetryEnabled() && 
            static_cast<int>(p0) == -1 && 
            !m_fields->m_cancelled) {
            
            int maxAttempts = manager->getMaxRetryAttempts();
            if (maxAttempts > 0 && m_fields->m_attempts >= maxAttempts) {
                customHideLoadingUI();
                manager->sendErrorNotification(fmt::format("Sync failed after {} attempts", maxAttempts));
                AccountLayer::syncAccountFailed(p0, p1);
                return;
            }
            
            incrementAttempt();
            manager->sendWarningNotification(fmt::format("Sync failed, retrying... (Attempt {})", m_fields->m_attempts));
            this->doSync();
            return;
        }
        
        customHideLoadingUI();
        manager->sendErrorNotification("Sync failed!");
        AccountLayer::syncAccountFailed(p0, p1);
    }
    
    void syncAccountFinished() {
        auto manager = SyncManager::get();
        
        if (manager->isDebugLoggingEnabled()) {
            log::debug("[SyncGD] Sync completed successfully");
        }
        
        AccountLayer::syncAccountFinished();
        
        if (manager->isSaveRetryEnabled()) {
            showSyncAttempts();
            customHideLoadingUI();
        }
        
        manager->sendSuccessNotification("Save data synced from cloud!");
    }
    
    // ===== DIALOG HANDLING =====
    
    void FLAlert_Clicked(FLAlertLayer *p0, bool p1) {
        AccountLayer::FLAlert_Clicked(p0, p1);
        
        auto manager = SyncManager::get();
        
        if (p1 == true) {
            if (manager->isSaveRetryEnabled()) {
                customShowLoadingUI();
            }
            
            // Show notification that operation started
            if (manager->shouldShowNotifications()) {
                manager->sendNotification("Cloud operation started...", "info");
            }
        }
    }
};
