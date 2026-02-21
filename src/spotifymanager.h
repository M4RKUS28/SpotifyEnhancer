#ifndef SPOTIFYMANAGER_H
#define SPOTIFYMANAGER_H

#include <QObject>
#include <QThread>
#include <windows.h>

struct v_window {
    v_window() {
    }
    v_window( HWND window, std::string exe, std::string titel, std::string path,  bool checkInv = false,  std::string checkTitel = "")
        : window(window), last_name_fitted_test_hwnd(nullptr) , exe(exe), titel(titel), path(path), checkInv(checkInv), checkTitel(checkTitel), count(0)
    {
    }
    HWND window;
    HWND last_name_fitted_test_hwnd;
    std::string exe;
    std::string titel;
    std::string path;
    bool checkInv;
    std::string checkTitel;
    int count;
};

class SpotifyManager : public QThread
{
    Q_OBJECT
public:
    typedef HWND WindowHandle;

    SpotifyManager();
    ~SpotifyManager();

    void startThread();
    void stopThread();

    WindowHandle alreadyRunningServiceAvaible();
    bool showHiddenService(WindowHandle w);

    void setExePath(QString newPath);
    QString getExePath();

    int getGesamtAnzahl();
    bool startSpotify();
    bool searchSpotifyWindow(v_window *vw, int trys = 30);
    bool sendPlaySignal(v_window w);
    bool stopSpotify(v_window w);

private:

    int counts;
    int ms_checkrate;
    int transitionWaitingTime;

    static constexpr int default_check_rate = 800;
    bool use_special_methode;
    int playDelay;

    void stop();
    virtual void run();
    void sleep(int seconds);
    void msleep(int m_seconds);
    void loadExePath();

    QString exePath;

public:
    void setMs_checkrate(int newMs_checkrate);
    int getCounts() const;
    int load_and_getMs_checkrate();

    void setUse_special_methode(bool newUse_special_methode);

    void setPlayDelay(int newPlayDelay);
    void setTransitionWaitingTime(int waitingTime);

signals:
    void updateReq();
    void stopedSpot();

};

#endif // SPOTIFYMANAGER_H
