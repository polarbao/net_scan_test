#ifndef PARAMETERMANAGER_H
#define PARAMETERMANAGER_H

#include <QObject>
#include <QString>
#include "../Inc/RYPrtCtler.h"

/**
 * @brief 参数管理类
 * 
 * 对应MFC版本的CNetScanDemoApp中的参数保存/加载函数
 * 负责所有参数的持久化存储
 */
class ParameterManager : public QObject
{
    Q_OBJECT

public:
    explicit ParameterManager(QObject *parent = nullptr);
    
    // 用户参数
    bool saveUserParam(const RYUSR_PARAM &param);
    bool loadUserParam(RYUSR_PARAM &param);
    
    // 图层参数
    bool saveLayerParam(const PRTIMG_LAYER &layer);
    bool loadLayerParam(PRTIMG_LAYER &layer);
    
    // 作业参数
    bool saveJobParam(const PRTJOB_ITEM &job);
    bool loadJobParam(PRTJOB_ITEM &job);
    
    // 校准参数
    bool saveCalibrationParam(const RYCalbrationParam &calibration);
    bool loadCalibrationParam(RYCalbrationParam &calibration);
    
    // 运动参数（可选）
#ifdef RY_MOVE_CTL
    bool saveMoveParam(const MOV_Config &config);
    bool loadMoveParam(MOV_Config &config);
#endif
    
    QString applicationPath() const { return m_appPath; }

private:
    QString m_appPath;
    QString getParamFilePath(const QString &filename) const;
    bool writeBinaryFile(const QString &filePath, const void *data, size_t size);
    bool readBinaryFile(const QString &filePath, void *data, size_t size);
};

#endif // PARAMETERMANAGER_H

