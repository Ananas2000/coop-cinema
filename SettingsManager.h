#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QStandardPaths>

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    static SettingsManager& instance() {
        static SettingsManager instance;
        return instance;
    }

    enum SettingsCategory {
        NETWORK,
        PLAYBACK,
        UI,
        PROFILE
    };
    Q_ENUM(SettingsCategory)

        QVariant getSetting(SettingsCategory category, const QString& key,
            const QVariant& defaultValue = QVariant()) const;
    void setSetting(SettingsCategory category, const QString& key,
        const QVariant& value);

    QString serverAddress() const;
    void setServerAddress(const QString& address);

    int serverPort() const;
    void setServerPort(int port);

    float playbackSpeed() const;
    void setPlaybackSpeed(float speed);

    bool hardwareAccelerationEnabled() const;
    void setHardwareAccelerationEnabled(bool enabled);

    void loadProfile(const QString& profileName);
    void saveCurrentProfile();
    void deleteProfile(const QString& profileName);

signals:
    void settingChanged(SettingsManager::SettingsCategory category,
        const QString& key,
        const QVariant& value);
    void profileLoaded(const QString& profileName);
    void settingsReset();

public slots:
    void resetToDefaults();

private:
    explicit SettingsManager(QObject* parent = nullptr);
    ~SettingsManager() = default;

    QSettings m_settings;
    QString m_currentProfile = "default";

    QString categoryToString(SettingsCategory category) const;
    void validateNetworkSettings();
};

#endif