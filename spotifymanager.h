#ifndef SPOTIFYMANAGER_H
#define SPOTIFYMANAGER_H

#include <QObject>
#include <QThread>
#include <windows.h>

struct v_window;


class SpotifyManager : public QThread
{
    Q_OBJECT
public:
    SpotifyManager();
    ~SpotifyManager();


    void startThread();
    void stopThread();

    bool alreadyRunningServiceAvaible();


    void setExePath(QString newPath);
    QString getExePath();


    int getGesammtAnzahl();
    bool startSpotify();
    bool searchSpotifyWindow(v_window &vw, int trys = 10);
    bool sendPlaySignal(v_window w);
    bool stopSpotify(v_window w);

private:

    int counts;
    int ms_checkrate;


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

signals:
    void updateReq();
    void stopedSpot();

};

#endif // SPOTIFYMANAGER_H
