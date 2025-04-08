#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

struct Song {
    std::string name;
    std::string location;
    // You can add other fields as needed
};

// Helper function to trim whitespace
std::string trim(const std::string& str) {
    if (str.empty()) {
        return str;
    }
    
    // Find the first non-whitespace character
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) {
        return ""; // String contains only whitespace
    }
    
    // Find the last non-whitespace character
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    
    // Return the trimmed substring
    return str.substr(first, last - first + 1);
}

// Function to read UTF16-LE file and convert to UTF-8
std::string readUtf16LEFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "";
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read entire file into a buffer
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Failed to read file: " << filePath << std::endl;
        return "";
    }
    
    // Skip UTF-16LE BOM if present
    size_t pos = 0;
    if (size >= 2 && static_cast<unsigned char>(buffer[0]) == 0xFF && static_cast<unsigned char>(buffer[1]) == 0xFE) {
        std::cout << "UTF-16LE BOM detected, skipping BOM" << std::endl;
        pos = 2;
    }
    
    // Convert UTF-16LE to UTF-8
    std::string result;
    for (; pos + 1 < buffer.size(); pos += 2) {
        // Extract UTF-16LE code point
        char16_t ch = static_cast<unsigned char>(buffer[pos]) | (static_cast<unsigned char>(buffer[pos + 1]) << 8);
        
        // Simple ASCII characters can be directly converted
        if (ch < 0x80) {
            result.push_back(static_cast<char>(ch));
        } else if (ch < 0x800) {
            // 2-byte UTF-8 encoding
            result.push_back(static_cast<char>(0xC0 | (ch >> 6)));
            result.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
        } else {
            // 3-byte UTF-8 encoding
            result.push_back(static_cast<char>(0xE0 | (ch >> 12)));
            result.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
        }
    }
    
    return result;
}

// Function to split a string by a delimiter
std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    
    return tokens;
}

std::vector<Song> parsePlaylistFile(const std::string& filePath) {
    std::vector<Song> songs;
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filePath << std::endl;
        return songs;
    }

    // Check for BOM
    unsigned char bom[3] = {0};
    file.read(reinterpret_cast<char*>(bom), 3);
    bool isUtf16LE = false;
    file.close();

    if (bom[0] == 0xFF && bom[1] == 0xFE) {
        std::cout << "UTF-16LE BOM detected, BOM consumed." << std::endl;
        isUtf16LE = true;
    }

    std::string fileContent;
    if (isUtf16LE) {
        // Handle UTF-16LE files with our custom converter
        fileContent = readUtf16LEFile(filePath);
    } else {
        // Handle UTF-8 or ASCII files
        std::ifstream textFile(filePath);
        if (!textFile) {
            std::cerr << "Failed to open file: " << filePath << std::endl;
            return songs;
        }
        
        std::stringstream buffer;
        buffer << textFile.rdbuf();
        fileContent = buffer.str();
    }

    if (fileContent.empty()) {
        std::cerr << "File is empty or could not be read: " << filePath << std::endl;
        return songs;
    }

    // Split content by lines
    std::vector<std::string> lines;
    std::string line;
    std::istringstream contentStream(fileContent);
    
    while (std::getline(contentStream, line)) {
        lines.push_back(line);
    }

    if (lines.empty()) {
        std::cerr << "No lines found in file: " << filePath << std::endl;
        return songs;
    }

    // Parse headers
    std::vector<std::string> headers = splitString(lines[0], '\t');
    
    // Find indices for name and location
    int nameIndex = -1;
    int locationIndex = -1;
    
    if (nameIndex == -1 || locationIndex == -1) {
        std::cerr << "Required headers 'Name' and/or 'Location' not found in file: " << filePath << std::endl;

        std::cout << "Available headers:" << std::endl;
        for (size_t i = 0; i < headers.size(); i++) {
            std::cout << "\tHeader: \"" << headers[i] << "\"" << std::endl;
        }

        return songs;
    }
    
    // Parse song entries
    for (size_t i = 1; i < lines.size(); i++) {
        std::vector<std::string> fields = splitString(lines[i], '\t');
        
        if (fields.size() > std::max(nameIndex, locationIndex)) {
            Song song;
            song.name = fields[nameIndex];
            song.location = fields[locationIndex];
            songs.push_back(song);
        }
    }

    return songs;
}

// Extract the playlist name from the file name
std::string getPlaylistName(const std::string& filePath) {
    fs::path path(filePath);
    return path.stem().string();
}

// Copy a file from source to destination
bool copySong(const std::string& sourcePath, const std::string& destPath) {
    try {
        if (!fs::exists(sourcePath)) {
            std::cerr << "Source file does not exist: " << sourcePath << std::endl;
            return false;
        }
        
        // Create parent directories if they don't exist
        fs::path destFilePath(destPath);
        fs::create_directories(destFilePath.parent_path());
        
        // Copy the file
        fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error copying file: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " playlist1.txt playlist2.txt ..." << std::endl;
        return 1;
    }
    
    // Process each playlist file
    for (int i = 1; i < argc; i++) {
        std::string filePath = argv[i];
        std::string playlistName = getPlaylistName(filePath);
        
        std::cout << "Processing playlist: " << playlistName << std::endl;
        
        // Create directory for the playlist
        std::string playlistDir = playlistName;
        try {
            fs::create_directories(playlistDir);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
            continue;
        }
        
        // Parse the playlist file
        std::vector<Song> songs = parsePlaylistFile(filePath);
        
        std::cout << "Found " << songs.size() << " songs in playlist" << std::endl;
        
        // Copy each song to the playlist directory
        int successCount = 0;
        for (const auto& song : songs) {
            if (song.location.empty()) {
                std::cerr << "Song location is empty for: " << song.name << std::endl;
                continue;
            }
            
            // Fix file:// prefix if present
            std::string cleanLocation = song.location;
            const std::string filePrefix = "file://";
            if (cleanLocation.substr(0, filePrefix.length()) == filePrefix) {
                cleanLocation = cleanLocation.substr(filePrefix.length());
            }
            
            fs::path sourcePath(cleanLocation);
            
            if (!sourcePath.has_filename()) {
                std::cerr << "Invalid song path: " << cleanLocation << std::endl;
                continue;
            }
            

            // Find the last backslash or forward slash
            size_t lastSlash = cleanLocation.find_last_of("/\\");
            std::string filename = (lastSlash != std::string::npos) ? 
                                cleanLocation.substr(lastSlash + 1) : 
                                cleanLocation;
            std::string destPath = (fs::path(playlistDir) / filename).string();
            
            std::cout << "Copying: " << sourcePath << " to " << destPath << std::endl;
            
            if (copySong(sourcePath.string(), destPath)) {
                successCount++;
            }
        }
        
        std::cout << "Successfully copied " << successCount << " out of " << songs.size() 
                  << " songs for playlist: " << playlistName << std::endl;
    }
    
    return 0;
}