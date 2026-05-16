#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/AppDelegate.hpp>
#include <Geode/modify/AccountLayer.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

class SyncManager : public CCNode {
public:
    bool m_pendingSync = false;
    float m_lastInterval = 0.0f;

    static SyncManager* get() {
        static auto instance = [] {
            auto ret = new SyncManager();
            ret->init();
            ret->retain(); // Keep it alive
            return ret;
        }();
        return instance;
    }

    bool init() override {
        if (!CCNode::init()) return false;
        this->updateSchedule();
        return true;
    }

    void updateSchedule() {
        this->unschedule(schedule_selector(SyncManager::onAutoSaveTimer));
        
        // Get interval from settings
        int64_t minutes = Mod::get()->getSettingValue<int64_t>("save-interval");
        if (minutes < 1) minutes = 5;
        
        m_lastInterval = static_cast<float>(minutes) * 60.0f;
        this->schedule(schedule_selector(SyncManager::onAutoSaveTimer), m_lastInterval);
        log::info("SyncGD: Auto-save scheduled every {} seconds", m_lastInterval);
    }

    void onAutoSaveTimer(float dt) {
        this->performFullSync();
    }

    void performFullSync() {
        if (this->saveLocally()) {
            this->performCloudSave();
        }
    }

    bool saveLocally() {
        auto gameManager = GameManager::get();
        if (!gameManager) return false;
        gameManager->save();
        return true;
    }

    void performCloudSave() {
        m_pendingSync = true;
        // The actual cloud save is handled by triggering a backup/sync request
        // In a real mod, we might call GLM methods here
    }
};

class $modify(SRAccountLayer, AccountLayer) {
    struct Fields {
        int m_attempts = 0;
    };

    void customSetup() {
        AccountLayer::customSetup();
    }

    void backupAccountFailed(const BackupAccountError p0, const int p1) {
        if (static_cast<int>(p0) == -1 && m_fields->m_attempts < 5) {
            m_fields->m_attempts++;
            Notification::create(
                fmt::format("Backup failed. Retrying... (Attempt {})", m_fields->m_attempts),
                NotificationIcon::Warning
            )->show();
            
            // Delay retry slightly to be nicer to servers
            this->runAction(CCSequence::create(
                CCDelayTime::create(2.0f),
                CCCallFunc::create(this, callfunc_selector(SRAccountLayer::doBackup)),
                nullptr
            ));
            return;
        }
        
        m_fields->m_attempts = 0;
        AccountLayer::backupAccountFailed(p0, p1);
    }

    void backupAccountFinished() {
        AccountLayer::backupAccountFinished();
        Notification::create(
            fmt::format("Backup successful! ({} attempts)", m_fields->m_attempts + 1),
            NotificationIcon::Success
        )->show();
        m_fields->m_attempts = 0;
        SyncManager::get()->m_pendingSync = false;
    }

    void syncAccountFailed(const BackupAccountError p0, const int p1) {
        if (static_cast<int>(p0) == -1 && m_fields->m_attempts < 5) {
            m_fields->m_attempts++;
            Notification::create(
                fmt::format("Sync failed. Retrying... (Attempt {})", m_fields->m_attempts),
                NotificationIcon::Warning
            )->show();
            
            this->runAction(CCSequence::create(
                CCDelayTime::create(2.0f),
                CCCallFunc::create(this, callfunc_selector(SRAccountLayer::doSync)),
                nullptr
            ));
            return;
        }
        
        m_fields->m_attempts = 0;
        AccountLayer::syncAccountFailed(p0, p1);
    }

    void syncAccountFinished() {
        AccountLayer::syncAccountFinished();
        Notification::create(
            fmt::format("Sync successful! ({} attempts)", m_fields->m_attempts + 1),
            NotificationIcon::Success
        )->show();
        m_fields->m_attempts = 0;
    }
};

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        
        static bool firstLoad = true;
        if (firstLoad) {
            firstLoad = false;
            // Add manager to a persistent node to ensure scheduler runs
            auto manager = SyncManager::get();
            if (!manager->getParent()) {
                CCDirector::sharedDirector()->getNotificationNode()->addChild(manager);
            }
        }
        
        return true;
    }
};

class $modify(AppDelegate) {
    void applicationDidEnterBackground() {
        AppDelegate::applicationDidEnterBackground();
        SyncManager::get()->performFullSync();
    }
};
