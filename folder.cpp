#include "folder.hpp"

#include <cstdio>
#include <stdexcept>
#include <utility>
#include <regex>

std::map<std::string, Folder> Folder::cache;

static std::string getHomeDirectoryPath()
{
    char *home = getenv("HOME");
    if (!home)
        return "/";
    return home;
}

Folder *Folder::makeFromHomeDirectory()
{
    return makeFromPath(getHomeDirectoryPath());
}


std::vector<FolderNode> readDirectory(const std::string& path);

Folder *Folder::makeFromPath(std::string path)
{
    if (path.length() != 1 && path[path.length() - 1] == '/')
        path.pop_back();
    
    try {
        return &cache.at(path);
    } catch (std::out_of_range& error) {
        cache.insert(make_pair(path, Folder()));
        Folder& newtree = cache[path];
        
        newtree.path = path;
        newtree.nodes = readDirectory(path);
        
        return &newtree;
    }
}

Folder *Folder::makeNewPlaylist()
{
    static int n_playlists = 0;
    n_playlists++;
    std::string playlist_path = "omp" + std::to_string(n_playlists) + ":";
    cache.insert(make_pair(playlist_path, Folder()));
    Folder& newtree = cache[playlist_path];
    newtree.path = playlist_path;
    return &newtree;
}

Folder *Folder::getSelectedFolder()
{
    if (nodes.size() == 0)
        throw std::runtime_error("No items in the current folder\n");
    if (nodes[selected_line].type != FolderNode::FOLDER)
        throw std::runtime_error("Selected item is not a folder\n");
    return makeFromPath(nodes[selected_line].path);
}

Folder *Folder::getParentFolder()
{
    if (std::regex_match(path, std::regex("omp[0-9]+:/?")) ||
        std::regex_match(path, std::regex("/?")))
        return this;
    std::string parent_path = path;
    while (parent_path[parent_path.length() - 1] != '/')
        parent_path.pop_back();
    return makeFromPath(parent_path);
}

int Folder::getSelectedLine()
{
    return selected_line;
}

void Folder::setSelectedLine(int n)
{
    selected_line = n % nodes.size();
}

void Folder::moveSelectedLine(int n)
{
    if (nodes.size() == 0)
        return;
    selected_line += n;
    while (selected_line < 0)
        selected_line += nodes.size();
    selected_line %= nodes.size();
}

const FolderNode& Folder::getSelectedNode()
{
    if (nodes.size() == 0)
        throw std::runtime_error("No items in the current folder\n");
    return nodes[selected_line];
}
