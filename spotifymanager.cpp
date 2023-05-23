#include "spotifymanager.h"

#include <QApplication>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>


#include <Windows.h>
#include <iostream>
#include <string>
#include <tlhelp32.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <vector>
#include <psapi.h>


struct v_window {
    v_window() { }
    v_window( HWND window, std::string exe, std::string titel, std::string path,  bool checkInv = false,  std::string checkTitel = "")
        : window(window), titel(titel) , path(path), exe(exe), checkInv(checkInv), checkTitel(checkTitel), count(0)
    { }
    HWND window;
    std::string titel;
    std::string path;
    std::string exe;
    bool checkInv;
    std::string checkTitel;
    int count;
};


std::string GetExecutablePathFromWindowHandle(HWND hwnd)
{
    std::string path;
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess != NULL) {
        std::vector<char> buffer(MAX_PATH);
        DWORD size = GetModuleFileNameExA(hProcess, NULL, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (size != 0 && size < buffer.size()) {
            path = buffer.data();
        }
        CloseHandle(hProcess);
    }
    return path;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    auto handles = reinterpret_cast<v_window*>(lParam);
    if (handles->checkInv || IsWindowVisible(hwnd)) {
            std::vector<char> buffer(GetWindowTextLengthA(hwnd) + 1);
            GetWindowTextA(hwnd, buffer.data(), buffer.size());
            if(buffer.data()) {
                auto s = std::string(GetExecutablePathFromWindowHandle(hwnd));
                if(s.find(handles->exe) != std::string::npos) {
                    if(buffer.data() == handles->checkTitel)
                        handles->count += 1;
//                    qDebug() << buffer.data() << " = " << handles->checkTitel;

                    handles->window = hwnd;
                    handles->path = s;
                    handles->titel = buffer.data();
                } else {

                }
            }

    }
    return true;
}



SpotifyManager::SpotifyManager()
{
    this->loadExePath();
    this->counts = 0;
    this->ms_checkrate = 2000;
}

SpotifyManager::~SpotifyManager()
{
    stop();
}

void SpotifyManager::startThread()
{
    this->start();
}

void SpotifyManager::stopThread()
{
    stop();
}

bool SpotifyManager::alreadyRunningServiceAvaible()
{
    v_window vw(nullptr, QFileInfo(QApplication::applicationFilePath()).fileName().toStdString(), "", "", true, "SpotifyEnhancer");
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&vw));
    return vw.count;
}

void SpotifyManager::stop()
{
    if(this->isRunning()) {
        this->requestInterruption();
        if( ! this->wait(1000)) {
            this->quit();

            if( ! this->wait(1000)) {
                this->terminate();

                if( ! this->wait(1000)) {
                    std::cout << "Stop failed!" << std::endl;



                }

            }

        }


    }
}

void SpotifyManager::loadExePath()
{
    exePath = QSettings("SpotifyEnhancer", "SpotifyEnhancer").value("exePath", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "\\Spotify\\Spotify.exe").toString();
}

void SpotifyManager::setExePath(QString newPath)
{
    QSettings("SpotifyEnhancer", "SpotifyEnhancer").setValue("exePath", (exePath = newPath));
}

QString SpotifyManager::getExePath()
{
    return exePath;
}

int SpotifyManager::getGesammtAnzahl()
{
    return QSettings("SpotifyEnhancer", "SpotifyEnhancer").value("gesammtAnzahlUebersprungeneWerbungen", 0).toInt();
}

int SpotifyManager::load_and_getMs_checkrate()
{
    return (ms_checkrate = QSettings("SpotifyEnhancer", "SpotifyEnhancer").value("ms_checkrate", 2000).toInt());
}

int SpotifyManager::getCounts() const
{
    return counts;
}

void SpotifyManager::setMs_checkrate(int newMs_checkrate)
{
    QSettings("SpotifyEnhancer", "SpotifyEnhancer").setValue("ms_checkrate", (ms_checkrate = newMs_checkrate));
}


void SpotifyManager::run()
{
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    int loop_count = 0;
    bool closed = false;

    while(!this->isInterruptionRequested()) {
        closed = false;
        v_window vw(nullptr, "Spotify.exe", "", "");

        EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&vw));
        if( vw.titel == "" ) {
//            std::cout << "SPOTIFY NOT STARTED...waiting" << std::endl;
            sleep(10);
            continue;
        }

        while(!this->isInterruptionRequested()) {

            if( ! IsWindow(vw.window)) {
                //Spotify closed
                std::cout << "Stoptify closed!!!" << std::endl;
                sleep(2);
                closed = true;
                break;
//                emit stopedSpot();
//                return;
            }


            //get titel
            std::vector<char> buffer(GetWindowTextLengthA(vw.window) + 1);
            GetWindowTextA(vw.window, buffer.data(), buffer.size());
            if(buffer.empty() || !buffer.data()) {
                emit stopedSpot();
                return;
            }
            //check titel
            std::string titel = buffer.data();
            if(titel.find(" - ") == std::string::npos) {

                if(titel == "Spotify Free") {

                    //Entwder pausiert oder spotify werbung für spotify premium

                } else {

                    //Advertisement
                    counts++;
                    emit updateReq();
                    QSettings("SpotifyEnhancer", "SpotifyEnhancer").setValue("gesammtAnzahlUebersprungeneWerbungen", this->getGesammtAnzahl() + 1);


                    //RESTARTING:::::::::
                    break;

                }
            }
//            std::cout << "FOUND MUSIK: " << vw.titel << " with Path: " << vw.path << " OK"/* << isWindowPlayingAudio(vw.window) */ << std::endl;
            msleep(this->ms_checkrate);
        }

        if(this->isInterruptionRequested()) {
            emit stopedSpot();
            return;
        }

        if(closed) {
            continue;
        }


        std::cout << "FOUND ADVERTISEMENT!!: " << vw.titel << " restarting..." << std::endl;

        // Check if 30 sec  has elapsed
        std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count() >= 30) {
            // Calculate loops per minute
            double loops_per_minute = static_cast<double>(loop_count) / (static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count()) / 60000.0);
            // Check if there are more than 3 loops per minute
            if (loops_per_minute > 4.0) {
                std::cout << "Breaking loop due to high loop rate" << std::endl;
                return;
            }

            // Reset loop count and start time
            loop_count = 0;
            start_time = current_time;
        }

        // Close the Spotify window
        if( !stopSpotify(vw) )
            continue;
        msleep(500);


        // Start the program again;
        if(!startSpotify()) {
            std::cerr << "Error starting program" << std::endl;
            continue;
        }
        msleep(500);

        // searchSpotifyWindow
        if( ! searchSpotifyWindow(vw) ) {
            std::cerr << "Couldnd find spoitfy after restart" << std::endl;
            break;
        }

        if( !sendPlaySignal(vw) ) {
            std::cerr << "Couldnd start playing after restart" << std::endl;
        }

        // Increment loop count
        loop_count++;

        sleep(8);
    }

}


bool SpotifyManager::stopSpotify(v_window w)
{
    // Close the Spotify window
    LRESULT result = SendMessageA(w.window, WM_CLOSE, 0, 0);
    if (result == 0) {
        DWORD error = GetLastError();
        if (error != ERROR_SUCCESS) {
            // Fehler behandeln
            std::cerr << "SendMessageA failed with error:" << error << std::endl;
            return false;
        }
    }
    return true;
}

bool SpotifyManager::startSpotify()
{
    // Start the program again;
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    if (!CreateProcessA(NULL, const_cast<char*>(exePath.toStdString().c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        return false;

    // Wait for the program to start
    WaitForInputIdle(pi.hProcess, INFINITE);
    return true;
}

bool SpotifyManager::searchSpotifyWindow(v_window &vw, int trys)
{
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&vw));
    for(int i = 0; i < trys && vw.titel == ""; i++) {
        EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&vw));
        if(i == trys - 1)
            break;
        else
            sleep(2);
    }
    // If the Spotify window is found, close it and restart the program
    if (vw.titel == "")
        return false;
    return true;
}

bool  SpotifyManager::sendPlaySignal(v_window new_win)
{
    HWND current_foreground_window = GetForegroundWindow();

    // Set the program window to the foreground
    if (!SetForegroundWindow(new_win.window))
    {
        qDebug() << "Fehler beim Festlegen des Fensters als Vordergrundfenster.";
        return false;
    }

    // Wait for the window to become the foreground window
    while(GetForegroundWindow() != new_win.window)
        Sleep(100);

    // Send keyboard input to the program window -> start music again
    INPUT input[4];
    ZeroMemory(input, sizeof(input));
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = VK_MEDIA_PLAY_PAUSE;
    if (SendInput(1, input, sizeof(INPUT)) == 0)
    {
        qDebug() << "Fehler beim Senden der Tastatureingabe.";
        return false;
    }

    // Restore the original foreground window
    if (!SetForegroundWindow(current_foreground_window))
    {
        qDebug() << "Fehler beim Wiederherstellen des ursprünglichen Vordergrundfensters.";
            return false;
    }


    auto ct = std::chrono::system_clock::now();
    std::time_t current_time_t = std::chrono::system_clock::to_time_t(ct);
    std::string current_time_str = std::ctime(&current_time_t);
    std::cout << "Restarted at " << current_time_str << std::endl;

    return true;
}


void SpotifyManager::sleep(int seconds)
{
    for(int i = 0; i < seconds * 100 && !this->isInterruptionRequested(); i++)
        usleep(10000);
}

void SpotifyManager::msleep(int m_seconds)
{
    for(int i = 0; i < m_seconds / 10 && !this->isInterruptionRequested(); i++)
        usleep(10000);
}
