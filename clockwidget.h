#ifndef CLOCKWIDGET_H
#define CLOCKWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPoint>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QColor>
#include <QFontDialog>
#include <QColorDialog>

class ClockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClockWidget(QWidget *parent = nullptr);
    ~ClockWidget();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void updateTime();
    void changeFont();
    void changeColor();

private:
    QLabel *m_label;
    QPoint m_dragPos;
    bool m_isDragging;

    QColor m_textColor;

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;

    QFontDialog *m_fontDialog;
    QColorDialog *m_colorDialog;

    void saveSettings();
    void loadSettings();
};

#endif // CLOCKWIDGET_H
