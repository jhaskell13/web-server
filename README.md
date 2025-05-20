# Simple C HTTP Web Server

A lightweight HTTP web server implemented in C from scratch, without third-party libraries.  
Designed to demonstrate low-level socket programming, HTTP protocol handling, static file serving, directory listing, templating, concurrency with thread pools, and basic security.

---

## Features

- Serve static files (HTML, CSS, JS, images, binary files)
- MIME type detection for proper `Content-Type` headers
- Directory listing with clickable links
- Simple routing and templating with placeholders (e.g. `{{title}}`, `{{content}}`)
- Basic security: prevents directory traversal (`..`) attacks
- HTTP/1.1 persistent connections with `Keep-Alive`
- Concurrent request handling using a configurable thread pool
- Compatible with Linux/Unix and Windows (via Visual Studio or CMake)
- Minimal dependencies, uses only standard C and POSIX APIs

---

## Project Structure

```
Root/
├── static/            # Static assets (images, JS, CSS)
├── templates/         # HTML templates
├── build/             # Build outputs (optional)
├── CMakeLists.txt     # Build configuration
├── README.md
└── .gitignore
```

---

## Requirements

- C compiler (gcc, clang, or Visual Studio)
- POSIX-compliant OS (Linux, macOS) or Windows with Visual Studio
- CMake (optional, recommended for build management)
- Ninja or Make (optional)

---

## Build & Run

### Using CMake + Ninja (Linux/macOS)

```bash
mkdir build
cmake -G Ninja -B build
cd build
ninja
./serve
```

### Using CMake + Visual Studio (Windows)

1. Open `CMakeLists.txt` folder in Visual Studio.
2. Configure and build.
3. Run `serve.exe`.

### Without CMake (gcc CLI)

```bash
gcc ./*.c -pthread -o serve
./serve
```

---

- Access via browser or `curl`:

```
http://localhost:8080/
```

- Static files served from `./static/`
- Templates served from `./templates/`
- Supports directory listing when browsing directories
- Supports persistent HTTP connections and concurrent clients

---

## Security

- Prevents directory traversal attacks
- Properly sets MIME types
- Limits concurrent threads with a thread pool

---

## Extending the Server

Possible enhancements:

- Support POST and other HTTP methods
- Implement HTTPS with TLS (OpenSSL integration)
- Add logging and metrics
- Support for caching static files
- URL parameter parsing and dynamic content generation

---

## License

MIT License — See `LICENSE` file for details.

---

## Author

Your Name — [j,e,haskell1998@gmail.com](mailto:j.e.haskell1998@gmail.com)

---

Feel free to contribute or report issues!
