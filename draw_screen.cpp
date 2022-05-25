#include "audio_player.hpp"
#include "cursed.hpp"
#include "folder.hpp"

#include <mutex>

extern Folder *file_system_folder;
extern Folder *playlist_folder;
extern Folder **selected_folder;

extern AudioPlayer player;


void drawFolder(Folder *folder, int start_y, int max_y)
{
    int n_lines = max_y - start_y;
    int startnode = 0, endnode = 0;
    
    if (folder->nodes.size() > n_lines) {
        if (folder->getSelectedLine() > n_lines / 2)
            startnode = folder->getSelectedLine() - n_lines / 2;
        if (startnode > folder->nodes.size() - n_lines)
            startnode = folder->nodes.size() - n_lines;
        endnode = startnode + n_lines - 1;
    } else
	endnode = folder->nodes.size() - 1;
    
    for (int i = startnode; i <= endnode; i++) {
        if (i == folder->getSelectedLine())
            attron(A_REVERSE);
        if (folder->nodes[i].type == FolderNode::FOLDER)
            attron(COLOR_PAIR(DIRECTORY_CP));
        if (player.isPlayingNode(folder->nodes[i]))
            attron(COLOR_PAIR(PLAYING_CP));
        
        mvprintw(start_y + i-startnode, 0, "%s", folder->nodes[i].name.c_str());

        attroff(COLOR_PAIR(PLAYING_CP));
        attroff(COLOR_PAIR(DIRECTORY_CP));
        attroff(A_REVERSE);
    }
}

void drawScreen()
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> guard(mutex);

    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);

    clear();
    refresh();

    if (selected_folder == &playlist_folder) {
        std::string title = "Playlist:";
        title.resize(max_x, ' ');

        attron(A_REVERSE);
        mvprintw(0, 0, title.c_str());
        attroff(A_REVERSE);
        
        drawFolder(*selected_folder, 1, max_y);
    } else {
        drawFolder(*selected_folder, 0, max_y);
    }

    if (player.isPlaying()) {
        unsigned int seconds = player.getPlayingTime_MS() / 1000;
        mvprintw(max_y - 1, max_x - 5, "%d:%02d", seconds / 60, seconds % 60);
    }
    
    refresh();
}

