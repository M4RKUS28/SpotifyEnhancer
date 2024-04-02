
echo "Delete old exe File..."
rm ./packages/de.spotifyenhancer.main/data/SpotifyEnhancer.exe

echo "Copying new exe File..."
cp ../release/SpotifyEnhancer.exe ./packages/de.spotifyenhancer.main/data/SpotifyEnhancer.exe


# Prompt the user to press Enter
read -p "Press Enter to close the terminal"