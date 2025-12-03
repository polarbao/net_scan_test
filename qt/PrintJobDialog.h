#ifndef PRINTJOBDIALOG_H
#define PRINTJOBDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class PrintJobDialog;
}

/**
 * @brief 打印作业对话框
 * 
 * 对应MFC版本的CPrtJobDlg
 * 负责打印作业参数配置
 */
class PrintJobDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrintJobDialog(QWidget *parent = nullptr);
    ~PrintJobDialog();
    
    // Getter方法
    int grayBits() const { return m_grayBits; }
    float xOutDPI() const { return m_xOutDPI; }
    float yOutDPI() const { return m_yOutDPI; }
    float clipWidth() const { return m_clipWidth; }
    float clipHeight() const { return m_clipHeight; }
    float yPrtPos() const { return m_yPrtPos; }
    float xPrtPos() const { return m_xPrtPos; }
    int fileType() const { return m_fileType; }
    QString jobFilePath() const { return m_jobFilePath; }
    QString jobName() const { return m_jobName; }
    int jobID() const { return m_jobID; }
    int imageLayerCount() const { return m_imageLayerCount; }
    bool whiteJump() const { return m_whiteJump; }
    bool cycleOff() const { return m_cycleOff; }
    bool radomJetOff() const { return m_radomJetOff; }
    bool xMirror() const { return m_xMirror; }
    bool yMirror() const { return m_yMirror; }
    bool doubleYDpi() const { return m_doubleYDpi; }
    bool screenshotPrt() const { return m_screenshotPrt; }
    
    // Setter方法
    void setGrayBits(int bits) { m_grayBits = bits; }
    void setXOutDPI(float dpi) { m_xOutDPI = dpi; }
    void setYOutDPI(float dpi) { m_yOutDPI = dpi; }
    void setClipWidth(float width) { m_clipWidth = width; }
    void setClipHeight(float height) { m_clipHeight = height; }
    void setYPrtPos(float pos) { m_yPrtPos = pos; }
    void setXPrtPos(float pos) { m_xPrtPos = pos; }
    void setFileType(int type) { m_fileType = type; }
    void setJobFilePath(const QString &path) { m_jobFilePath = path; }
    void setJobName(const QString &name) { m_jobName = name; }
    void setJobID(int id) { m_jobID = id; }
    void setImageLayerCount(int count) { m_imageLayerCount = count; }
    void setWhiteJump(bool enable) { m_whiteJump = enable; }
    void setCycleOff(bool enable) { m_cycleOff = enable; }
    void setRadomJetOff(bool enable) { m_radomJetOff = enable; }
    void setXMirror(bool enable) { m_xMirror = enable; }
    void setYMirror(bool enable) { m_yMirror = enable; }
    void setDoubleYDpi(bool enable) { m_doubleYDpi = enable; }
    void setScreenshotPrt(bool enable) { m_screenshotPrt = enable; }

private slots:
    void onAccepted();

private:
    void initUI();
    void loadFromGlobal();
    void saveToGlobal();
    
    Ui::PrintJobDialog *ui;
    
    // 参数变量
    int m_grayBits;
    float m_xOutDPI;
    float m_yOutDPI;
    float m_clipWidth;
    float m_clipHeight;
    float m_yPrtPos;
    float m_xPrtPos;
    int m_fileType;
    QString m_jobFilePath;
    QString m_jobName;
    int m_jobID;
    int m_imageLayerCount;
    bool m_whiteJump;
    bool m_cycleOff;
    bool m_radomJetOff;
    bool m_xMirror;
    bool m_yMirror;
    bool m_doubleYDpi;
    bool m_screenshotPrt;
};

#endif // PRINTJOBDIALOG_H

