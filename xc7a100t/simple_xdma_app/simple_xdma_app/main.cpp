#include "stdafx.h"
#include "simple_xdma_app.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    // 步骤1：启用高DPI缩放（Qt5.6+）
    // 自动根据系统DPI计算缩放因子
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // 步骤2：启用高DPI位图支持（Qt5.10+）
    // 确保QPixmap/QImage在高DPI下使用高分辨率资源
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // 步骤3：Qt6 额外设置（可选，增强多屏DPI支持）
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough // 禁用缩放因子舍入，提高精度
    );
#endif

    QApplication a(argc, argv);
    simple_xdma_app w;
    w.show();
    return a.exec();
}
