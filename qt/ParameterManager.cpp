#include "ParameterManager.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QDebug>

ParameterManager::ParameterManager(QObject *parent)
    : QObject(parent)
{
    // 获取应用程序路径
    m_appPath = QApplication::applicationDirPath();
}

QString ParameterManager::getParamFilePath(const QString &filename) const
{
    return QDir(m_appPath).filePath(filename);
}

bool ParameterManager::writeBinaryFile(const QString &filePath, const void *data, size_t size)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }
    
    QDataStream out(&file);
    out.writeRawData(reinterpret_cast<const char*>(data), static_cast<int>(size));
    
    file.close();
    return true;
}

bool ParameterManager::readBinaryFile(const QString &filePath, void *data, size_t size)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file for reading:" << filePath;
        return false;
    }
    
    QDataStream in(&file);
    int bytesRead = in.readRawData(reinterpret_cast<char*>(data), static_cast<int>(size));
    
    file.close();
    
    if (bytesRead != static_cast<int>(size)) {
        qWarning() << "Failed to read all data from file:" << filePath;
        return false;
    }
    
    return true;
}

bool ParameterManager::saveUserParam(const RYUSR_PARAM &param)
{
    QString filePath = getParamFilePath("userparam.dat");
    return writeBinaryFile(filePath, &param, sizeof(RYUSR_PARAM));
}

bool ParameterManager::loadUserParam(RYUSR_PARAM &param)
{
    QString filePath = getParamFilePath("userparam.dat");
    return readBinaryFile(filePath, &param, sizeof(RYUSR_PARAM));
}

bool ParameterManager::saveLayerParam(const PRTIMG_LAYER &layer)
{
    QString filePath = getParamFilePath("pmc.dat");
    return writeBinaryFile(filePath, &layer, sizeof(PRTIMG_LAYER));
}

bool ParameterManager::loadLayerParam(PRTIMG_LAYER &layer)
{
    QString filePath = getParamFilePath("pmc.dat");
    return readBinaryFile(filePath, &layer, sizeof(PRTIMG_LAYER));
}

bool ParameterManager::saveJobParam(const PRTJOB_ITEM &job)
{
    QString filePath = getParamFilePath("job.dat");
    return writeBinaryFile(filePath, &job, sizeof(PRTJOB_ITEM));
}

bool ParameterManager::loadJobParam(PRTJOB_ITEM &job)
{
    QString filePath = getParamFilePath("job.dat");
    return readBinaryFile(filePath, &job, sizeof(PRTJOB_ITEM));
}

bool ParameterManager::saveCalibrationParam(const RYCalbrationParam &calibration)
{
    QString filePath = getParamFilePath("Calibration.dat");
    return writeBinaryFile(filePath, &calibration, sizeof(RYCalbrationParam));
}

bool ParameterManager::loadCalibrationParam(RYCalbrationParam &calibration)
{
    QString filePath = getParamFilePath("Calibration.dat");
    return readBinaryFile(filePath, &calibration, sizeof(RYCalbrationParam));
}

#ifdef RY_MOVE_CTL
bool ParameterManager::saveMoveParam(const MOV_Config &config)
{
    QString filePath = getParamFilePath("moveParam.dat");
    return writeBinaryFile(filePath, &config, sizeof(MOV_Config));
}

bool ParameterManager::loadMoveParam(MOV_Config &config)
{
    QString filePath = getParamFilePath("moveParam.dat");
    return readBinaryFile(filePath, &config, sizeof(MOV_Config));
}
#endif

