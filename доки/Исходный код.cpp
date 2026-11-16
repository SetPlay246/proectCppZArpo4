#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <random>
#include <algorithm>

#include <clocale>
#include <windows.h>

#include <cstdlib>

#include <commdlg.h>
#pragma comment(lib, "comdlg32.lib")

#include <sstream>
#include <set>

using json = nlohmann::json;
using namespace std;

static string get_filename(const std::string& path) {
	size_t pos = path.find_last_of("/\\");
	if (pos != string::npos) {
		return path.substr(pos + 1);
	}
	return path;
}

class Music {
public:
    string Name;
    string Path;

    Music() = default;

    Music(string path) {
        Path = path;
        Name = get_filename(path);
    }

    string toString() {
        return ("имя: " + Name);
    }

    static friend void to_json(json& j, const Music& m) {
        j = json{
            {"name", m.Name},
            {"path", m.Path}
        };
    }

    static friend void from_json(const json& j, Music& m) {
        j.at("name").get_to(m.Name);
        j.at("path").get_to(m.Path);
    }
};

class Playlist {
public:
    string Name;
    vector<Music> Musiclists;

    Playlist() = default;

    Playlist(string name) {
        Name = name;
    }

    string toString() {
        return ("имя: " + Name + " треков: " + to_string(Musiclists.size()));
    }

    void printmusics() {
        cout << "треки: " << endl;
        if (Musiclists.size() == 0) {
            cout << "нет треков\n";
            return;
        }
        for (int i = 0; i < Musiclists.size(); i++) {
            cout << "id: " << i << " " << Musiclists[i].toString() << endl;
        }
    }

    static friend void to_json(json& j, const Playlist& p) {
        j = json{
            {"name", p.Name},
            {"music_list", p.Musiclists}
        };
    }

    static friend void from_json(const json& j, Playlist& p) {
        j.at("name").get_to(p.Name);
        j.at("music_list").get_to(p.Musiclists);
    }
};

class Group {
public:
    string Name;
    vector<Playlist> Playlists;

    Group() = default;

    Group(string name) {
        Name = name;
    }

    string toString() {
        return ("имя: " + Name + " листов: " + to_string(Playlists.size()));
    }

    void printlists() {
        cout << "листы: \n";
        if (Playlists.size() == 0) {
            cout << "нет листов\n";
            return;
        }
        for (int i = 0; i < Playlists.size(); i++) {
            cout << "id: " << i << " " << Playlists[i].toString() << endl;
        }
    }

    static friend void to_json(json& j, const Group& g) {
        j = json{
            {"name", g.Name},
            {"playlist", g.Playlists}
        };
    }

    static friend void from_json(const json& j, Group& g) {
        j.at("name").get_to(g.Name);
        j.at("playlist").get_to(g.Playlists);
    }
};

static void printMenu(int layer)
{
    switch (layer)
    {
    case 1:
        cout << "уровень программы\n";
        cout << "команды: \n";
        cout << "1 переход в группу \n2 создать группу\n3 играть треки из всех групп\n0 сохранить и выйти\n";
        break;
    case 11:
        cout << "введите id группы чтобы перейти к ней\nвне вариантов - назад\n";
        break;
    case 12:
        cout << "1 по порядку\n2 в случайном порядке\nвне вариантов - назад\n";
        break;
    case 2:
        cout << "команды: \n";
        cout << "1 переход в плейлист \n2 создать плейлист\n3 играть треки из всех плейлистов\n4 удалить эту группу\n5 переименовать\n0 выйти\n";
        break;
    case 21:
        cout << "введите id плейлиста чтобы перейти к нему\nвне вариантов - назад\n";
        break;
    case 3:
        cout << "команды: \n";
        cout << "1 выбор музыки\n2 добавить треки(любое кол-во)\n3 играть треки\n4 удалить этот плейлист\n5 переименовать\n6 выбрать треки для удаления\n0 выйти\n";
        break;
    case 31:
        cout << "введите id трека чтобы перейти к нему\nвне вариантов - назад\n";
        break;
    case 33:
        cout << "введите id треков через ; пример: 1;4;53;2\nпри выполнении удалит выбранные треки\nвне вариантов - назад\n";
        break;
    case 4:
        cout << "1 играть\n2 удалить\n0 выйти\n";
        break;  
    default:
        break;
    }
}

static void clearConsole() {
#if defined(_WIN32) || defined(_WIN64)
    std::system("cls");
#else
    std::system("clear");
#endif
}

static void printGroups(vector<Group> a) {
    int b = a.size();
    cout << "группы: \n";
    if (b == 0) {
        cout << "нет групп\n";
        return;
    }
    
    for (int i = 0; i < b; i++) {
        cout << "id: " << i << " " << a[i].toString() << endl;
    }
}
static string WStringToString1251(const wstring& wstr) {
    if (wstr.empty()) return string();

    int size_needed = WideCharToMultiByte(1251, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    string strTo(size_needed, 0);

    WideCharToMultiByte(1251, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

static void AddMusicToPlaylist(Playlist* playlist) {
    if (playlist == nullptr) return;

    vector<wchar_t> buffer(32767, 0);

    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"аудиофайлы (*.mp3;*.ogg;*.wav)\0*.mp3;*.ogg;*.wav\0Все файлы (*.*)\0*.*\0";
    ofn.lpstrFile = buffer.data();
    ofn.nMaxFile = static_cast<DWORD>(buffer.size());
    ofn.lpstrTitle = L"выберите треки для добавления";
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;

    if (GetOpenFileNameW(&ofn)) {
        wchar_t* p = buffer.data();
        wstring directory = p;
        p += directory.length() + 1;

        auto tryAddTrack = [&](const string& fullPath) {
            bool exists = std::any_of(playlist->Musiclists.begin(), playlist->Musiclists.end(),
                [&](const Music& m) { return m.Path == fullPath; });

            if (exists) {
                cout << "  [пропущен дубликат]: " << get_filename(fullPath) << "\n";
            }
            else {
                playlist->Musiclists.push_back(Music(fullPath));
            }
            };

        if (*p == L'\0') {
            tryAddTrack(WStringToString1251(directory));
        }
        else {
            while (*p != L'\0') {
                wstring filename = p;
                wstring fullPath = directory + L"\\" + filename;

                tryAddTrack(WStringToString1251(fullPath));

                p += filename.length() + 1;
            }
        }
    }
}

static void RemoveTracksFromPlaylist(Playlist* playlist, string input) {
    if (playlist == nullptr || playlist->Musiclists.empty()) {
        cout << "плейлист пуст\n";
        return;
    }

    vector<int> idsToDelete;
    stringstream ss(input);
    string token;

    while (getline(ss, token, ';')) {
        if (token.empty()) continue;
        try {
            int id = stoi(token);
            if (id >= 0 && id < playlist->Musiclists.size()) {
                idsToDelete.push_back(id);
            }
            else {
                cout << "id " << id << " вне диапазона, пропуск\n";
            }
        }
        catch (...) {
            cout << "'" << token << "' не является числом и будет пропущен\n";
        }
    }

    if (idsToDelete.empty()) {
        cout << "удолять нечего (некоректная запись)\n";
        return;
    }

    cout << "вы уверенны что хотите удалить выбранные треки из " << playlist->Name << "?\nвведите y что-бы потдвердить удаление, иной ввод - назад\n";
    string confirm;
    cin >> confirm;

    if (confirm == "y") {
        std::sort(idsToDelete.rbegin(), idsToDelete.rend());

        int deletedCount = 0;
        for (int id : idsToDelete) {
            playlist->Musiclists.erase(playlist->Musiclists.begin() + id);
            deletedCount++;
        }

        cout << "успешне удаление, " << deletedCount << " треков\n";
    }
}


static void PlayTracksInExternalPlayer(const vector<Music>& tracks, bool isrand) {
    if (tracks.empty()) {
        cout << "список треков пуст\n";
        return;
    }

    vector<Music> playList = tracks;

    if (isrand) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(playList.begin(), playList.end(), g);
    }

    string playlistPath = "temp_playlist.m3u";
    ofstream m3uFile(playlistPath);

    if (!m3uFile.is_open()) {
        cout << "не удалось создать файл плейлиста\n";
        return;
    }

    m3uFile << "#EXTM3U\n";
    cout << "\nтреки переданы плееру:\n";

    for (const auto& track : playList) {
        m3uFile << track.Path << "\n";
        cout << "  " << track.Name << "\n";
    }

    m3uFile.close();

    string command = "start \"\" \"" + playlistPath + "\"";
    int result = std::system(command.c_str());

    if (result != 0) {
        cout << "не удалось открыть плеер\n";
    }
    else {
        cout << "плейлист отдан в плеер\n";
    }

    cout << "нажмите Enter чтобы вернуться в меню" << endl;
    cin.ignore();
    cin.get();
}

static void MusicMenu(vector<Music> *Musics, int id) {
    int inInt;
    string inStr;
    bool o = true;
    while (o == true) {
        clearConsole();
        cout << "вы смотрите музыку " << (*Musics)[id].Name << endl;
        cout << "путь к файлу: " << (*Musics)[id].Path << endl;
        printMenu(4);
        cin >> inStr;
        try { inInt = stoi(inStr); }
        catch (...) { inInt = 9; }
        switch (inInt)
        {
        case 1:
            PlayTracksInExternalPlayer((vector<Music>{ (*Musics)[id] }), false);
            break;
        case 2:
            cout << "вы уверенны что хотите удалить музыку " << (*Musics)[id].Name << "\nвведите y что-бы потдвердить удаление, иной ввод - назад\n";
            cin >> inStr;
            if (inStr == "y")
            {
                (*Musics).erase((*Musics).begin() + id);
                o = false;
            }
            break;
        case 0:
            o = false;
            break;
        default:
            break;
        }
    }
}

static void ListMenu(vector<Playlist>* Playlists, int id) {
    int inInt;
    string inStr;
    bool o = true;
    while (o == true) {
        clearConsole();
        cout << "вы находитесь в плейлисте " << (*Playlists)[id].Name << endl;
        printMenu(3);
        (*Playlists)[id].printmusics();
        cin >> inStr;
        try { inInt = stoi(inStr); }
        catch (...) { inInt = 9; }
        switch (inInt)
        {
        case 1:
            clearConsole();
            printMenu(31);
            (*Playlists)[id].printmusics();
            cin >> inStr;
            try { inInt = stoi(inStr); }
            catch (...) { inInt = 9; }
            if (inInt >= 0 and inInt < (*Playlists)[id].Musiclists.size()) {
                MusicMenu(&(*Playlists)[id].Musiclists, inInt);
            }
            break;
        case 2:
            clearConsole();
            AddMusicToPlaylist(&(*Playlists)[id]);
            cout << "Треки успешно добавлены! Нажмите Enter..." << endl;
            cin.ignore();
            cin.get();
            break;
        case 3:
            printMenu(12);
            cin >> inStr;
            try { inInt = stoi(inStr); }
            catch (...) { inInt = 9; }
            /*if (inInt == 1)
            {
                player(&(*Playlists)[id].Musiclists, true);
            }
            else if (inInt == 2) {
                player(&(*Playlists)[id].Musiclists, false);
            }*/
            if (inInt == 1 || inInt == 2) {
                PlayTracksInExternalPlayer((*Playlists)[id].Musiclists, inInt == 2);
            }
            break;
        case 4:
            cout << "вы уверенны что хотите удалить плейлист " << (*Playlists)[id].Name << "\nвведите y что-бы потдвердить удаление, иной ввод - назад\n";
            cin >> inStr;
            if (inStr == "y")
            {
                (*Playlists).erase((*Playlists).begin() + id);
                o = false;
            }
            break;
        case 5:
            cout << "введите новое название для этого плейлиста: ";
            cin >> inStr;
            (*Playlists)[id].Name = inStr;
            break;
        case 6:
            clearConsole();
            printMenu(33);
            (*Playlists)[id].printmusics();
            cin >> inStr;
            RemoveTracksFromPlaylist(&(*Playlists)[id], inStr);
            cout << "нажмите Enter для продолжения" << endl;
            cin.ignore();
            cin.get();
            break;
        case 0:
            o = false;
            break;
        default:
            break;
        }
    }
}

static void GroupMenu(vector<Group>* Groups, int id) {
    int inInt;
    string inStr;
    bool o = true;
    while (o == true) {
        clearConsole();
        cout << "вы находитесь в группе " << (*Groups)[id].Name << endl;
        printMenu(2);
        (*Groups)[id].printlists();
        cin >> inStr;
        try { inInt = stoi(inStr); }
        catch (...) { inInt = 9; }
        switch (inInt)
        {
        case 1:
            clearConsole();
            printMenu(21);
            (*Groups)[id].printlists();
            cin >> inStr;
            try { inInt = stoi(inStr); }
            catch (...) { inInt = 9; }
            if (inInt >= 0 and inInt < (*Groups)[id].Playlists.size()) {
                ListMenu(&(*Groups)[id].Playlists, inInt);
            }
            break;
        case 2:
            cout << "введите название нового плейлиста: ";
            cin >> inStr;
            (*Groups)[id].Playlists.push_back(*new Playlist(inStr));
            break;
        case 3:
            printMenu(12);
            cin >> inStr;
            try { inInt = stoi(inStr); }
            catch (...) { inInt = 9; }
            /*if (inInt == 1)
            {
                player(&(*Groups)[id].Playlists, true);
            }
            else if (inInt == 2) {
                player(&(*Groups)[id].Playlists, false);
            }*/
            if (inInt == 1 || inInt == 2) {
                vector<Music> allGroupTracks;
                for (const auto& pl : (*Groups)[id].Playlists) {
                    allGroupTracks.insert(allGroupTracks.end(), pl.Musiclists.begin(), pl.Musiclists.end());
                }
                PlayTracksInExternalPlayer(allGroupTracks, inInt == 2);
            }
            break;
        case 4:
            cout << "вы уверенны что хотите удалить группу " << (*Groups)[id].Name << "\nвведите y что-бы потдвердить удаление, иной ввод - назад\n";
            cin >> inStr;
            if (inStr == "y")
            {
                (*Groups).erase((*Groups).begin() + id);
                o = false;
            }
            break;
        case 5:
            cout << "введите новое название для этой группы: ";
            cin >> inStr;
            (*Groups)[id].Name = inStr;
            break;
        case 0:
            o = false;
            break;
        default:
            break;
        }
    }
}



static void SystemMenu(vector<Group> *Groups) {
    int inInt;
    string inStr;
    cout << "вас приветствует локальный менеджер треков SimpleLocalPlayer!" << endl;
    cout << "учтите, данные сохранятся только при ручном выполнении выхода с сохранением в меню программы\n";
    cout << "при закрытии консоли, падения программы, или инной смерти процесса данные не сохранятся!\n";
    cout << "нажмите Enter чтобы продолжить" << endl;
    cin.get();
    bool o = true;
    while (o == true) {
        clearConsole();
        printMenu(1);
        printGroups(*Groups);
        cin >> inStr;
        try { inInt = stoi(inStr); }
        catch (...) { inInt = 9; }
        switch (inInt)
        {
        case 1:
            clearConsole();
            printMenu(11);
            printGroups(*Groups);
            cin >> inStr;
            try { inInt = stoi(inStr); }
            catch (...) { inInt = 9; }
            if (inInt >= 0 and inInt < (*Groups).size()) {
                GroupMenu(Groups, inInt);
            }
            break;
        case 2:
            cout << "введите название новой группы: ";
            cin >> inStr;
            (*Groups).push_back(*new Group(inStr));
            break;
        case 3:
            printMenu(12);
            cin >> inStr;
            try { inInt = stoi(inStr); }
            catch (...) { inInt = 9; }
            if (inInt == 1 || inInt == 2) {
                vector<Music> allSystemTracks;
                for (const auto& group : *Groups) {
                    for (const auto& pl : group.Playlists) {
                        allSystemTracks.insert(allSystemTracks.end(), pl.Musiclists.begin(), pl.Musiclists.end());
                    }
                }
                PlayTracksInExternalPlayer(allSystemTracks, inInt == 2);
            }
            break;
        case 0:
            o = false;
            break;
        default:
            break;
        }
    }
}

static string GetSelfDirect() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    wstring ws(buffer);
    size_t pos = ws.find_last_of(L"\\/");
    if (pos != wstring::npos) {
        ws = ws.substr(0, pos);
    }
    return WStringToString1251(ws);
}

// Windows-1251 -> UTF-8
static std::string CP1251ToUTF8(const std::string& str) {
    if (str.empty()) return "";
    int wlen = MultiByteToWideChar(1251, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(1251, 0, str.c_str(), -1, &wstr[0], wlen);

    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string out(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &out[0], len, NULL, NULL);

    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}

// UTF-8 -> Windows-1251
static std::string UTF8ToCP1251(const std::string& str) {
    if (str.empty()) return "";
    int wlen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], wlen);

    int len = WideCharToMultiByte(1251, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string out(len, 0);
    WideCharToMultiByte(1251, 0, wstr.c_str(), -1, &out[0], len, NULL, NULL);

    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}



static void SaveToJson(std::vector<Group>& groups) {
    for (auto& group : groups) {
        group.Name = CP1251ToUTF8(group.Name);
        for (auto& playlist : group.Playlists) {
            playlist.Name = CP1251ToUTF8(playlist.Name);
            for (auto& music : playlist.Musiclists) {
                music.Name = CP1251ToUTF8(music.Name);
                music.Path = CP1251ToUTF8(music.Path);
            }
        }
    }

    std::string dir = GetSelfDirect();
    std::string filePath = dir + "\\library.json";

    json j = groups;

    std::ofstream file(filePath);
    if (file.is_open()) {
        file << j.dump(4);
        file.close();
    }
    else {
        std::cout << "не удалось сохранить данные в " << filePath << "\n";
    }
    cout << "данные сохранены\n";


    for (auto& group : groups) {
        group.Name = UTF8ToCP1251(group.Name);
        for (auto& playlist : group.Playlists) {
            playlist.Name = UTF8ToCP1251(playlist.Name);
            for (auto& music : playlist.Musiclists) {
                music.Name = UTF8ToCP1251(music.Name);
                music.Path = UTF8ToCP1251(music.Path);
            }
        }
    }
    cout << "нажмите Enter чтобы продолжить" << endl;
    cin.ignore();
    cin.get();
}

static void LoadFromJson(std::vector<Group>& groups) {
    std::string dir = GetSelfDirect();
    std::string filePath = dir + "\\library.json";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        return;
    }

    try {
        json j;
        file >> j;
        file.close();

        groups = j.get<std::vector<Group>>();

        for (auto& group : groups) {
            group.Name = UTF8ToCP1251(group.Name);
            for (auto& playlist : group.Playlists) {
                playlist.Name = UTF8ToCP1251(playlist.Name);
                for (auto& music : playlist.Musiclists) {
                    music.Name = UTF8ToCP1251(music.Name);
                    music.Path = UTF8ToCP1251(music.Path);
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cout << "ошибка чтения json: " << e.what() << "\n";
    }
}

int main()
{
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    vector<Group> Groups;
    LoadFromJson(Groups);
    SystemMenu(&Groups);
    SaveToJson(Groups);
}
