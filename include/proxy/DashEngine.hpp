// Contains bitrate selection logic, based on bandwidth estimation and MpdParser output
#pragma once

#include "MpdParser.hpp"

namespace proxy
{
    class DashEngine
    {
    public:
        explicit DashEngine(const std::string &mpdXML);
        // Given the input bandwidth (kbps), returns the best Representation (throws or returns lowest quality if none found)
        Representation selectRepresentation(int bandwidthKbps) const;
        // Retrieves all available candidate Representations
        std::vector<Representation> getRepresentations() const;

    private:
        std::vector<Representation> representations_;
    };
}