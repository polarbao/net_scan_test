#ifndef PARAMETERMANAGER_H
#define PARAMETERMANAGER_H

#include <QObject>
#include <QSettings>
#include "../Inc/ryprtapi.h" // For RYUSR_PARAM, PRTIMG_LAYER, PRTJOB_ITEM, RYCalbrationParam
#include "PrintThread.h"    // For MOV_Config

class ParameterManager : public QObject
{
    Q_OBJECT
public:
    explicit ParameterManager(QObject *parent = nullptr);

    bool loadUserParam(RYUSR_PARAM& param);
    bool saveUserParam(const RYUSR_PARAM& param);

    bool loadLayerParam(PRTIMG_LAYER& param);
    bool saveLayerParam(const PRTIMG_LAYER& param);

    bool loadJobParam(PRTJOB_ITEM& param);
    bool saveJobParam(const PRTJOB_ITEM& param);

    bool loadCalibrationParam(RYCalbrationParam& param);
    bool saveCalibrationParam(const RYCalbrationParam& param);

#ifdef RY_MOVE_CTL
    bool loadMoveParam(MOV_Config& param);
    bool saveMoveParam(const MOV_Config& param);
#endif
    
    QString applicationPath() const { return m_appPath; }

private:
    QString m_appPath;
    QString getParamFilePath(const QString &filename) const;
    bool writeBinaryFile(const QString &filePath, const void *data, size_t size);
    bool readBinaryFile(const QString &filePath, void *data, size_t size);
};

#endif // PARAMETERMANAGER_H

