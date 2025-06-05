#ifndef VIDEOCONTROLS_H
#define VIDEOCONTROLS_H

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QSpacerItem>

class VideoControls : public QWidget
{
    Q_OBJECT
public:
    explicit VideoControls(QWidget* parent = nullptr);

    void setPlaybackState(bool isPlaying);
    void setVolume(int volume);
    void setPlaybackRate(float rate);
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);

signals:
    void playPauseClicked();
    void volumeChanged(int volume);
    void playbackRateChanged(float rate);
    void fullScreenClicked();

private:
    QPushButton* m_playPauseBtn;
    QSlider* m_positionSlider;
    QLabel* m_positionLabel;
    QLabel* m_durationLabel;
    QSlider* m_volumeSlider;
    QComboBox* m_rateCombo;
    QPushButton* m_fullScreenBtn;
    QIcon m_playIcon;
    QIcon m_pauseIcon;
};

#endif // VIDEOCONTROLS_H