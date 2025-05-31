#include "../include/proxy/HttpParser.hpp"
#include <sstream> // For std::istringstream
#include <string>
#include <vector>
#include <algorithm>
// Simple helper to trim leading/trailing whitespace from a string
namespace
{
    static std::string trim_whitespace(const std::string &s)
    {
        const std::string WHITESPACE = " \t\n\r\f\v";
        ssize_t first = s.find_first_not_of(WHITESPACE);
        if (std::string::npos == first)
        {
            return s; // No non-whitespace content
        }
        ssize_t last = s.find_last_not_of(WHITESPACE);
        return s.substr(first, (last - first + 1));
    }
}
namespace proxy
{

    HttpRequest HttpParser::parse(const std::string &raw_request)
    {
        HttpRequest request; // Initialize with default port 80
        std::istringstream request_stream(raw_request);
        std::string line;

        // 1. Parse Request Line: METHOD target HTTP-Version
        if (!std::getline(request_stream, line) || line.empty())
        {
            // Invalid: No request line or empty request
            return HttpRequest{}; // Return default-constructed (empty) request
        }

        if (line.back() == '\r')
        { // Remove trailing CR if present
            line.pop_back();
        }

        std::istringstream request_line_stream(line);
        std::string http_version; // We'll read it but not store it in your current HttpRequest struct

        request_line_stream >> request.method;
        request_line_stream >> request.url; // This is the request-target
        request_line_stream >> http_version;

        if (request.method.empty() || request.url.empty() || http_version.empty())
        {
            // Invalid request line format
            return HttpRequest{};
        }

        // Default path to the request-target (URL) for now.
        // This will be refined if the URL is absolute.
        request.path = request.url;

        // 2. Parse Headers (mainly looking for "Host")
        std::string host_header_value;

        while (std::getline(request_stream, line))
        {
            if (!line.empty() && line.back() == '\r')
            { // Remove trailing CR
                line.pop_back();
            }
            if (line.empty())
            { // Blank line indicates end of headers
                break;
            }

            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos)
            {
                std::string header_name = line.substr(0, colon_pos);
                std::string header_value = trim_whitespace(line.substr(colon_pos + 1));

                // Case-insensitive comparison for "Host"
                std::string temp_header_name = header_name;
                std::transform(temp_header_name.begin(), temp_header_name.end(), temp_header_name.begin(), ::tolower);

                if (temp_header_name == "host")
                {
                    host_header_value = header_value;
                }
                // Note: Your HttpRequest struct doesn't store other headers.
                // If it did, you'd add them here.
            }
        }

        // 3. Determine actual host, port, and path
        // Check if the request.url (request-target) is an absolute URL
        bool is_absolute_url = false;
        std::string scheme;

        if (request.url.rfind("http://", 0) == 0)
        {
            is_absolute_url = true;
            scheme = "http";
        }
        else if (request.url.rfind("https://", 0) == 0)
        {
            is_absolute_url = true;
            scheme = "https";
            request.port = 443; // Default port for HTTPS
        }

        if (is_absolute_url)
        {
            // Parse host, port, path from the absolute URL
            // Example: http://example.com:8080/path/to/resource
            size_t scheme_separator_pos = request.url.find("://");
            std::string url_no_scheme = request.url.substr(scheme_separator_pos + 3); // "example.com:8080/path/to/resource"

            size_t path_start_pos = url_no_scheme.find('/');
            std::string host_port_part;

            if (path_start_pos != std::string::npos)
            {
                host_port_part = url_no_scheme.substr(0, path_start_pos); // "example.com:8080"
                request.path = url_no_scheme.substr(path_start_pos);      // "/path/to/resource"
            }
            else
            {
                // No path in URL, e.g., http://example.com
                host_port_part = url_no_scheme;
                request.path = "/"; // Default path
            }

            size_t port_colon_pos_in_url = host_port_part.find(':');
            if (port_colon_pos_in_url != std::string::npos)
            {
                request.host = host_port_part.substr(0, port_colon_pos_in_url);
                try
                {
                    request.port = static_cast<unsigned short>(std::stoul(host_port_part.substr(port_colon_pos_in_url + 1)));
                }
                catch (const std::exception &e)
                {
                    // Invalid port in URL, keep default (80 for http, 443 for https)
                }
            }
            else
            {
                request.host = host_port_part;
                // Port not in URL, it's already defaulted (80 or 443 based on scheme)
            }
        }
        else
        {
            // URL is not absolute (e.g., "/path/page.html"), so path is already set correctly.
            // Host and port must come from the "Host" header.
            if (!host_header_value.empty())
            {
                size_t port_colon_pos_in_host = host_header_value.find(':');
                if (port_colon_pos_in_host != std::string::npos)
                {
                    request.host = host_header_value.substr(0, port_colon_pos_in_host);
                    try
                    {
                        request.port = static_cast<unsigned short>(std::stoul(host_header_value.substr(port_colon_pos_in_host + 1)));
                    }
                    catch (const std::exception &e)
                    {
                        // Invalid port in Host header, keep default (80)
                    }
                }
                else
                {
                    request.host = host_header_value;
                    // request.port remains its default (80)
                }
            }
            else
            {
                // No Host header and URL is not absolute. This is an error for HTTP/1.1.
                // For this simple parser, request.host will be empty.
                // A more robust parser would flag this as an error.
            }
        }

        // Ensure path is at least "/" if it was meant to be a path from an absolute URL with no explicit path
        if (is_absolute_url && request.path.empty())
        {
            request.path = "/";
        }

        return request;
    }

    std::string HttpParser::serialize(const HttpRequest &req)
    {
        std::ostringstream out;
        out << req.method << " " << req.path << " " << req.http_version << "\r\n";
        for (const auto &kv : req.headers)
        {
            out << kv.first << ": " << kv.second << "\r\n";
        }
        out << "\r\n"; // End of header
        return out.str();
    }

}