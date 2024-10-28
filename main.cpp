#include <iostream>
#include <windows.h>
#include <string>
#include <filesystem>

void copy_directory(const std::string& source, const std::string& destination)
{
    // Create the directory only if it doesn't already exist.
    if (!std::filesystem::exists(destination))
        std::filesystem::create_directories(destination);

    WIN32_FIND_DATA find_file_data;
    HANDLE find_handle = FindFirstFileA((source + "\\*").c_str(), &find_file_data);

    if (!find_handle)
        return;

    do
    {
        const std::string file_name = find_file_data.cFileName;

        // Skip the current directory (.) and the parent directory (..)
        if (file_name != "." && file_name != "..")
        {
            std::string source_path = source + "\\" + file_name;
            std::string dest_path = destination + "\\" + file_name;

            if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // Recursively copy the subdirectory.
                copy_directory(source_path, dest_path);
            }
            else
            {
                // Copy the file(s).
                CopyFileA(source_path.c_str(), dest_path.c_str(), FALSE);
            }
        }
    } while (FindNextFileA(find_handle, &find_file_data) != 0);

    // Cleanup.
    FindClose(find_handle);
}

void find_and_copy_sln_files(const std::string& directory, const std::string& dest_root)
{
    WIN32_FIND_DATA find_file_data;
    HANDLE find_handle = FindFirstFileA((directory + "\\*").c_str(), &find_file_data);

    if (!find_handle)
        return;

    do
    {
        const std::string file_name = find_file_data.cFileName;

        // Check if it's a directory.
        if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // Skip the current directory (.) and the parent directory (..)
            if (file_name != "." && file_name != "..")
            {
                // Recursively search in the subdirectory.
                find_and_copy_sln_files(directory + "\\" + file_name, dest_root);
            }
        }
        else
        {
            // Check if the file is a .sln or .vcxproj file
            if ((file_name.size() > 4 && file_name.substr(file_name.size() - 4) == ".sln") ||
                (file_name.size() > 8 && file_name.substr(file_name.size() - 8) == ".vcxproj"))
            {
                // Use the current directory to get the project folder name
                std::string project_folder_name = directory.substr(directory.find_last_of('\\') + 1);
                std::string dest_path = dest_root + "\\" + project_folder_name;

                // Ignore directories that are Microsoft projects
                if (directory.find("(x86)\\Microsoft Visual Studio") == std::string::npos)
                {
                    // Check if the current directory is not within the copied directory
                    if (directory.find(dest_root) == std::string::npos)
                    {
                        // Copy the entire directory containing the .sln or .vcxproj file
                        printf("[+] Copied %s\\%s\n", directory.c_str(), file_name.c_str());
                        copy_directory(directory, dest_path);
                    }
                }
            }
        }
    } while (FindNextFileA(find_handle, &find_file_data) != 0);

    // Cleanup.
    FindClose(find_handle);
}

bool is_alpha_char(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

int main()
{
    // Directory where we will copy all copied directories into.
    static const std::string stored_location = "C:\\Copied";

    // Get all drive letters.
    char buf[MAX_PATH];
    GetLogicalDriveStringsA(sizeof(buf), buf);

    for (int i = 0; i < sizeof(buf); i++)
    {
        // Only interested in drive letters.
        if (is_alpha_char(buf[i]))
        {
            std::string drive = "";
            drive += buf[i];
            drive += ":\\";

            // Search drive.
            find_and_copy_sln_files(drive, stored_location);
        }
    }

    return 0;
}
