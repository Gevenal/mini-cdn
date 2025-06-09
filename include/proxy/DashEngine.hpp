// 放“码率选择”逻辑，结合带宽估算与 MpdParser 的输出
#pragma once

#include "MpdParser.hpp"

namespace proxy
{
    class DashEngine
    {
    public:
        explicit DashEngine(const std::string &mpdXML);
        // 输入带宽（kbps），返回最佳 Representation（找不到时可抛异常/返回最低清晰度）
        Representation selectRepresentation(int bandwidthKbps) const;
        // 获取当前所有候选
        std::vector<Representation> getRepresentations() const;

    private:
        std::vector<Representation> representations_;
    };
}