#ifndef PRINTINFOTREEWIDGET_H
#define PRINTINFOTREEWIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>

/**
 * @brief 打印信息树形控件
 * 
 * 对应MFC版本的CSWListTreeCtrl
 * 负责树形结构数据展示和多列数据显示
 */
class PrintInfoTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit PrintInfoTreeWidget(QWidget *parent = nullptr);
    ~PrintInfoTreeWidget();
    
    void initColumns(const QStringList &headers);
    void updateDeviceInfo();
    void updateVTListInfo();
    void clearAll();

private:
    void setupTree();
    QTreeWidgetItem* createRootItem(const QString &text);
    QTreeWidgetItem* createChildItem(QTreeWidgetItem *parent, const QString &text);
    void setItemText(QTreeWidgetItem *item, int column, const QString &text);
};

#endif // PRINTINFOTREEWIDGET_H

