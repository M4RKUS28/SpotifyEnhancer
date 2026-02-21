#include "spotifymanager.h"

#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>


#include <Windows.h>
#include <iostream>
#include <string>
#include <tlhelp32.h>
#include <chrono>
#include <thread>
#include <vector>
#include <psapi.h>
#include <QDebug>







std::string GetExecutablePathFromWindowHandle(HWND hwnd)
{
    std::string path;
    DWORD pid = 0;
    if (!GetWindowThreadProcessId(hwnd, &pid)) {
        // Failed to get process ID
        DWORD error = GetLastError();
        qDebug() << "Failed to get process ID. Error code:" << error;
        return "";
    }

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess != NULL) {
        std::vector<char> buffer(MAX_PATH);
        DWORD size = GetModuleFileNameExA(hProcess, NULL, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (size != 0 && size < buffer.size()) {
            path = buffer.data();
        } else {
            // Failed to get module file name
            DWORD error = GetLastError();
            qDebug() << "Failed to get module file name. Error code:" << error;
        }
        if (!CloseHandle(hProcess)) {
            // Failed to close handle
            DWORD error = GetLastError();
            qDebug() << "Failed to close process handle. Error code:" << error;
        }
    }
    return path;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    auto handles = reinterpret_cast<v_window*>(lParam);
    if (handles->checkInv || IsWindowVisible(hwnd)) {
        int textLen = GetWindowTextLengthA(hwnd);
        if (textLen <= 0)
            return true;
        std::vector<char> buffer(textLen + 1);
        if (!GetWindowTextA(hwnd, buffer.data(), buffer.size()))
            return true;

        // Pre-filter: if a specific title is required, check it cheaply before the
        // expensive path lookup
        std::string title = buffer.data();
        if (!handles->checkTitel.empty() && title != handles->checkTitel)
            return true;

        auto s = std::string(GetExecutablePathFromWindowHandle(hwnd));
        if (s.find(handles->exe) != std::string::npos) {
            if (title == handles->checkTitel) {
                handles->count += 1;
                handles->last_name_fitted_test_hwnd = hwnd;
            }
            handles->window = hwnd;
            handles->path = s;
            handles->titel = title;
        }
    }
    return true;
}



SpotifyManager::SpotifyManager()
{
    this->loadExePath();
    this->counts = 0;
    this->ms_checkrate = 1000;
    use_special_methode = false;
    playDelay = 500;
    transitionWaitingTime = 0;
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

SpotifyManager::WindowHandle SpotifyManager::alreadyRunningServiceAvaible()
{
    v_window vw(nullptr, QFileInfo(QApplication::applicationFilePath()).fileName().toStdString(), "", "", true, "SpotifyEnhancer");
    if (!EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&vw))) {
        DWORD error = GetLastError();         // Failed to enumerate windows
        qDebug() << "Failed to enumerate windows. Error code:" << error;
        return nullptr;
    }
    return vw.count > 0 ? vw.last_name_fitted_test_hwnd : nullptr;
}

bool SpotifyManager::showHiddenService(WindowHandle w)
{
    //Send signal to Show Window
        // Using WM_USER + 1 in a SendMessage call means you're sending a custom message with an identifier of 1025.  This message will have no predefined meaning to Windows itself;
        //it's up to the application receiving the message to interpret and handle it.

    return w ? (SendMessage(w, WM_USER + 2, 0, (LPARAM)187) == 187) : false;
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
    QSettings s("M4RKUS", "SpotifyEnhancer");
    bool init = s.contains("exePath");
    QString default_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/../Spotify/Spotify.exe";

    exePath = s.value("exePath", default_path).toString();
    if(!init && !QFile(default_path).exists()) {
        QMessageBox::information(nullptr, "Spotify Pfad festlegen", "Um Spotify Enhancer verwenden zu können, müssen sie den Pfad zur Spotify Anwendung festlegen! (Unter Einstellungen -> Spotify Pfad setzen)"
                                                                    "\n\nWichtig: Spotify muss über einen Installer in einem echten Ordner installiert sein "
                                                                    "und nicht über den Microsoft Store!");
    }
}

void SpotifyManager::setExePath(QString newPath)
{

    QSettings("M4RKUS", "SpotifyEnhancer").setValue("exePath", (exePath = newPath));
}

QString SpotifyManager::getExePath()
{
    return exePath;
}

int SpotifyManager::getGesamtAnzahl()
{
    return QSettings("M4RKUS", "SpotifyEnhancer").value("gesamtAnzahlUebersprungeneWerbungen", 0).toInt();
}

int SpotifyManager::load_and_getMs_checkrate()
{
    ms_checkrate = QSettings("M4RKUS", "SpotifyEnhancer").value("ms_checkrate", this->default_check_rate).toInt();
    std::cout << "  -> LOADED ms_checkrate := " << ms_checkrate << std::endl;
    return ms_checkrate;
}

int SpotifyManager::getCounts() const
{
    return counts;
}

void SpotifyManager::setMs_checkrate(int newMs_checkrate)
{
    QSettings("M4RKUS", "SpotifyEnhancer").setValue("ms_checkrate", (ms_checkrate = newMs_checkrate));
    std::cout << "  -> ms_checkrate := " << newMs_checkrate << std::endl;
}


void SpotifyManager::run()
{
    sleep(2);

    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    int loop_count = 0;
    bool closed = false;

    while(!this->isInterruptionRequested()) {
        closed = false;
        v_window vw(nullptr, "Spotify.exe", "", "");

        if (!EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&vw))) {
            // Failed to enumerate windows
            DWORD error = GetLastError();
            qDebug() << "Failed to enumerate windows. Error code:" << error;
        }
        if( vw.titel == "" ) {
//            std::cout << "SPOTIFY NOT STARTED...waiting" << std::endl;
            sleep(10);
            continue;
        }

        while(!this->isInterruptionRequested()) {

            if( ! IsWindow(vw.window)) {
                //Spotify closed
                std::cout << "Spotify closed!" << std::endl;
                sleep(2);
                closed = true;
                break;
//                emit stopedSpot();
//                return;
            }


            //get titel
            std::vector<char> buffer(GetWindowTextLengthA(vw.window) + 1);
            int windowTextLength = GetWindowTextLengthA(vw.window);
            if (windowTextLength > 0) {
                if (GetWindowTextA(vw.window, buffer.data(), buffer.size()) <= 0) {
                    // Failed to get window text
                    DWORD error = GetLastError();
                    qDebug() << "Failed to get window text. Error code:" << error;
                }
            } else if (windowTextLength == 0) {
                // Window has no text
                qDebug() << "Window has no text.";
                emit stopedSpot();
                return;
            } else {
                // Failed to get window text length
                DWORD error = GetLastError();
                qDebug() << "Failed to get window text length. Error code:" << error;
                emit stopedSpot();
                return;
            }

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
                    QSettings("M4RKUS", "SpotifyEnhancer").setValue("gesamtAnzahlUebersprungeneWerbungen", this->getGesamtAnzahl() + 1);

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

        //Wait delay
        msleep(transitionWaitingTime);

        // Check if 60 sec  has elapsed
        std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count() >= 60) {
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
        msleep(50 + playDelay);


        // Start the program again;
        if(!startSpotify()) {
            std::cerr << "Error starting program" << std::endl;
            continue;
        }
        msleep(50 + playDelay);

        // searchSpotifyWindow
        if( ! searchSpotifyWindow(&vw) ) {
            std::cerr << "Couldn't find Spotify after restart" << std::endl;
            break;
        }
        msleep(100);

        if( !sendPlaySignal(vw) ) {
            std::cerr << "Couldn't start playing after restart" << std::endl;
        }

        // Increment loop count
        loop_count++;

        sleep(10);
    }

}


bool SpotifyManager::stopSpotify(v_window w)
{
    // Get the process ID associated with the window
    DWORD processId;
    GetWindowThreadProcessId(w.window, &processId);

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

    // Get a handle to the process
    HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (hProcess == NULL) {
        // Handle error
        DWORD error = GetLastError();
        std::cerr << "OpenProcess failed with error:" << error << std::endl;
        return false;
    }

    // Wait for the process to exit
    DWORD waitResult = WaitForSingleObject(hProcess, 5000);
    if (waitResult == WAIT_FAILED) {
        // Handle error
        DWORD error = GetLastError();
        std::cerr << "WaitForSingleObject failed with error:" << error << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    // Get the exit code of the process
    DWORD exitCode;
    if (!GetExitCodeProcess(hProcess, &exitCode)) {
        // Handle error
        DWORD error = GetLastError();
        std::cerr << "GetExitCodeProcess failed with error:" << error << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    // Close the process handle
    if(!CloseHandle(hProcess)) {
        std::cerr << "CloseHandle failed" << std::endl;
    }

    // Check if the process exited successfully
    if (exitCode != STILL_ACTIVE) {
        // Process has exited
        return true;
    } else {
        // Process did not exit within the specified time
        std::cerr << "WaitForSingleObject timed out" << std::endl;
        return false;
    }
}

void SpotifyManager::setPlayDelay(int newPlayDelay)
{
    playDelay = newPlayDelay;
    std::cout << "  -> PlayDelay := " << newPlayDelay << std::endl;
}

void SpotifyManager::setTransitionWaitingTime(int waitingTime)
{
    transitionWaitingTime = waitingTime;
    std::cout << "  -> transitionWaitingTime := " << transitionWaitingTime << std::endl;
}

void SpotifyManager::setUse_special_methode(bool newUse_special_methode)
{
    use_special_methode = newUse_special_methode;
}

bool SpotifyManager::startSpotify()
{
    // Start the program again;
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    if (!CreateProcessA(NULL, const_cast<char*>(exePath.toStdString().c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))  {
        qDebug() << "CreateProcessA failed: " << GetLastError();
        return false;
    }
    qDebug() << "Wait for the program to start";
    // Wait for the program to start
    WaitForInputIdle(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    qDebug() << "started";
    return true;
}

bool SpotifyManager::searchSpotifyWindow(v_window *vw, int trys)
{
    if (!EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(vw))) {
        // Failed to enumerate windows
        DWORD error = GetLastError();
        qDebug() << "Failed to enumerate windows. Error code:" << error;
    }
    for(int i = 0; i < trys && vw->titel == ""; i++) {
        if (!EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(vw))) {
            // Failed to enumerate windows
            DWORD error = GetLastError();
            qDebug() << "Failed to enumerate windows. Error code:" << error;
        }
        if(i == trys - 1) {
            break;
        }
        else
            msleep(750);
    }
    // If the Spotify window is found, close it and restart the program
    if (vw->titel == "")
        return false;
    return true;
}

bool  SpotifyManager::sendPlaySignal(v_window new_win)
{
    HWND current_foreground_window = GetForegroundWindow();

    // Set the program window to the foreground
    bool forgroundsetting_worked = true;
    bool tried_sepcialthings = false;
    DWORD dwCurrentThread = GetCurrentThreadId();
    DWORD dwFGThread      = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    if (!SetForegroundWindow(new_win.window))
    {
        qDebug() << "Fehler beim Festlegen des Fensters als Vordergrundfenster." << GetLastError();
        forgroundsetting_worked = false;

        if(use_special_methode) {
            tried_sepcialthings = true;

            qDebug() << "try sonder methode";
            if(!AttachThreadInput(dwCurrentThread, dwFGThread, TRUE)) {
                qDebug() << "Special Methode: AttachThreadInput() failed";
            } else if(SetForegroundWindow(new_win.window)) {
                qDebug() << "In special Methode it worked";
                forgroundsetting_worked = true;
            }
        }
    }

    // Wait for the window to become the foreground window
    for(int i = 0; i < 40 && GetForegroundWindow() != new_win.window; i++)
        msleep(100);

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
    if (forgroundsetting_worked && !SetForegroundWindow(current_foreground_window))
    {
        qDebug() << "Fehler beim Wiederherstellen des ursprünglichen Vordergrundfensters.";
//            return false;
    }

    if(tried_sepcialthings)
        if(!AttachThreadInput(dwCurrentThread, dwFGThread, FALSE)) {
            qDebug() << "Special Methode - restore : AttachThreadInput() failed";
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
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void SpotifyManager::msleep(int m_seconds)
{
    for(int i = 0; i < m_seconds / 10 && !this->isInterruptionRequested(); i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
