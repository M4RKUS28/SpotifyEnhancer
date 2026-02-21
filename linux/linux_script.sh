#!/usr/bin/env bash

# ==============================================================================
# Spotify Ad Skipper für Linux (Kompatibel mit Debian/Ubuntu & Fedora)
# ------------------------------------------------------------------------------
# Dieses Skript überwacht den Fenstertitel von Spotify. Wenn eine Werbung
# erkannt wird (d.h. der Titel enthält keinen Künstler/Song), wird Spotify
# schnell neu gestartet und die Wiedergabe fortgesetzt.
#
# Benötigte Befehle: wmctrl, xprop, readlink, xdotool
# ==============================================================================

# Funktion zur Überprüfung der Abhängigkeiten
check_dependencies() {
    local missing_tools=()
    # 'readlink' ist Teil von coreutils und sollte immer vorhanden sein.
    local required_tools=("wmctrl" "xprop" "xdotool")

    echo "Prüfe auf benötigte Werkzeuge..."
    for tool in "${required_tools[@]}"; do
        if ! command -v "$tool" &> /dev/null; then
            missing_tools+=("$tool")
        fi
    done

    if [ ${#missing_tools[@]} -ne 0 ]; then
        echo "------------------------------------------------------------"
        echo "FEHLER: Die folgenden Werkzeuge fehlen: ${missing_tools[*]}"
        echo "Bitte installieren Sie diese, um das Skript auszuführen."

        # Betriebssystemerkennung für Installationsanweisungen
        if [ -f /etc/os-release ]; then
            # Lädt Variablen wie ID, PRETTY_NAME etc.
            . /etc/os-release
            case "$ID" in
                ubuntu|debian|mint)
                    echo "Auf Debian/Ubuntu/Mint: sudo apt update && sudo apt install wmctrl x11-utils xdotool"
                    ;;
                fedora)
                    echo "Auf Fedora: sudo dnf install wmctrl xorg-x11-utils xdotool"
                    ;;
                arch)
                    echo "Auf Arch Linux: sudo pacman -S wmctrl xorg-xprop xdotool"
                    ;;
                *)
                    echo "Ihr Betriebssystem '$PRETTY_NAME' wurde nicht automatisch erkannt."
                    echo "Bitte installieren Sie die fehlenden Pakete mit dem Paketmanager Ihrer Distribution."
                    ;;
            esac
        else
            echo "Konnte das Betriebssystem nicht identifizieren. Bitte installieren Sie die Pakete manuell."
        fi
        echo "------------------------------------------------------------"
        exit 1
    fi
    echo "Alle Werkzeuge sind vorhanden."
}

# --- Skript-Konfiguration ---
SPOTIFY_EXE_NAME="spotify"
CHECK_INTERVAL=3 # Sekunden zwischen den Prüfungen

# Führe die Abhängigkeitsprüfung aus
check_dependencies

# --- Hauptschleife ---
echo "Spotify Ad Skipper für Linux gestartet... (Drücke Strg+C zum Beenden)"

while true; do
    # Finde das Hauptfenster von Spotify.
    line=$(wmctrl -lp | grep -i "$SPOTIFY_EXE_NAME" | head -n1)

    if [[ -z "$line" ]]; then
        # Nichts zu tun, wenn Spotify nicht läuft
        sleep 5
        continue
    fi

    # Extrahiere Fenster-ID (WID), Prozess-ID (PID) und Titel
    wid=$(awk '{print $1}' <<<"$line")
    pid=$(awk '{print $3}' <<<"$line")
    title=$(echo "$line" | cut -d ' ' -f5-)

    # Prüfe, ob es sich um Musik oder Werbung handelt
    # Logik: Echte Songs haben fast immer ein " - " im Titel (Künstler - Titel)
    if [[ "$title" != *" - "* ]] && [[ "$title" != "Spotify Free" ]]; then
        echo "----------------------------------------"
        echo "Werbung erkannt: '$title'"
        echo "Starte Spotify neu..."

        # Hole den exakten Pfad der laufenden Anwendung.
        # Dies ist robuster als 'which spotify', besonders bei Snap/Flatpak.
        exe_path=$(readlink -f "/proc/$pid/exe" 2>/dev/null)
        if [[ -z "$exe_path" ]]; then
            echo "Fehler: Konnte den Pfad zur Spotify-Anwendung nicht finden. Überspringe Neustart."
            sleep 10
            continue
        fi

        # 1. Spotify sauber beenden (entspricht WM_CLOSE)
        wmctrl -ic "$wid"
        # Warte, bis der Prozess wirklich beendet ist
        for ((i=0; i<10; i++)); do # Max 5 Sekunden warten
             kill -0 "$pid" 2>/dev/null || break
             sleep 0.5
        done
        # Wenn der Prozess nach 5 Sekunden immer noch läuft, beende ihn hart
        if kill -0 "$pid" 2>/dev/null; then
            echo "Spotify konnte nicht sauber beendet werden. Sende SIGKILL..."
            kill -9 "$pid"
        fi
        echo "Spotify wurde beendet."
        sleep 1

        # 2. Spotify neu starten
        nohup "$exe_path" >/dev/null 2>&1 &
        echo "Spotify wird neu gestartet mit: $exe_path"

        # 3. Warten, bis das neue Spotify-Fenster erscheint
        new_wid=""
        for ((i=0; i<20; i++)); do # Max. 10 Sekunden warten
            new_wid=$(wmctrl -l | grep -i "Spotify" | awk '{print $1}')
            if [[ -n "$new_wid" ]]; then
                echo "Neues Spotify-Fenster gefunden (WID: $new_wid)."
                break
            fi
            sleep 0.5
        done

        if [[ -z "$new_wid" ]]; then
            echo "Fehler: Konnte das neue Spotify-Fenster nach dem Neustart nicht finden."
            continue
        fi

        # 4. Wiedergabe starten
        sleep 2 # Gib der GUI einen Moment Zeit, sich zu initialisieren
        echo "Sende 'Play'-Signal, um die Musik fortzusetzen."
        xdotool key --window "$new_wid" XF86AudioPlay

        echo "Neustart abgeschlossen. Musik sollte weiterlaufen."
        echo "----------------------------------------"
        sleep 10 # Länger warten, damit Spotify vollständig laden kann

    fi

    sleep "$CHECK_INTERVAL"
done