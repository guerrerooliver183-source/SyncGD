#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/AppDelegate.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

class SyncManager : public CCNode {
public:
    bool m_isSyncing = false;
    float m_lastInterval = 0.0f;

    static SyncManager* get() {
        static auto instance = [] {
            auto ret = new SyncManager();
            ret->init();
            ret->retain();
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
        int64_t minutes = Mod::get()->getSettingValue<int64_t>("save-interval");
        if (minutes < 1) minutes = 5;
        m_lastInterval = static_cast<float>(minutes) * 60.0f;
        this->schedule(schedule_selector(SyncManager::onAutoSaveTimer), m_lastInterval);
    }

    void onAutoSaveTimer(float dt) {
        this->performFullSyncCycle();
    }

    void performFullSyncCycle() {
        if (m_isSyncing) return;
        
        Notification::create("SyncGD: Saving locally...", NotificationIcon::Info)->show();
        if (this->saveToDataDat()) {
            this->uploadToAccount();
        }
    }

    bool saveToDataDat() {
        auto gameManager = GameManager::get();
        if (!gameManager) return false;
        gameManager->save();

        auto savePath = geode::utils::dirs::getGameDir();
        auto modSavePath = Mod::get()->getSaveDir();
        
        auto managerSrc = savePath / "CCGameManager.dat";
        auto dest = modSavePath / "data.dat";
        
        if (std::filesystem::exists(managerSrc)) {
            std::error_code ec;
            std::filesystem::copy_file(managerSrc, dest, std::filesystem::copy_options::overwrite_existing, ec);
            return !ec;
        }
        return false;
    }

    void uploadToAccount() {
        auto am = GJAccountManager::sharedState();
        if (am && am->m_accountID > 0) {
            m_isSyncing = true;
            auto al = AccountLayer::create();
            if (al) {
                al->doBackup();
            }
        } else {
            Notification::create("SyncGD: Not logged in!", NotificationIcon::Error)->show();
        }
    }
};

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        
        static bool firstLoad = true;
        if (firstLoad) {
            firstLoad = false;
            auto manager = SyncManager::get();
            if (!manager->getParent()) {
                CCDirector::sharedDirector()->getNotificationNode()->addChild(manager);
            }
            
            auto am = GJAccountManager::sharedState();
            if (am && am->m_accountID > 0) {
                auto al = AccountLayer::create();
                if (al) {
                    al->doSync();
                }
            }
        }
        
        return true;
    }
};

class $modify(AppDelegate) {
    void applicationDidEnterBackground() {
        AppDelegate::applicationDidEnterBackground();
        SyncManager::get()->performFullSyncCycle();
    }
};
