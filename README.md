# Playlist 2 Folders

Itunes has failed me for the last time, and now I need a way to listen to local music files on my phone some other way. One thing I can't do without is the convenience of playlists, but how can I have playlists? Most apps I could find were either freemium or had limited tools for managing local files - looking at you Spotify! So my compromise is to use a file browser with a built-in media player and to organise my music files into folders that represent each playlist I've got. 

## Goal

1. Take a playlist exported from itunes
2. Parse the playlist extracting all the songs and their locations
3. Create folders and copy in the songs
4. *Transfer the folders to my phone (FTP or some such)*
5. *Listen to my beautiful beautiful playlists*

*Note: Italicized items are not in the scope of this program*

```
Music
  ├── play2fold.exe
  ├── playlist_1.txt
  └── playlist_2.txt

        |
        ▼

Music
  ├── playlist_1
  │   ├── songA.mp3
  │   └── songB.m4a
  ├── playlist_2
  │   ├── songA.mp3
  │   ├── songC.mp3
  │   └── songD.m4a
  │
  ├── play2fold.exe
  ├── playlist_1.txt
  └── playlist_2.txt
```

## Discussion

* Itunes exports uft-16le txt files so I convert it to uft-8 before doing the folder creation. This involves reading the entire playlist txt into RAM which shouldn't be an issue for most playlists.
* As with the diagram where songA appears twice, this will make duplicate copies of songs for each playlist it appears in. To avoid this, we can use shortcuts instead, but that makes copying the folders to a phone more complicated than it needs to be.

## Usage

Usage is pretty simple, just run:

```powershell
g++ -std=c++20 play2fold.cpp -o play2fold

./play2fold.exe playlist_1.txt playlist_2.txt ...
```

Of course, this relies on the itunes playlist export being up-to-date with the song file locations on your system.
