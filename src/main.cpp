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

    static SyncManager* get() {
        static auto instance = new SyncManager();
        return instance;
    }

    void initSync() {
        this->schedule(schedule_selector(SyncManager::onAutoSaveTimer), 300.0f);
        this->schedule(schedule_selector(SyncManager::checkConnectivity), 10.0f);
    }

    void onAutoSaveTimer(float dt) {
        this->performFullSync();
    }

    void checkConnectivity(float dt) {
        if (m_pendingSync) {
            this->performCloudSave();
        }
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
        auto glm = GameLevelManager::sharedState();
        if (glm) {
            m_pendingSync = false;
        } else {
            m_pendingSync = true;
        }
    }
};

class $modify(SRAccountLayer, AccountLayer) {
    struct Fields {
        int m_attempts = 1;
        bool m_cancelled = false;
    };

    void customSetup() {
        AccountLayer::customSetup();
    }

    void backupAccountFailed(const BackupAccountError p0, const int p1) {
        if (static_cast<int>(p0) == -1 && !m_fields->m_cancelled) {
            m_fields->m_attempts++;
            Notification::create(
                fmt::format("Backup failed. Retrying... (Attempt {})", m_fields->m_attempts),
                NotificationIcon::Warning
            )->show();
            this->doBackup();
            return;
        }
        AccountLayer::backupAccountFailed(p0, p1);
    }

    void backupAccountFinished() {
        AccountLayer::backupAccountFinished();
        Notification::create(
            fmt::format("Backup successful! ({} attempts)", m_fields->m_attempts),
            NotificationIcon::Success
        )->show();
        m_fields->m_attempts = 1;
    }

    void syncAccountFailed(const BackupAccountError p0, const int p1) {
        if (static_cast<int>(p0) == -1 && !m_fields->m_cancelled) {
            m_fields->m_attempts++;
            Notification::create(
                fmt::format("Sync failed. Retrying... (Attempt {})", m_fields->m_attempts),
                NotificationIcon::Warning
            )->show();
            this->doSync();
            return;
        }
        AccountLayer::syncAccountFailed(p0, p1);
    }

    void syncAccountFinished() {
        AccountLayer::syncAccountFinished();
        Notification::create(
            fmt::format("Sync successful! ({} attempts)", m_fields->m_attempts),
            NotificationIcon::Success
        )->show();
        m_fields->m_attempts = 1;
    }
};

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        static bool firstLoad = true;
        if (firstLoad) {
            firstLoad = false;
            SyncManager::get()->initSync();
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
