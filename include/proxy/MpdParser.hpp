// Parses the .mpd (XML) file and extracts Representation information
#pragma once

#include <string>
#include <vector>

namespace proxy
{
    /**
     * @brief Represents a media representation from a DASH manifest.
     *
     * This struct aggregates information from <Representation> and <SegmentTemplate> tags
     * to fully describe a video stream of a specific quality and its segments.
     */
    struct Representation
    {
        // --- Attributes from the <Representation> tag ---
        std::string id;             // e.g., "video/avc1"
        unsigned int bandwidth = 0; // Bitrate (bits per second)
        unsigned int width = 0;     // Video width (pixels)
        unsigned int height = 0;    // Video height (pixels)
        std::string codecs;         // e.g., "avc1.4d401f"

        // --- Attributes from the <SegmentTemplate> tag ---
        // URL template for building media segment URLs.
        // e.g., "media/video-$RepresentationID$-$Number$.m4s"
        // The parser needs to replace placeholders like $RepresentationID$ and $Number$.
        std::string media_template_url;

        // URL template for building the initialization segment URL.
        // e.g., "media/video-$RepresentationID$-init.m4s"
        std::string init_template_url;

        // The starting number for media segments, usually 1.
        unsigned int start_number = 1;

        // The duration of a single media segment in seconds.
        // This is calculated from the timescale and duration attributes in the MPD.
        double segment_duration_seconds = 0.0;
    };
    /**
     * @brief Contains all relevant information parsed from an MPD file.
     */
    struct MpdInfo
    {
        // The total duration of the media presentation in seconds.
        // Parsed from <MPD mediaPresentationDuration="...">.
        double media_presentation_duration_seconds = 0.0;

        // A list of all video Representations found in the MPD file.
        std::vector<Representation> representations;
    };
    /**
     * @brief A simple, string-based parser for MPEG-DASH MPD files.
     *
     * Note: This is not a full-featured XML parser. It extracts required information
     * using string searching and basic parsing logic, suitable for MPD files with
     * a relatively fixed structure.
     */
    class MpdParser
    {
    public:
        explicit MpdParser(const std::string &mpdXML);
        std::vector<Representation> getRepresentations() const;
        // MpdInfo getMpdInfo() const;

    private:
        std::vector<Representation> representations_;
        void parse(const std::string &mpdXML);
    };
}
