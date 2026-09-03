#pragma once

#include <QString>
#include <QVariant>
#include <QMap>
#include <QObject>
#include <memory>

class QSettings;

/**
 * @brief Settings data model with D-Bus integration
 * 
 * Handles:
 * - Local Qt settings (QSettings)
 * - D-Bus system settings (NetworkManager, systemd, etc.)
 * - Permission management (Polkit)
 */
class SettingsModel : public QObject {
    Q_OBJECT

public:
    explicit SettingsModel(QObject* parent = nullptr);
    ~SettingsModel();

    // Get settings
    QVariant getSetting(const QString& key) const;
    QVariant getSystemSetting(const QString& key) const;

    // Set settings
    bool setSetting(const QString& key, const QVariant& value);
    bool setSystemSetting(const QString& key, const QVariant& value);

    // Queries
    bool hasSetting(const QString& key) const;
    QStringList allSettings() const;

    // D-Bus operations
    bool getNetworkStatus() const;
    bool setNetworkStatus(bool enabled);
    bool getBluetoothStatus() const;
    bool setBluetoothStatus(bool enabled);

    // Display settings
    QStringList getDisplays() const;
    bool setDisplayResolution(const QString& displayName, int width, int height, int refreshRate);
    int getDisplayScaling() const;
    bool setDisplayScaling(int percent);

    // Sound settings
    QStringList getAudioDevices() const;
    int getVolume() const;
    bool setVolume(int percent);

    // Power settings
    QString getPowerProfile() const;
    bool setPowerProfile(const QString& profile);

signals:
    void settingChanged(const QString& key, const QVariant& value);
    void networkStatusChanged(bool enabled);
    void bluetoothStatusChanged(bool enabled);
    void displaySettingsChanged();
    void soundSettingsChanged();

private:
    // D-Bus setup
    void setupDBusConnections();
    void connectToNetworkManager();
    void connectToSystemd();
    void connectToUdisks2();

    std::unique_ptr<QSettings> m_localSettings;
    QMap<QString, QVariant> m_cachedSettings;
    bool m_dBusInitialized;
};
