#pragma once

#include <string>

enum class MimeType {
    TextPlain,          // text/plain
    TextHtml,           // text/html
    TextCss,            // text/css
    TextJavascript,     // text/javascript
    ApplicationJson,    // application/json
    ApplicationXml,     // application/xml
    ApplicationPdf,     // application/pdf
    ImageJpeg,          // image/jpeg
    ImagePng,           // image/png
    ImageGif,           // image/gif
    AudioMpeg,          // audio/mpeg
    VideoMp4,           // video/mp4
    MultipartFormData,  // multipart/form-data
    ApplicationOctetStream // application/octet-stream
};

std::string mime_type_to_string(MimeType mime);
