#include "VideoControls.h"
#include <QIcon>
#include <QStyle>
#include <QTime>
#include <QApplication>

VideoControls::VideoControls(QWidget* parent)
    : QWidget(parent),
    m_playIcon(QIcon::fromTheme("media-playback-start")),
    m_pauseIcon(QIcon::fromTheme("media-playback-pause"))
{
    setAutoFillBackground(true);
    setStyleSheet("background-color: rgba(0, 0, 0, 180); color: white;");
    setFixedHeight(40);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(10);

    // Кнопка Play/Pause
    m_playPauseBtn = new QPushButton(this);
    m_playPauseBtn->setIcon(m_playIcon);
    m_playPauseBtn->setFixedSize(32, 32);
    m_playPauseBtn->setStyleSheet("background: transparent; border: none;");

    // Слайдер позиции
    m_positionSlider = new QSlider(Qt::Horizontal, this);
    m_positionSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    height: 4px;"
        "    background: #555;"
        "    border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "    width: 12px;"
        "    height: 12px;"
        "    background: white;"
        "    margin: -4px 0;"
        "    border-radius: 6px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "    background: #1E90FF;"
        "    border-radius: 2px;"
        "}"
    );

    // Метки времени
    m_positionLabel = new QLabel("00:00", this);
    m_positionLabel->setMinimumWidth(40);
    m_positionLabel->setAlignment(Qt::AlignCenter);
    m_positionLabel->setStyleSheet("color: white; font: 10pt;");

    m_durationLabel = new QLabel("/ 00:00", this);
    m_durationLabel->setMinimumWidth(50);
    m_durationLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_durationLabel->setStyleSheet("color: #aaa; font: 9pt;");

    // Слайдер громкости
    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(80);
    m_volumeSlider->setFixedWidth(80);
    m_volumeSlider->setStyleSheet(m_positionSlider->styleSheet());

    // Выбор скорости
    m_rateCombo = new QComboBox(this);
    m_rateCombo->addItems({ "0.5x", "0.75x", "1.0x", "1.25x", "1.5x", "2.0x" });
    m_rateCombo->setCurrentText("1.0x");
    m_rateCombo->setFixedWidth(80);

    // Кнопка полного экрана
    m_fullScreenBtn = new QPushButton(this);
    m_fullScreenBtn->setIcon(QIcon::fromTheme("view-fullscreen"));
    m_fullScreenBtn->setFixedSize(32, 32);
    m_fullScreenBtn->setStyleSheet("background: transparent; border: none;");

    // Добавляем элементы в layout
    layout->addWidget(m_playPauseBtn);
    layout->addWidget(m_positionSlider, 4); // Основное пространство
    layout->addWidget(m_positionLabel);
    layout->addWidget(m_durationLabel);
    layout->addWidget(new QLabel("🔊", this));
    layout->addWidget(m_volumeSlider);
    layout->addWidget(new QLabel("Speed:", this));
    layout->addWidget(m_rateCombo);
    layout->addWidget(m_fullScreenBtn);

    // Настройка сигналов
    connect(m_playPauseBtn, &QPushButton::clicked, this, &VideoControls::playPauseClicked);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &VideoControls::volumeChanged);
    connect(m_rateCombo, &QComboBox::currentTextChanged, [this](const QString& text) {
        QString modified = text;
        float rate = modified.replace("x", "").toFloat();
        emit playbackRateChanged(rate);
        });
    connect(m_fullScreenBtn, &QPushButton::clicked, this, &VideoControls::fullScreenClicked);
}

void VideoControls::setPlaybackState(bool isPlaying)
{
    m_playPauseBtn->setIcon(isPlaying ? m_pauseIcon : m_playIcon);
}

void VideoControls::setVolume(int volume)
{
    if (m_volumeSlider->value() != volume) {
        m_volumeSlider->setValue(volume);
    }
}

void VideoControls::setPlaybackRate(float rate)
{
    // Форматируем с одним знаком после запятой
    QString rateText = QString::number(rate, 'f', 1) + "x";
    if (m_rateCombo->currentText() != rateText) {
        m_rateCombo->setCurrentText(rateText);
    }
}


void VideoControls::updatePosition(qint64 position)
{
    QTime time(0, 0, 0, 0);
    time = time.addMSecs(static_cast<int>(position));
    m_positionLabel->setText(time.toString("mm:ss"));
}

void VideoControls::updateDuration(qint64 duration)
{
    QTime time(0, 0, 0, 0);
    time = time.addMSecs(static_cast<int>(duration));
    m_durationLabel->setText("/ " + time.toString("mm:ss"));
}