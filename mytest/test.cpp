#include <iostream>
#include <sstream>
#include <vector>
#include <string>

std::string extractContent(const std::string &inputText, const std::string &boundary) {
  std::istringstream input(inputText);
    std::string line;
    std::ostringstream extractedContent;
    bool contentStarted = false;

    while (std::getline(input, line)) {
        // std::cout << inputText << std::endl;
        if (line.find("Content-Type:") != std::string::npos)
        {

            while (std::getline(input, line))
            {
                // std::cout << line << std::endl;
                if (line.find(boundary) != std::string::npos)
                  break;
                extractedContent << line << "\n";
            }
        }

    }
    return extractedContent.str();
}

int main() {
    std::string inputText = "------WebKitFormBoundarycb6NKgEtm2UHu2lB\nContent-Disposition: form-data; name=\"myFile\"; filename=\"test.txt\"\nContent-Type: text/plain\nTest\n\n------WebKitFormBoundarycb6NKgEtm2UHu2lB--";

    std::string boundary = "------";
    std::string content = extractContent(inputText, boundary);

    if (!content.empty()) {
        std::cout << content << std::endl;
    } else {
        std::cout << "Content not found." << std::endl;
    }

    return 0;
}
