#pragma once

#include <vector>
#include <map>
#include <string>
#include <dirent.h>

struct FolderNode {
    enum {FOLDER, FILE} type;
    std::string name;
    std::string path;
    ino_t inode_number = 0;
};

class Folder {
public:
    static Folder *makeNewPlaylist();
    static Folder *makeFromHomeDirectory();
    static Folder *makeFromPath(std::string path);
    
    Folder *getSelectedFolder();
    Folder *getParentFolder();
    int getSelectedLine();
    void setSelectedLine(int n);
    void moveSelectedLine(int n);
    const FolderNode& getSelectedNode();
    
    std::vector<FolderNode> nodes;
private:
    static std::map<std::string, Folder> cache;
    
    int selected_line = 0;
    std::string path;
};
