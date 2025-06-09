#include "../include/proxy/DashEngine.hpp"
#include <stdexcept>
#include <algorithm>

namespace proxy
{

    // Constructor: Parses MPD XML to populate the representations vector.
    DashEngine::DashEngine(const std::string &mpdXML)
    {
        // Use MpdParser to parse all representations from the given MPD XML.
        MpdParser parser(mpdXML);
        representations_ = parser.getRepresentations();

        // Optional: sort representations from lowest to highest bandwidth for easier selection
        std::sort(representations_.begin(), representations_.end(),
                  [](const Representation &a, const Representation &b)
                  {
                      return a.bandwidth < b.bandwidth;
                  });
    }

    // Returns the best representation for the given bandwidth (in kbps).
    // Selects the highest-quality representation whose bandwidth requirement
    // does not exceed the available bandwidth.
    // If no such representation exists, returns the lowest-quality one.
    Representation DashEngine::selectRepresentation(int bandwidthKbps) const
    {
        if (representations_.empty())
        {
            throw std::runtime_error("No representations available.");
        }

        // Convert input bandwidth from kbps to bps (to match representation.bandwidth units)
        unsigned int bandwidthBps = static_cast<unsigned int>(bandwidthKbps * 1000);

        // Default to the lowest-bandwidth representation
        const Representation *best = &representations_.front();

        // Find the best match: highest bandwidth that fits under the limit
        for (const auto &rep : representations_)
        {
            if (rep.bandwidth <= bandwidthBps)
            {
                best = &rep;
            }
            else
            {
                break; // Since representations are sorted, we can stop here
            }
        }
        return *best;
    }

    // Returns all candidate representations.
    std::vector<Representation> DashEngine::getRepresentations() const
    {
        return representations_;
    }

} // namespace proxy
