#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/AppDelegate.hpp>
#include <Geode/modify/AccountLayer.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/ui/Notification.hpp>
#include <ghc/filesystem.hpp>

using namespace geode::prelude;
namespace fs = ghc::filesystem;

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

        try {
            auto savePath = geode::utils::dirs::getGameDir();
            auto modSavePath = Mod::get()->getSaveDir();
            
            // Consolidate game data into data.dat
            auto managerSrc = savePath / "CCGameManager.dat";
            auto levelsSrc = savePath / "CCLocalLevels.dat";
            auto dest = modSavePath / "data.dat";
            
            if (fs::exists(managerSrc)) {
                fs::copy_file(managerSrc, dest, fs::copy_options::overwrite_existing);
                return true;
            }
            return false;
        } catch (const std::exception& e) {
            log::error("SyncGD: Local save failed: {}", e.what());
            return false;
        }
    }

    void uploadToAccount() {
        auto am = GJAccountManager::sharedState();
        if (am && am->m_accountID > 0) {
            m_isSyncing = true;
            // Create a temporary AccountLayer to use its backup logic
            auto al = AccountLayer::create();
            if (al) {
                al->doBackup();
            }
        } else {
            Notification::create("SyncGD: Not logged in!", NotificationIcon::Error)->show();
        }
    }
};

class $modify(SRAccountLayer, AccountLayer) {
    struct Fields {
        int m_attempts = 0;
        CCLayerColor* m_overlay = nullptr;
        CCLabelBMFont* m_statusLabel = nullptr;
    };

    void customSetup() {
        AccountLayer::customSetup();
        
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        m_fields->m_overlay = CCLayerColor::create({0, 0, 0, 180});
        m_fields->m_overlay->setID("sync-overlay");
        this->addChild(m_fields->m_overlay, 100);

        m_fields->m_statusLabel = CCLabelBMFont::create("Initiating Cloud Sync...", "goldFont.fnt");
        m_fields->m_statusLabel->setPosition(winSize / 2);
        m_fields->m_statusLabel->setScale(0.7f);
        m_fields->m_overlay->addChild(m_fields->m_statusLabel);
        
        m_fields->m_overlay->setVisible(false);
    }

    void showOverlay(const std::string& text) {
        if (m_fields->m_overlay && m_fields->m_statusLabel) {
            m_fields->m_statusLabel->setString(text.c_str());
            m_fields->m_overlay->setVisible(true);
        }
    }

    void backupAccountFailed(const BackupAccountError p0, const int p1) {
        if (static_cast<int>(p0) == -1 && m_fields->m_attempts < 5) {
            m_fields->m_attempts++;
            this->showOverlay(fmt::format("Cloud Save Failed.\nRetrying... Attempt {}", m_fields->m_attempts));
            
            this->runAction(CCSequence::create(
                CCDelayTime::create(2.5f),
                CCCallFunc::create(this, callfunc_selector(SRAccountLayer::doBackup)),
                nullptr
            ));
            return;
        }
        SyncManager::get()->m_isSyncing = false;
        AccountLayer::backupAccountFailed(p0, p1);
    }

    void backupAccountFinished() {
        AccountLayer::backupAccountFinished();
        Notification::create("Cloud Sync Complete!", NotificationIcon::Success)->show();
        if (m_fields->m_overlay) m_fields->m_overlay->setVisible(false);
        SyncManager::get()->m_isSyncing = false;
    }

    void syncAccountFailed(const BackupAccountError p0, const int p1) {
        if (static_cast<int>(p0) == -1 && m_fields->m_attempts < 5) {
            m_fields->m_attempts++;
            this->showOverlay(fmt::format("Cloud Load Failed.\nRetrying... Attempt {}", m_fields->m_attempts));
            
            this->runAction(CCSequence::create(
                CCDelayTime::create(2.5f),
                CCCallFunc::create(this, callfunc_selector(SRAccountLayer::doSync)),
                nullptr
            ));
            return;
        }
        AccountLayer::syncAccountFailed(p0, p1);
    }

    void syncAccountFinished() {
        AccountLayer::syncAccountFinished();
        Notification::create("Data Downloaded!", NotificationIcon::Success)->show();
        if (m_fields->m_overlay) m_fields->m_overlay->setVisible(false);
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
            
            // Auto-Sync on startup if logged in
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
