#include "../include/mime_type.hpp"
#include <unordered_map>

std::string mime_type_to_string(MimeType mime) {
    static const std::unordered_map<MimeType, std::string> mime_map = {
        {MimeType::TextPlain, "text/plain"},
        {MimeType::TextHtml, "text/html"},
        {MimeType::TextCss, "text/css"},
        {MimeType::TextJavascript, "text/javascript"},
        {MimeType::ApplicationJson, "application/json"},
        {MimeType::ApplicationXml, "application/xml"},
        {MimeType::ApplicationPdf, "application/pdf"},
        {MimeType::ImageJpeg, "image/jpeg"},
        {MimeType::ImagePng, "image/png"},
        {MimeType::ImageGif, "image/gif"},
        {MimeType::AudioMpeg, "audio/mpeg"},
        {MimeType::VideoMp4, "video/mp4"},
        {MimeType::MultipartFormData, "multipart/form-data"},
        {MimeType::ApplicationOctetStream, "application/octet-stream"}
    };

    auto it = mime_map.find(mime);
    if (it != mime_map.end()) {
        return it->second;
    }
    return "application/octet-stream";
}
