#include "PrintInfoTreeWidget.h"
#include "../Inc/RYPrtCtler.h"
#include <QHeaderView>
#include <QDebug>

// 外部变量
extern LPPRINTER_INFO g_pSysInfo;

PrintInfoTreeWidget::PrintInfoTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    setHeaderHidden(false);
    setRootIsDecorated(true);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    
    // 设置列数
    setColumnCount(8);
    
    // 设置表头
    QStringList headers;
    headers << "打印板卡" << "状态" << "设备ID" << "FPGA版本" 
            << "MCU版本" << "Appmain版本" << "驱动连接" << "其他";
    setHeaderLabels(headers);
    
    // 设置列宽
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    
    setupTree();
}

PrintInfoTreeWidget::~PrintInfoTreeWidget()
{
}

void PrintInfoTreeWidget::initColumns(const QStringList &headers)
{
    setColumnCount(headers.size());
    setHeaderLabels(headers);
}

void PrintInfoTreeWidget::setupTree()
{
    clear();
}

void PrintInfoTreeWidget::updateDeviceInfo()
{
    if (!g_pSysInfo) {
        return;
    }
    
    clear();
    
    // 创建根节点
    QTreeWidgetItem *rootItem = createRootItem("打印系统");
    
    // 遍历所有MCP
    for (int p = 0; p < MAX_MCP_CNT; p++) {
        if (g_pSysInfo->nMcpValidMask & (1 << p)) {
            QString cardName = QString("打印卡%1").arg(p + 1);
            QTreeWidgetItem *cardItem = createChildItem(rootItem, cardName);
            
            // 设置状态
            QString status = (g_pSysInfo->nMaiMcpIndex == p) ? "主" : "从";
            setItemText(cardItem, 1, status);
            
            // 设置设备ID
            QString deviceID = QString("%1").arg(g_pSysInfo->nMcpDeviceID[p], 8, 16, QChar('0')).toUpper();
            setItemText(cardItem, 2, deviceID);
            
            // 设置FPGA版本
            QString fpgaVer = QString("%1").arg(g_pSysInfo->nMcpFpgaVer[p], 8, 16, QChar('0')).toUpper();
            setItemText(cardItem, 3, fpgaVer);
            
            // 设置MCU版本
            QString mcuVer = QString("%1").arg(g_pSysInfo->nMcpMcuVersion[p], 8, 16, QChar('0')).toUpper();
            setItemText(cardItem, 4, mcuVer);
            
            // 设置App版本
            QString appVer = QString("%1").arg(g_pSysInfo->nMcpAppVersion[p], 8, 16, QChar('0')).toUpper();
            setItemText(cardItem, 5, appVer);
            
            // 设置驱动连接
            QString drvLink = QString("%1").arg(g_pSysInfo->nMcpDrvLinkMask[p], 8, 16, QChar('0')).toUpper();
            setItemText(cardItem, 6, drvLink);
            
            cardItem->setExpanded(true);
        }
    }
    
    rootItem->setExpanded(true);
}

void PrintInfoTreeWidget::updateVTListInfo()
{
    if (!g_pSysInfo) {
        return;
    }
    
    // 更新VT列表信息（温度、电压等）
    // 这里需要根据具体的树形结构来实现
    // 对应MFC版本的UpdateVTListInfo()函数
}

void PrintInfoTreeWidget::clearAll()
{
    clear();
}

QTreeWidgetItem* PrintInfoTreeWidget::createRootItem(const QString &text)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(this);
    item->setText(0, text);
    return item;
}

QTreeWidgetItem* PrintInfoTreeWidget::createChildItem(QTreeWidgetItem *parent, const QString &text)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, text);
    return item;
}

void PrintInfoTreeWidget::setItemText(QTreeWidgetItem *item, int column, const QString &text)
{
    if (item && column >= 0 && column < columnCount()) {
        item->setText(column, text);
    }
}

