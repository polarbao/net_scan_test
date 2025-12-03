#include "SysParamDialog.h"
#include "ui_SysParamDialog.h"
#include "../Inc/RYPrtCtler.h"

// 外部变量
extern RYUSR_PARAM g_sysParam;

SysParamDialog::SysParamDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SysParamDialog)
    , m_cbDistance(0.0f)
    , m_cbWidth(0.0f)
    , m_cbInterval(0.0f)
    , m_cbInkDensity(0.0f)
    , m_prtPeriod(0)
    , m_cbValidMask(0)
    , m_cbPosMode(0)
    , m_equiDis(false)
    , m_microJetUnit(0)
    , m_microJetCount(0)
    , m_microStpJet(0)
    , m_encReverse(false)
    , m_idleFlash(false)
    , m_bigInkDrops(false)
    , m_colorSel(0)
    , m_overlapProc(0)
    , m_flashPrtFrequency(0.0f)
    , m_brustValidSec(0.0f)
    , m_brustCycleSec(0.0f)
    , m_useAdib(false)
{
    ui->setupUi(this);
    
    // 从全局变量加载数据
    loadFromGlobal();
    
    // 初始化UI
    initUI();
    
    // 连接信号槽
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &SysParamDialog::onAccepted);
    if (ui->buttonSet) {
        connect(ui->buttonSet, &QPushButton::clicked,
                this, &SysParamDialog::onButtonSetClicked);
    }
}

SysParamDialog::~SysParamDialog()
{
    delete ui;
}

void SysParamDialog::initUI()
{
    // 设置控件值（需要根据实际UI设计）
    // 这里只是示例，实际需要根据UI文件中的控件名称设置
}

void SysParamDialog::loadFromGlobal()
{
    // 从全局变量g_sysParam加载数据
    // 需要根据RYUSR_PARAM结构体的实际定义来加载
    // 这里只是示例
    m_cbDistance = 0.0f;  // g_sysParam.fCBDistance;
    m_cbWidth = 0.0f;     // g_sysParam.fCBWidth;
    // ... 其他参数
}

void SysParamDialog::onAccepted()
{
    // 从控件读取数据
    // 需要根据实际UI控件读取
    
    // 保存到全局变量
    saveToGlobal();
    
    accept();
}

void SysParamDialog::saveToGlobal()
{
    // 保存到全局变量g_sysParam
    // 需要根据RYUSR_PARAM结构体的实际定义来保存
    // g_sysParam.fCBDistance = m_cbDistance;
    // g_sysParam.fCBWidth = m_cbWidth;
    // ... 其他参数
}

void SysParamDialog::onButtonSetClicked()
{
    // 设置按钮点击处理
    // 可能需要调用API设置参数
}

void SysParamDialog::initFlashList()
{
    // 初始化闪喷列表
    // 对应MFC版本的InitFlashList()
}

void SysParamDialog::reDrawFlashList()
{
    // 重绘闪喷列表
    // 对应MFC版本的ReDrawFlashList()
}

