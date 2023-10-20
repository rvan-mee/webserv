#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

// Function to parse multipart/form-data request
bool parseMultipartFormData(const std::string& requestData, std::string& boundary, std::unordered_map<std::string, std::string>& fields) {
    // Find the boundary string
    size_t boundaryPos = requestData.find("boundary=");
    if (boundaryPos == std::string::npos) {
        return false;
    }
    boundary = "--" + requestData.substr(boundaryPos + 9);  // 9 is the length of "boundary="

    // Split the request into parts using the boundary
    std::vector<std::string> parts;
    size_t start = requestData.find(boundary);
    size_t end = requestData.find(boundary, start + 1);
    while (start != std::string::npos && end != std::string::npos) {
        parts.push_back(requestData.substr(start, end - start));
        start = requestData.find(boundary, end);
        end = requestData.find(boundary, start + 1);
    }

    // Extract fields from the parts
    for (const std::string& part : parts) {
        size_t contentDispositionPos = part.find("Content-Disposition: form-data; name=");
        if (contentDispositionPos != std::string::npos) {
            size_t fieldNameStart = part.find("\"", contentDispositionPos) + 1;
            size_t fieldNameEnd = part.find("\"", fieldNameStart);
            std::string fieldName = part.substr(fieldNameStart, fieldNameEnd - fieldNameStart);

            size_t contentStart = part.find("\r\n\r\n") + 4;
            std::string fieldValue = part.substr(contentStart);

            fields[fieldName] = fieldValue;
        }
    }

    return true;
}

std::string generatePythonInput(const std::string& httpRequest, const std::string& requestData) {
    std::string boundary;
    std::unordered_map<std::string, std::string> fields;

    if (!parseMultipartFormData(requestData, boundary, fields)) {
        std::cerr << "Failed to parse multipart/form-data." << std::endl;
        return "";
    }

    // Construct the full HTTP request
    std::stringstream pythonInput;
    pythonInput << httpRequest;
    pythonInput << "\r\n";  // Empty line separates headers from the body

    // Construct the request data body with the boundary and field data
    pythonInput << boundary << "\r\n";
    for (const auto& field : fields) {
        pythonInput << "Content-Disposition: form-data; name=\"" << field.first << "\"\r\n\r\n";
        pythonInput << field.second << "\r\n";
        pythonInput << boundary << "\r\n";
    }
    pythonInput << "--\r\n";  // Final boundary to indicate the end

    return pythonInput.str();
}

int main() {
    // Example HTTP request and request data
    std::string requestData = "POST /upload.py HTTP/1.1\r\nHost: localhost:8070\r\nContent-Type: multipart/form-data; boundary=---------------------------3081233490862001734699800631\r\nContent-Length: 220\r\n\r\n-----------------------------3081233490862001734699800631\r\nContent-Disposition: form-data; name=\"myFile\"; filename=\"hoi.txt\"\r\nContent-Type: text/plain\r\n\r\nhoi\r\n-----------------------------3081233490862001734699800631--";

    std::string pythonInput = generatePythonInput(requestData);
    std::cout << pythonInput;
    return 0;
}
