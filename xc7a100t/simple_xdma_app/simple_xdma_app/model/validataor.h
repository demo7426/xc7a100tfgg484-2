/*************************************************
Copyright (C), 2026-2040    , Level Chip Co., Ltd.
文件名:	validataor.h
作  者:	钱锐      版本: V1.0     新建日期: 2026.08.24
描  述: validataor 校验
备  注:	
修改记录:

  1.  日期: 2026.08.24
      作者: 钱锐
      内容:
          1) 此为模板第一个版本；
      版本:V1.0

*************************************************/

#pragma once

#include <string>
#include <vector>


namespace hzcc
{
    namespace simple_xdma_app
    {

        enum class FILED_TYPE : int
        {
            NONE = 0,                       //无效
            PCIE_BAR,                       //PCIe Bar
            PCIE_CONFIG_SPACE,              //PCIe 配置空间
            PCIE_XMDA,                      //PCIe XDMA
        };

        enum class MESSAGE_TYPE : int
        {
            NONE = 0,           //无效
            INFOMATION,         //提示
            WARNING,            //警告
            CRITICAL,           //错误
        };

        typedef struct _VALIDATOR_ERROR
        {
            std::string name;                   //名称
            FILED_TYPE filed;                   //作用域
            MESSAGE_TYPE info_type;             //信息类型
            std::string detail_info;            //详细信息
            unsigned long long tag;             //错误标志;(一般为错误数据的内存地址)
        }VALIDATOR_ERROR, *PVALIDATOR_ERROR;

        class CIValidator
        {
        protected:

        public:
            CIValidator() = default;
            virtual ~CIValidator() = 0;

            /// <summary>
            /// 数据校验
            /// </summary>
            /// <param name=""></param>
            virtual void Validate(std::vector<VALIDATOR_ERROR>& errors) = 0;

        private:

        };

    }
}

