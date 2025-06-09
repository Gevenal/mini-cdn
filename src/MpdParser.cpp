#include "../include/proxy/MpdParser.hpp"
#include "tinyxml2.h"
#include <stdexcept>
#include <iostream>

using namespace tinyxml2;

namespace proxy
{

    // Constructor: parses the MPD XML upon creation.
    MpdParser::MpdParser(const std::string &mpdXML)
    {
        parse(mpdXML);
    }

    // Return all parsed representations.
    std::vector<Representation> MpdParser::getRepresentations() const
    {
        return representations_;
    }

    // Internal parse function.
    void MpdParser::parse(const std::string &mpdXML)
    {
        representations_.clear();

        XMLDocument doc;
        XMLError err = doc.Parse(mpdXML.c_str());
        if (err != XML_SUCCESS)
        {
            throw std::runtime_error("Failed to parse MPD XML: " + std::string(doc.ErrorStr()));
        }

        // Find the root MPD node
        XMLElement *mpd = doc.FirstChildElement("MPD");
        if (!mpd)
            throw std::runtime_error("MPD tag not found.");

        // Only handle the first Period for simplicity
        XMLElement *period = mpd->FirstChildElement("Period");
        if (!period)
            throw std::runtime_error("Period tag not found.");

        // Only handle the first AdaptationSet for simplicity
        XMLElement *adaptationSet = period->FirstChildElement("AdaptationSet");
        if (!adaptationSet)
            throw std::runtime_error("AdaptationSet tag not found.");

        // Extract global SegmentTemplate if any (for fallback)
        XMLElement *setSegTpl = adaptationSet->FirstChildElement("SegmentTemplate");
        std::string set_media, set_init;
        unsigned int set_start_number = 1;
        unsigned int set_timescale = 1;
        unsigned int set_duration = 0;
        if (setSegTpl)
        {
            if (setSegTpl->Attribute("media"))
                set_media = setSegTpl->Attribute("media");
            if (setSegTpl->Attribute("initialization"))
                set_init = setSegTpl->Attribute("initialization");
            setSegTpl->QueryUnsignedAttribute("startNumber", &set_start_number);
            setSegTpl->QueryUnsignedAttribute("timescale", &set_timescale);
            setSegTpl->QueryUnsignedAttribute("duration", &set_duration);
        }

        // Loop through all <Representation> tags
        for (XMLElement *repElem = adaptationSet->FirstChildElement("Representation");
             repElem; repElem = repElem->NextSiblingElement("Representation"))
        {

            Representation rep;
            // Parse basic attributes
            if (repElem->Attribute("id"))
                rep.id = repElem->Attribute("id");
            repElem->QueryUnsignedAttribute("bandwidth", &rep.bandwidth);
            repElem->QueryUnsignedAttribute("width", &rep.width);
            repElem->QueryUnsignedAttribute("height", &rep.height);
            if (repElem->Attribute("codecs"))
                rep.codecs = repElem->Attribute("codecs");

            // SegmentTemplate can be on Representation or AdaptationSet
            XMLElement *segTpl = repElem->FirstChildElement("SegmentTemplate");
            if (!segTpl)
                segTpl = setSegTpl; // fallback to AdaptationSet

            // Parse SegmentTemplate info
            if (segTpl)
            {
                if (segTpl->Attribute("media"))
                    rep.media_template_url = segTpl->Attribute("media");
                else
                    rep.media_template_url = set_media;
                if (segTpl->Attribute("initialization"))
                    rep.init_template_url = segTpl->Attribute("initialization");
                else
                    rep.init_template_url = set_init;
                segTpl->QueryUnsignedAttribute("startNumber", &rep.start_number);
                unsigned int timescale = 1, duration = 0;
                segTpl->QueryUnsignedAttribute("timescale", &timescale);
                segTpl->QueryUnsignedAttribute("duration", &duration);
                // Calculate segment duration (seconds)
                if (timescale && duration)
                    rep.segment_duration_seconds = static_cast<double>(duration) / timescale;
                else if (set_timescale && set_duration)
                    rep.segment_duration_seconds = static_cast<double>(set_duration) / set_timescale;
            }

            representations_.push_back(rep);
        }
    }

} // namespace proxy
