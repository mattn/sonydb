/*
** Sonydb.h
**
** Made by (julien)
*/

#ifndef SONYDB_H
# define SONYDB_H

#ifdef __GNUC__
#undef _WIN32
#endif

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#ifdef __MINGW32__
#include <io.h>
#endif
#endif
#include <stdio.h>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <algorithm>

using namespace std;

#ifndef _ID3LIB_ID3_H_
typedef short 		      int16;
typedef int 		      int32;
//typedef long long 	      int64;
typedef char 		      int8;
typedef unsigned short 	      uint16;
typedef unsigned int 	      uint32;
//typedef unsigned long long    uint64;
typedef unsigned char 	      uint8;
#endif
#if !defined(_WIN32) && !defined(__MINGW32__)
typedef long long __int64;
#endif
typedef unsigned short       utf16char;

#define UINT32_SWAP_BE_LE(val) ((uint32) ( \
(((uint32) (val) & (uint32) 0x000000ffU) << 24) | \
(((uint32) (val) & (uint32) 0x0000ff00U) <<  8) | \
(((uint32) (val) & (uint32) 0x00ff0000U) >>  8) | \
(((uint32) (val) & (uint32) 0xff000000U) >> 24)))

#define UINT16_SWAP_BE_LE(val) ((uint16) ( \
(((uint16) (val) & (uint16) 0x00ffU) << 8) | \
(((uint16) (val) & (uint16) 0xff00U) >> 8)))

#define SYNCHSAFE_B1(val) (((uint32) (val) >> 21) & (uint32) 0x000007F)
#define SYNCHSAFE_B2(val) (((uint32) (val) >> 14) & (uint32) 0x000007F)
#define SYNCHSAFE_B3(val) (((uint32) (val) >> 7) & (uint32) 0x000007F)
#define SYNCHSAFE_B4(val) (((uint32) (val) & (uint32) 0x0000007F))

#define NOT_SYNCHSAFE_B1(val) (uint8) (((val) & (uint32) 0xff000000U) >> 24);
#define NOT_SYNCHSAFE_B2(val) (uint8) (((val) & (uint32) 0x00ff0000U) >> 16);
#define NOT_SYNCHSAFE_B3(val) (uint8) (((val) & (uint32) 0x0000ff00U) >>  8);
#define NOT_SYNCHSAFE_B4(val) (uint8) ((val) & (uint32)  0x000000ffU);

utf16char *ansi_to_utf16(const char  *str, long len, bool endian);
char *utf16_to_ansi(const utf16char *str, long len, bool endian);

#define TAGSIZE 128
#define OUTPUT_TAGSIZE 128

#define ON_DEVICE	      0
#define ADD_TO_DEVICE	      1
#define REMOVE_FROM_DEVICE    2
// 3 is reserved for MODIFIED
#define EMPTYTRACK            4

#define NOT_LOADED	      0
#define LOADED		      1
#define MODIFIED	      2

#define ENCODING_USE_NONE	      0
#define ENCODING_USE_TABLE	      1
#define ENCODING_USE_KEY	      2

#define DATABASE_HEADER_SIZE 0


typedef struct {
     char *artist;
     char *title;
     char *album;
     char *filename;
     char *genre;
     utf16char *wArtist;
     utf16char *wTitle;
     utf16char *wAlbum;
     //utf16char *wFilename;
     utf16char *wGenre;
     int songlen; // seconds?
     int track_nr;
     int year;
     
     int       sonyDbOrder;
     int       statusOfSong; //0 was present on player, 1 was not present on player needs & to be added, 2 was present & needs to be removed
     uint8     encoding; // mpeg version(2bits), layer version(2bits), bitrate(4bits)
} Song;

typedef struct {
	 int  index;
     char *name;
     vector<Song*> songs;
} Playlist;

typedef struct
{
  uint8 magic[4];      /* "magic file descriptor" */
  uint8 cte[4];        /* Constant value */
  uint8 count;         /* Number of object pointers */
  uint8 padding[7];    /* padding to 16 bytes */
} sonyFileHeader;

typedef struct
{
  uint8  magic[4];      /* magic (same as object) */
  uint32 offset;        /* offset of the object (from the beginning)*/
  uint32 length;        /* size of object in bytes */
  uint32 padding;       /* padding to 16 bytes */
} sonyObjectPointer;

typedef struct
{
  uint8 magic[4];       /* magic (same as object pointer) */
  uint16 count;         /* record count */
  uint16 size;		/* record size */
  uint32 padding[2];    /* padding to 16 bytes */
} sonyObject;

typedef struct
{
 uint8  fileType[4];
 uint32 trackEncoding;
 uint32 trackLength;
 uint16 nbTagRecords;
 uint16 sizeTagRecords;
} sonyTrack;

typedef struct
{
     uint8 tagType[4];
     uint8 tagEncoding[2];
}sonyTrackTag;

bool sortByIndex(Song *a, Song *b);
bool sortByTrackNumber(Song *a, Song *b);
bool sortByAlbumName(Song *a, Song *b);
bool sortByArtistName(Song *a, Song *b);
bool sortByTitleName(Song *a, Song *b);
bool sortByGenreName(Song *a, Song *b);
bool sortPlaylist(Playlist s, Playlist s2);
bool sortByPlaylistIndex(Playlist s, Playlist s2);


class SonyDb
{
  private:

     
     int  id;
     int  lastTrackIndex;
     int  nbTrackToDel;
     int  nbTrackToAdd;
     char* driveLetter;    
     int  trackListLoaded; //0 not loaded, 1 loaded, 2 modified
     char *deviceName;
     char *decodeTableFilename;
     bool useAllTags; //if you want to have "all albums" "all genre" etc...

     /* disk space */
     __int64 addTrackTotalByte;
     __int64 delTrackTotalByte;
     __int64 usedSpaceDisk;
     __int64 freeSpaceDisk;
     __int64 neededSpaceValue;
     __int64 totalDiskSpaceValue;
     char *totalDiskSpace;
     char *totalUsedSpaceAfterApply;
     char *freeDiskSpaceAfterApply;
     char *addTrackTotalByteString;
     char *delTrackTotalByteString;
     char *neededSpace;

     /* estimated time */
     int bytePerSec; //estimated value will be calculated after first file as been transfered
     __int64 totalByteLeftToWrite;

     /* encoding decoding*/
     int  getTrackNumber(char *filename); //read the track number directly from the omg header
     uint32 DvId;
     int  codeType; //0 no code, 1 decodeKeys.dat, 2 DvId.dat

     /* copy progress */
     bool copying; //currently getting or adding Oma files, or rewriting db
     int  copyIndex; //current file index
     float copyPercent;// progress of the current file (in percent)


     /* device related */
     void freeAllTracks();
     void freeAllPlaylist();
     bool writeDatabase(vector<Song *> songsToSend);
     char *GetOMAFilename(int id);


     /* common header files */
     bool getHeader(sonyFileHeader *fh, FILE *f);
     bool getObjectPointer(sonyObjectPointer *op, FILE *f);
     bool getObject(sonyObject *obj, FILE *f);
     bool getTrack(FILE *f, Song *output);
     bool writeHeader(sonyFileHeader *h, FILE *f);
     bool writeHeader(sonyFileHeader *h, FILE *f, int count);
     bool writeObjectPointer(sonyObjectPointer *p, FILE *f);
     bool writeObject(sonyObject *obj, FILE *f);
     bool writeTrackHeader(sonyTrack *t, FILE *f);
     bool writeTrackTag(sonyTrackTag *tt, char *input, FILE *f);

     /* file writers */
     bool write_00GTRLST();
     bool write_01TREEXX(vector<Song *> songsToSend, vector<Song *> list, int type);
     bool write_01TREE22();
     bool write_02TREINF(vector<Song *> songsToSend);
     bool write_03GINFXX(vector<Song *> songsToSend, int type);
     bool write_03GINF22();
     bool write_04CNTINF(vector<Song *> songsToSend);
     bool write_05CIDLST(vector<Song *> songsToSend);
     bool write_TrackNumber(vector<Song *> songsToSend);


     /* decoder encoder */
     uint8 codeTable[1024];
     bool loadCodeTable(int id);
     bool addOMA(Song *s, int destination);
     void deleteOMA(char *filename);

     /* directory */
     void createDir(int highestValue);

     //debug
     FILE *fp;
     vector<Song>     songs;
     vector<Song>     songs_temporary;
     vector<Playlist> playlist;
     vector<Playlist> playlist_temporary;

  public:

     SonyDb();
     ~SonyDb();

     bool writeTracks();
     int  readAllTracks();
     int  readAllPlaylist();
     
     vector<Song*> getSongs();
     vector<Song*> getSongsInPlaylist(int source);
     vector<Playlist*> getPlaylist();
     bool deletePlaylist(int source, bool removeSongs);

     bool addSong(Song *s); //add to the database
     int  delSong(Song *s); //del to the database
	 bool updSong(Song *s); //update to the database
     bool getOMA(Song *s, char *destination);//download oma to mp3

     
     /*encode decoder*/
     void setTable(char *decodeTableFileName, int type);

     /* misc */
     bool isPresent();
     bool isCopying();
     int  progressIndex();
     int  getCopyPercent();
     bool detectPlayer();
     bool detectPlayer(char* drive);
     char* getDriveLetter();
     int  getNumberOfTracks();
     char *getDeviceName();
     int  getId();
     void setId(int newId);
     int  getNbTrackToDel();
     int  getNbTrackToAdd();
     int  isTrackListLoaded();
     void setUseAllTags(bool value); //if you want to have "all albums" "all genre" etc...
     bool getUseAllTags();

     /* disk space*/
     void updateDiskSpaceInfo();
     char *getTotalDiskSpace();
     char *getTotalUsedSpaceAfterApply();
     char *getFreeDiskSpaceAfterApply();
     char *getSizeTrackToAdd();
     char *getSizeTrackToDel();
     char *getNeededSpace();
     int  getNeededSpaceValue();

     /* playlist */
     bool addPlaylist(Playlist *p);
     int  getNbPlaylist();


};

#endif /* !SONYDB_H */
