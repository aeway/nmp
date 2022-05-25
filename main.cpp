#include "audio_player.hpp"
#include "cursed.hpp"
#include "folder.hpp"

constexpr int SHIFT_SKIP = 5;

Folder *file_system_folder = Folder::makeFromHomeDirectory();
Folder *playlist_folder = Folder::makeNewPlaylist();
Folder **selected_folder = &file_system_folder;

void drawScreen();
void fillPlaylistFromFolder(Folder *folder);

static int playing_song_idx = 0;
void finishedPlayingCallback()
{
    // play next song in the playlist
    extern AudioPlayer player;
    playing_song_idx = (playing_song_idx + 1) % playlist_folder->nodes.size();
    player.playFile(playlist_folder->nodes[playing_song_idx]);
}

void timeChangedCallback()
{
    drawScreen();
}

AudioPlayer player(finishedPlayingCallback, timeChangedCallback);

int main()
{    
    START_NCURSES;
        
    while (true) {
        try {
            drawScreen();
            int ch = getch();
            switch (ch) {
            case 'g':
                (*selected_folder)->setSelectedLine(0);
                break;
            case 'j':
                (*selected_folder)->moveSelectedLine(1);
                break;
            case 'k':
                (*selected_folder)->moveSelectedLine(-1);
                break;
            case 'J':
                (*selected_folder)->moveSelectedLine(SHIFT_SKIP);
                break;
            case 'K':
                (*selected_folder)->moveSelectedLine(-1 * SHIFT_SKIP);
                break;
            case 'h':
                *selected_folder = (*selected_folder)->getParentFolder();
                break;
            case 'l':
                if ((*selected_folder)->nodes.size() == 0)
                    break;
                if ((*selected_folder)->getSelectedNode().type == FolderNode::FOLDER)
                    *selected_folder = (*selected_folder)->getSelectedFolder();
                else if (player.isPlayingNode((*selected_folder)->getSelectedNode()))
                    player.togglePause();
                else {
                    player.playFile((*selected_folder)->getSelectedNode());
                    
                    if (selected_folder == &file_system_folder)
                        fillPlaylistFromFolder(file_system_folder);                   
                }
                break;
            case ' ':
                player.togglePause();
                break;
            case 'p':
            case '\t':
                if (selected_folder == &file_system_folder)
                    selected_folder = &playlist_folder;
                else
                    selected_folder = &file_system_folder;
                break;
            }
        } catch (std::runtime_error& e) {
            erase();
            refresh();
            mvprintw(0, 0, "%s", e.what());
            refresh();
            getch();
        }
    }
    
    return 0;
}

void fillPlaylistFromFolder(Folder *folder)
{
    if (folder == playlist_folder)
        return;
    playlist_folder->nodes.clear();
    for (int i = 0; i < folder->nodes.size(); i++)
        if (folder->nodes[i].type == FolderNode::FILE) {
            playlist_folder->nodes.push_back(folder->nodes[i]);
            if (player.isPlayingNode(folder->nodes[i])) {
                playlist_folder->setSelectedLine(i);
                playing_song_idx = i;
            }
        }
}
