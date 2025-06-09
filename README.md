# DASH Adaptive Bitrate (ABR) Proxy

## Overview

This project implements an HTTP proxy that supports **DASH (Dynamic Adaptive Streaming over HTTP)** and **adaptive bitrate (ABR) selection**. The proxy intercepts client requests for DASH media, parses `.mpd` manifest files, and dynamically selects the most appropriate video representation based on real-time bandwidth estimation. This ensures smooth and efficient video streaming experiences.

---

## Features

* **DASH Manifest Parsing**: Extracts available video representations (bitrate, resolution, codecs) from `.mpd` files.
* **Adaptive Bitrate (ABR) Selection**: Dynamically selects the optimal video representation based on real-time network conditions.
* **Sliding Window Bandwidth Estimation**: Calculates available bandwidth using recent segment download speeds.
* **Transparent Proxying**: Forwards non-DASH HTTP requests as a standard proxy.
* **Robust Logging**: Maintains `access.log` (request records) and `error.log` (error events).
* **Graceful Error Handling**: Handles network, parsing, and segment errors gracefully.

---

## Quick Start

### 1. Build the Proxy

```bash
mkdir build && cd build
cmake ..
make
```

### 2. Prepare Origin Server Content

* Place your `manifest.mpd` and video segment files in a directory (see [Testing](#testing)).
* Start a simple HTTP server in that directory:

```bash
cd /path/to/video_test_data
python3 -m http.server 8000
```

### 3. Run the Proxy

```bash
./mini_cdn
```

By default, the proxy listens on port `8080`.

---

## Testing

**Fetch manifest and a video segment via the proxy:**

```bash
curl -x http://localhost:8080 http://localhost:8000/manifest.mpd
curl -x http://localhost:8080 http://localhost:8000/video_240p/chunk-1.m4s
```

* **Logs:**

  * `access.log`: Records every client request (timestamp, path, status, bytes sent).
  * `error.log`: Records proxy errors and exceptions.

---

## Key Implementation Details

* **Bandwidth Estimation:**
  For every segment request, the proxy measures segment size and download duration to compute the bandwidth (in kbps), then uses a sliding window average to smooth out fluctuations.
* **Representation Selection:**
  The proxy selects the highest representation whose bitrate does not exceed the current estimated bandwidth.

---

## File Structure

* `src/` — Main proxy, DASH, and networking logic
* `include/` — Header files and data structures
* `access.log`, `error.log` — Runtime logs
* `video_test_data/` — Example DASH content for local testing

---

## Notes

* This project is intended for research/educational purposes and is **not production-hardened**.
* The proxy can be extended to support more advanced ABR logic, audio, or HTTPS.

---

## License

MIT License (or specify your own license).

