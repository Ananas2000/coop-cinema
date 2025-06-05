#include "SettingsManager.h"

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent),
    m_settings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/settings.ini", QSettings::IniFormat)
{
    validateNetworkSettings();
}

QString SettingsManager::categoryToString(SettingsCategory category) const {
    switch (category) {
    case NETWORK:  return "Network";
    case PLAYBACK: return "Playback";
    case UI:       return "UI";
    case PROFILE:  return "Profile";
    default:       return "General";
    }
}

QVariant SettingsManager::getSetting(SettingsCategory category, const QString& key, const QVariant& defaultValue) const {
    QString group = m_currentProfile + "/" + categoryToString(category);
    return m_settings.value(group + "/" + key, defaultValue);
}

void SettingsManager::setSetting(SettingsCategory category, const QString& key, const QVariant& value) {
    QString group = m_currentProfile + "/" + categoryToString(category);
    m_settings.setValue(group + "/" + key, value);
    emit settingChanged(category, key, value);
}

QString SettingsManager::serverAddress() const {
    return getSetting(NETWORK, "serverAddress", "127.0.0.1").toString();
}

void SettingsManager::setServerAddress(const QString& address) {
    setSetting(NETWORK, "serverAddress", address);
}

int SettingsManager::serverPort() const {
    return getSetting(NETWORK, "serverPort", 8888).toInt();
}

void SettingsManager::setServerPort(int port) {
    setSetting(NETWORK, "serverPort", port);
}

float SettingsManager::playbackSpeed() const {
    return getSetting(PLAYBACK, "playbackSpeed", 1.0f).toFloat();
}

void SettingsManager::setPlaybackSpeed(float speed) {
    setSetting(PLAYBACK, "playbackSpeed", speed);
}

bool SettingsManager::hardwareAccelerationEnabled() const {
    return getSetting(PLAYBACK, "hardwareAcceleration", true).toBool();
}

void SettingsManager::setHardwareAccelerationEnabled(bool enabled) {
    setSetting(PLAYBACK, "hardwareAcceleration", enabled);
}

void SettingsManager::loadProfile(const QString& profileName) {
    m_currentProfile = profileName;
    emit profileLoaded(profileName);
}

void SettingsManager::saveCurrentProfile() {
    m_settings.sync();
}

void SettingsManager::deleteProfile(const QString& profileName) {
    QString groupPrefix = profileName + "/";
    QStringList allKeys = m_settings.allKeys();

    for (const QString& key : allKeys) {
        if (key.startsWith(groupPrefix)) {
            m_settings.remove(key);
        }
    }

    if (profileName == m_currentProfile) {
        m_currentProfile = "default";
    }
}

void SettingsManager::resetToDefaults() {
    QString prefix = m_currentProfile + "/";
    QStringList keys = m_settings.allKeys();

    for (const QString& key : keys) {
        if (key.startsWith(prefix)) {
            m_settings.remove(key);
        }
    }

    m_settings.sync();
    emit settingsReset();
}

void SettingsManager::validateNetworkSettings() {
    if (serverAddress().isEmpty()) {
        setServerAddress("127.0.0.1");
    }

    int port = serverPort();
    if (port <= 0 || port > 65535) {
        setServerPort(8888);
    }
}