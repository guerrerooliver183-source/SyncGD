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
    bool m_pendingSync = false;
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
        this->performFullSync();
    }

    void performFullSync() {
        Notification::create("Saving...", NotificationIcon::Info)->show();
        if (this->saveLocally()) {
            this->performCloudSave();
        }
    }

    bool saveLocally() {
        auto gameManager = GameManager::get();
        if (!gameManager) return false;
        gameManager->save();

        try {
            auto savePath = geode::utils::dirs::getGameDir();
            auto modSavePath = Mod::get()->getSaveDir();
            
            // Files to sync
            std::vector<std::string> files = {"CCGameManager.dat", "CCLocalLevels.dat"};
            
            for (const auto& file : files) {
                auto src = savePath / file;
                auto dest = modSavePath / "data.dat"; // Consolidating or naming it data.dat as requested
                
                if (fs::exists(src)) {
                    fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
                }
            }
            return true;
        } catch (const std::exception& e) {
            log::error("SyncGD: Failed to copy files: {}", e.what());
            return false;
        }
    }

    void performCloudSave() {
        m_pendingSync = true;
        auto accountLayer = AccountLayer::create();
        if (accountLayer) {
            accountLayer->doBackup();
        }
    }

    void autoLoad() {
        auto modSavePath = Mod::get()->getSaveDir() / "data.dat";
        if (fs::exists(modSavePath)) {
            Notification::create("Loading data.dat...", NotificationIcon::Info)->show();
            // Logic to restore files from data.dat would go here
        }
    }
};

class $modify(SRAccountLayer, AccountLayer) {
    struct Fields {
        int m_attempts = 0;
        CCLabelBMFont* m_statusLabel = nullptr;
    };

    void customSetup() {
        AccountLayer::customSetup();
        
        // Fullscreen Attempt UI
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto bg = CCLayerColor::create({0, 0, 0, 150});
        bg->setID("sync-overlay");
        this->addChild(bg, 100);

        m_fields->m_statusLabel = CCLabelBMFont::create("Syncing...", "goldFont.fnt");
        m_fields->m_statusLabel->setPosition(winSize / 2);
        bg->addChild(m_fields->m_statusLabel);
        
        bg->setVisible(false);
    }

    void updateStatus(const std::string& text) {
        if (m_fields->m_statusLabel) {
            m_fields->m_statusLabel->setString(text.c_str());
            if (auto bg = this->getChildByID("sync-overlay")) {
                bg->setVisible(true);
            }
        }
    }

    void backupAccountFailed(const BackupAccountError p0, const int p1) {
        if (static_cast<int>(p0) == -1 && m_fields->m_attempts < 5) {
            m_fields->m_attempts++;
            this->updateStatus(fmt::format("Retrying Backup... Attempt {}", m_fields->m_attempts));
            
            this->runAction(CCSequence::create(
                CCDelayTime::create(2.0f),
                CCCallFunc::create(this, callfunc_selector(SRAccountLayer::doBackup)),
                nullptr
            ));
            return;
        }
        AccountLayer::backupAccountFailed(p0, p1);
    }

    void backupAccountFinished() {
        AccountLayer::backupAccountFinished();
        Notification::create("Backup Success!", NotificationIcon::Success)->show();
        if (auto bg = this->getChildByID("sync-overlay")) bg->setVisible(false);
    }

    void syncAccountFailed(const BackupAccountError p0, const int p1) {
        if (static_cast<int>(p0) == -1 && m_fields->m_attempts < 5) {
            m_fields->m_attempts++;
            this->updateStatus(fmt::format("Retrying Sync... Attempt {}", m_fields->m_attempts));
            
            this->runAction(CCSequence::create(
                CCDelayTime::create(2.0f),
                CCCallFunc::create(this, callfunc_selector(SRAccountLayer::doSync)),
                nullptr
            ));
            return;
        }
        AccountLayer::syncAccountFailed(p0, p1);
    }

    void syncAccountFinished() {
        AccountLayer::syncAccountFinished();
        Notification::create("Sync Success!", NotificationIcon::Success)->show();
        if (auto bg = this->getChildByID("sync-overlay")) bg->setVisible(false);
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
            manager->autoLoad();
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
