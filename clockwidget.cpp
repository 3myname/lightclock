#include "clockwidget.h"
#include <QTimer>
#include <QTime>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QApplication>
#include <QIcon>
#include <QStyle>
#include <QPainter>
#include <QScreen>
#include <QSettings>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

ClockWidget::ClockWidget(QWidget *parent)
    : QWidget(parent), m_isDragging(false), m_textColor(QColor("#00FFCC")),
      m_fontDialog(nullptr), m_colorDialog(nullptr)
{
    // 去掉边框，使用工具属性（不占任务栏）
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(0);

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);

    QFont defaultFont("Consolas", 48, QFont::Bold);
    m_label->setFont(defaultFont);
    layout->addWidget(m_label);

    loadSettings();
    m_label->setStyleSheet(QString("color: %1; background: transparent;").arg(m_textColor.name()));

    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ClockWidget::updateTime);
    timer->start(1000);
    updateTime();

    // ================= 系统托盘 =================
    m_trayMenu = new QMenu(this);
    QAction *showAction = m_trayMenu->addAction("显示/隐藏时钟");
    QAction *fontAction = m_trayMenu->addAction("设置字体大小");
    QAction *colorAction = m_trayMenu->addAction("设置文字颜色");
    m_trayMenu->addSeparator();
    QAction *quitAction = m_trayMenu->addAction("退出程序");

    connect(showAction, &QAction::triggered, this, [this]() {
        if (this->isVisible()) this->hide();
        else this->show();
    });
    connect(fontAction, &QAction::triggered, this, &ClockWidget::changeFont);
    connect(colorAction, &QAction::triggered, this, &ClockWidget::changeColor);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    m_trayIcon->setToolTip("轻量时钟");
    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            if (this->isVisible()) this->hide();
            else this->show();
        }
    });
}

ClockWidget::~ClockWidget() {}

void ClockWidget::updateTime()
{
    m_label->setText(QTime::currentTime().toString("hh:mm:ss"));
    adjustSize();
    setMask(rect());
}

// ================= 设置对话框 =================
void ClockWidget::changeFont()
{
    if (m_fontDialog) { m_fontDialog->show(); m_fontDialog->raise(); m_fontDialog->activateWindow(); return; }

    m_fontDialog = new QFontDialog(m_label->font(), this);
    m_fontDialog->setWindowTitle("设置时钟字体");
    m_fontDialog->setAttribute(Qt::WA_DeleteOnClose);
    m_fontDialog->setOption(QFontDialog::NoButtons, true);
    m_fontDialog->setWindowFlag(Qt::WindowStaysOnTopHint, true);

    connect(m_fontDialog, &QFontDialog::currentFontChanged, this, [this](const QFont &font){
        m_label->setFont(font);
        adjustSize();
        setMask(rect());
    });
    connect(m_fontDialog, &QFontDialog::destroyed, this, [this](){ m_fontDialog = nullptr; });

    m_fontDialog->show();
}

void ClockWidget::changeColor()
{
    if (m_colorDialog) { m_colorDialog->show(); m_colorDialog->raise(); m_colorDialog->activateWindow(); return; }

    m_colorDialog = new QColorDialog(m_textColor, this);
    m_colorDialog->setWindowTitle("设置时钟颜色");
    m_colorDialog->setAttribute(Qt::WA_DeleteOnClose);
    m_colorDialog->setOption(QColorDialog::NoButtons, true);
    m_colorDialog->setWindowFlag(Qt::WindowStaysOnTopHint, true);

    connect(m_colorDialog, &QColorDialog::currentColorChanged, this, [this](const QColor &color){
        if (color.isValid()) {
            m_textColor = color;
            m_label->setStyleSheet(QString("color: %1; background: transparent;").arg(m_textColor.name()));
        }
    });
    connect(m_colorDialog, &QColorDialog::destroyed, this, [this](){ m_colorDialog = nullptr; });

    m_colorDialog->show();
}

// ================= 拖拽 =================
void ClockWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragPos = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void ClockWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - m_dragPos);
        event->accept();
    }
}

void ClockWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        saveSettings();
        event->accept();
    }
}

void ClockWidget::paintEvent(QPaintEvent *event) { Q_UNUSED(event); }

// ================= 降级置底核心 =================
void ClockWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    adjustSize();
    setMask(rect());

#ifdef Q_OS_WIN
    // 直接使用 Windows API 把窗口塞到所有窗口的最底层
    HWND hwnd = (HWND)this->winId();
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#endif
}

void ClockWidget::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QWidget::closeEvent(event);
}

// ================= 配置保存 =================
void ClockWidget::saveSettings()
{
    QSettings settings("LightClock", "ClockApp");
    settings.setValue("color", m_textColor.name());
    settings.setValue("font", m_label->font().toString());
    settings.setValue("pos", this->pos());
}

void ClockWidget::loadSettings()
{
    QSettings settings("LightClock", "ClockApp");

    QString colorStr = settings.value("color", "#00FFCC").toString();
    m_textColor = QColor(colorStr);

    QString fontStr = settings.value("font").toString();
    if (!fontStr.isEmpty()) {
        QFont font;
        if (font.fromString(fontStr)) m_label->setFont(font);
    }

    QPoint pos = settings.value("pos", QPoint(-1, -1)).toPoint();
    if (pos.x() != -1 && pos.y() != -1) {
        this->move(pos);
    } else {
        QRect screenRect = QApplication::primaryScreen()->availableGeometry();
        int x = (screenRect.width() - this->width()) / 2;
        int y = screenRect.top() + 50;
        this->move(x, y);
    }
}
