#include "folder.hpp"

#include <stdexcept>
#include <algorithm>

#include <sys/types.h>
#include <unistd.h>

static int getTrackNumber(const std::string& filename)
{
    // TODO: if song metadata is available, use it instead
    
    int x = 0;
    
    for (int i = 0; i < filename.length() && filename[i] >= '0' && filename[i] <= '9'; i++)
        x = x * 10 + (filename[i] - '0'); 
    
    return x;
}

std::vector<FolderNode> readDirectory(const std::string& path)
{
    DIR *dir = opendir(path.c_str());
    if (!dir)
        throw std::runtime_error("Couldn't open directory: " + path + "\n");
    
    dirent *entry;
    std::vector<FolderNode> nodes;
    
    while (entry = readdir(dir)) {
        if (entry->d_name[0] == '.')
            continue;
        
        FolderNode node;
        
        if (entry->d_type == DT_REG)
            node.type = FolderNode::FILE;
        else if (entry->d_type == DT_DIR)
            node.type = FolderNode::FOLDER;
        else
            continue;
        
        node.name = std::string(entry->d_name);
        node.inode_number = entry->d_ino;
        if (path[path.length() - 1] == '/')
            node.path = path + node.name;
        else 
            node.path = path + "/" + node.name;
        nodes.push_back(node);
    }
    
    closedir(dir);
    
    std::sort(nodes.begin(), nodes.end(),
              [](const FolderNode& a, const FolderNode& b) -> bool {
                  if (a.type != b.type)
                      return a.type == FolderNode::FOLDER;
                  int anum = getTrackNumber(a.name);
                  int bnum = getTrackNumber(b.name);
                  if (anum != bnum)
                      return anum < bnum;
                  std::string a_lower_name, b_lower_name;
                  for (const auto& c : a.name)
                      a_lower_name.push_back(std::tolower(c));
                  for (const auto& c : b.name)
                      b_lower_name.push_back(std::tolower(c));
                  return a_lower_name.compare(b_lower_name) < 0;
              });
    
    return nodes;
}
