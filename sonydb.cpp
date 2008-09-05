/*
 ** SonyDB.cpp
 **
 ** Made by (julien)
 */

#include <stdio.h>
#include <fstream>
#include "sonydb.h"

using namespace std;

#ifndef _WIN32
#define strnicmp(x, y, z) strncasecmp(x, y, z)
#define stricmp(x, y) strcasecmp(x, y)
#endif

//SORT function for song vectors
#define SKIP_THE_AND_WHITESPACE(x) { while (!isalnum(*x) && *x) x++; if (!strnicmp(x,"the ",4)) x+=4; while (*x == ' ') x++; }
static int STRCMP_NULLOK(const char *pa, const char *pb)
{
	if (!pa) pa="";
	else SKIP_THE_AND_WHITESPACE(pa)

		if (!pb) pb="";
		else SKIP_THE_AND_WHITESPACE(pb)
			return stricmp(pa,pb);
}

static int STRCMP2_NULLOK(const char *pa, const char *pb)
{
	if (!pa) pa="";
	if (!pb) pb="";
	return stricmp(pa,pb);
}

static int STRNCMP_NULLOK(const char *pa, const char *pb, int size)
{
	if (!pa) pa="";  
	if (!pb) pb="";
	return strnicmp(pa, pb, size);
}



SonyDb::SonyDb()
{
	this->driveLetter = 0;
	this->deviceName = (char*)calloc(sizeof(char), 256);
	this->trackListLoaded = false;
	this->nbTrackToAdd = 0;
	this->nbTrackToDel = 0;
	this->lastTrackIndex = 1;
	this->codeType = ENCODING_USE_NONE;
	this->copying = false;
	this->copyIndex = 0;
	this->copyPercent = 0;
	this->addTrackTotalByte = 0;
	this->delTrackTotalByte = 0;
	this->freeSpaceDisk = 0;
	this->neededSpace = 0;
	this->totalDiskSpaceValue = 0;
	this->useAllTags = false;


	totalDiskSpace = (char*)calloc(sizeof(char), 64);
	freeDiskSpaceAfterApply = (char*)calloc(sizeof(char), 64);
	totalUsedSpaceAfterApply = (char*)calloc(sizeof(char), 64);
	delTrackTotalByteString = (char*)calloc(sizeof(char), 64);
	addTrackTotalByteString = (char*)calloc(sizeof(char), 64);
	neededSpace = (char*)calloc(sizeof(char), 64);

	//debug
	//fp = fopen("c:/Debug_ml_sony.log", "ab");
	//fprintf(fp,"+++++++++++++\n"); 
	//fflush(fp);
	fp = stderr;
}

//clean the song list before living
SonyDb::~SonyDb()
{
	//free track list 
	freeAllTracks();

	//free device Name
	free(this->deviceName);

	//free disk space info
	free(totalDiskSpace);
	free(totalUsedSpaceAfterApply);
	free(freeDiskSpaceAfterApply);
	free(delTrackTotalByteString);
	free(addTrackTotalByteString);
	free(neededSpace);

	//debug
	//fclose(fp);

	if (driveLetter) free(driveLetter);
}

//free track list
void SonyDb::freeAllTracks()
{
	for (vector<Song>::iterator i = songs.begin(); i != songs.end(); i++)
	{
		if ((*i).album) free((*i).album);
		if ((*i).artist) free((*i).artist);
		if ((*i).title) free((*i).title);
		if ((*i).genre) free((*i).genre);
		if ((*i).filename) free((*i).filename);
	}
	while (songs.size() > 0)
		songs.pop_back();
}

//free playlist
void SonyDb::freeAllPlaylist()
{
	vector<Song *> plSongs;

	for (vector<Playlist>::iterator pl = playlist.begin(); pl != playlist.end(); pl++)
	{
		plSongs = (*pl).songs;
		for (vector<Song*>::iterator i = plSongs.begin(); i != plSongs.end(); i++)
		{
			if ((*i)->statusOfSong == MODIFIED)
			{
				free((*i)->filename);
				free((*i)->artist);
				free((*i)->album);
				free((*i)->genre);
				free((*i)->title);
			}
		}
	}
	while (playlist.size() > 0)
		playlist.pop_back();
}

int  SonyDb::getId()
{
	return (this->id);
}

void SonyDb::setId(int newId)
{
	this->id = newId;
}

void SonyDb::setUseAllTags(bool value)
{
	useAllTags = value;
}

bool SonyDb::getUseAllTags()
{
	return (useAllTags);
}

vector<Playlist*> SonyDb::getPlaylist()
{    
	vector<Playlist*> ret;
	for (vector<Playlist>::iterator pl = playlist_temporary.begin(); pl != playlist_temporary.end(); pl++)
		ret.push_back(&(*pl));
	for (vector<Playlist>::iterator pl2 = playlist.begin(); pl2 != playlist.end(); pl2++)
		ret.push_back(&(*pl2));
	return (ret);
}

vector<Song*> SonyDb::getSongsInPlaylist(int source)
{
	vector<Song*> ret;
	Song *p;
	vector<Song *> plSongs;
	vector<Playlist>::iterator playlistBegin;
	vector<Playlist>::iterator playlistEnd;

	if (this->copying)
	{
		playlistBegin = playlist_temporary.begin();
		playlistEnd = playlist_temporary.end();
	}
	else
	{
		playlistBegin = playlist.begin();
		playlistEnd = playlist.end();
	}

	for (vector<Playlist>::iterator pl = playlistBegin; pl != playlistEnd; pl++)
	{
		if (pl->index  == source)
		{
			plSongs = pl->songs;
			for (vector<Song*>::iterator i = plSongs.begin(); i != plSongs.end(); i++)
			{
				if ((*i)->statusOfSong != EMPTYTRACK)
				{
					//new song
					p = (Song *)calloc(sizeof(Song),1);
					p->filename = strdup((*i)->filename);
					p->artist = strdup((*i)->artist);
					p->album = strdup((*i)->album);
					p->genre = strdup((*i)->genre);
					p->title = strdup((*i)->title);

					p->track_nr = (*i)->track_nr;
					p->songlen = (*i)->songlen;
					p->year = (*i)->year;
					p->statusOfSong = (*i)->statusOfSong;
					p->sonyDbOrder = (*i)->sonyDbOrder;
					ret.push_back(p);
				}
			}
			return (ret);
		}
	}

	//not found in temporary playlists so try in other playlist
	playlistBegin = playlist.begin();
	playlistEnd = playlist.end();

	for (vector<Playlist>::iterator pl2 = playlistBegin; pl2 != playlistEnd; pl2++)
	{
		if (pl2->index  == source)
		{
			plSongs = pl2->songs;
			for (vector<Song*>::iterator i = plSongs.begin(); i != plSongs.end(); i++)
			{
				if ((*i)->statusOfSong != EMPTYTRACK)
				{
					//new song
					p = (Song *)calloc(sizeof(Song),1);
					p->filename = strdup((*i)->filename);
					p->artist = strdup((*i)->artist);
					p->album = strdup((*i)->album);
					p->genre = strdup((*i)->genre);
					p->title = strdup((*i)->title);

					p->track_nr = (*i)->track_nr;
					p->songlen = (*i)->songlen;
					p->year = (*i)->year;
					p->statusOfSong = (*i)->statusOfSong;
					p->sonyDbOrder = (*i)->sonyDbOrder;
					ret.push_back(p);
				}
			}
			return (ret);
		}
	}

	return (ret);
}

bool SonyDb::deletePlaylist(int source, bool removeSongs)
{
	vector<Song *> plSongs;

	for (vector<Playlist>::iterator pl = playlist.begin(); pl != playlist.end(); pl++)
	{
		if (pl->index  == source)
		{
			plSongs = pl->songs;
			if (removeSongs)
			{
				for (vector<Song*>::iterator song = plSongs.begin(); song != plSongs.end(); song++)
				{
					delSong(*song);
				}
			}
			pl->songs.clear();
			playlist.erase(pl);
			return (true);
		}
	}
	return (false);

}

vector<Song*> SonyDb::getSongs()
{
	vector<Song*> ret;
	Song *p;

	if (this->copying)
	{
		for (vector<Song>::iterator i = songs_temporary.begin(); i != songs_temporary.end(); i++)
		{
			if ((*i).statusOfSong != EMPTYTRACK)
			{
				//new song
				p = (Song *)calloc(sizeof(Song),1);
				p->filename = strdup((*i).filename);
				p->artist = strdup((*i).artist);
				p->album = strdup((*i).album);
				p->genre = strdup((*i).genre);
				p->title = strdup((*i).title);

				p->track_nr = (*i).track_nr;
				p->songlen = (*i).songlen;
				p->year = (*i).year;
				p->statusOfSong = (*i).statusOfSong;
				p->sonyDbOrder = (*i).sonyDbOrder;
				ret.push_back(p);
			}
		}
	}
	else
	{
		for (vector<Song>::iterator i = songs.begin(); i != songs.end(); i++)
		{
			if ((*i).statusOfSong != EMPTYTRACK)
			{
				//new song
				p = (Song *)calloc(sizeof(Song),1);
				p->filename = strdup((*i).filename);
				p->artist = strdup((*i).artist);
				p->album = strdup((*i).album);
				p->genre = strdup((*i).genre);
				p->title = strdup((*i).title);

				p->track_nr = (*i).track_nr;
				p->songlen = (*i).songlen;
				p->year = (*i).year;
				p->statusOfSong = (*i).statusOfSong;
				p->sonyDbOrder = (*i).sonyDbOrder;
				ret.push_back(p);
			}
		}
	}
	return (ret);
}


int SonyDb::progressIndex()
{
	return (this->copyIndex);
}


int SonyDb::isTrackListLoaded()
{
	return (this->trackListLoaded);
}

int SonyDb::getNbTrackToDel()
{
	return (this->nbTrackToDel);
}

int SonyDb::getNbTrackToAdd()
{
	return (this->nbTrackToAdd);
}

bool SonyDb::updSong(Song *songToUpd)
{
	if (this->copying)
	{
		for (vector<Song>::iterator song = songs_temporary.begin(); song != songs_temporary.end(); song++)
		{
			//song is the same
			if (song->sonyDbOrder == songToUpd->sonyDbOrder)
			{
				if (song->statusOfSong != REMOVE_FROM_DEVICE &&
					song->statusOfSong != EMPTYTRACK)
				{
					song->artist = strdup(songToUpd->artist);
					song->album = strdup(songToUpd->album);
					song->genre = strdup(songToUpd->genre);
					song->title = strdup(songToUpd->title);

					song->track_nr = songToUpd->track_nr;
					song->songlen = songToUpd->songlen;
					song->year = songToUpd->year;
					return (true);
				}
				return (false);
			}
		}
	}
	else
	{
		for (vector<Song>::iterator song = songs.begin(); song != songs.end(); song++)
		{
			//song is the same
			if (song->sonyDbOrder == songToUpd->sonyDbOrder)
			{
				if (song->statusOfSong != REMOVE_FROM_DEVICE &&
					song->statusOfSong != EMPTYTRACK)
				{
					song->artist = strdup(songToUpd->artist);
					song->album = strdup(songToUpd->album);
					song->genre = strdup(songToUpd->genre);
					song->title = strdup(songToUpd->title);

					song->track_nr = songToUpd->track_nr;
					song->songlen = songToUpd->songlen;
					song->year = songToUpd->year;
					return (true);
				}
				return (false);
			}
		}
	}
	return (0);
}

int SonyDb::delSong(Song *songToDel)
{
	if (this->copying)
	{
		for (vector<Song>::iterator song = songs_temporary.begin(); song != songs_temporary.end(); song++)
		{
			//song is the same
			if ((STRCMP2_NULLOK((*song).album, (songToDel)->album) == 0) &&
					(STRCMP2_NULLOK((*song).artist, (songToDel)->artist) == 0) &&
					(STRCMP2_NULLOK((*song).title, (songToDel)->title) == 0))
			{
				if ((*song).statusOfSong == ADD_TO_DEVICE) //song is not yet on the device
				{
					//remove it from the list
					songs_temporary.erase(song);
					nbTrackToAdd--;
					//size of the song
					struct stat results;
					if (stat((*songToDel).filename, &results) == 0)
						addTrackTotalByte -= results.st_size;
					return (2);
				}
			}
		}
	}
	else
	{
		for (vector<Song>::iterator song = songs.begin(); song != songs.end(); song++)
		{
			//song is the same
			if ((STRCMP2_NULLOK((*song).album, (songToDel)->album) == 0) &&
					(STRCMP2_NULLOK((*song).artist, (songToDel)->artist) == 0) &&
					(STRCMP2_NULLOK((*song).title, (songToDel)->title) == 0))
			{
				if ((*song).statusOfSong == ADD_TO_DEVICE) //song is not yet on the device
				{
					//remove it from the list
					songs.erase(song);
					nbTrackToAdd--;
					//size of the song
					struct stat results;
					if (stat((*songToDel).filename, &results) == 0)
						addTrackTotalByte -= results.st_size;
					return (2);
				}
				if ((*song).statusOfSong == ON_DEVICE)
				{
					(*song).statusOfSong = REMOVE_FROM_DEVICE;
					nbTrackToDel++;
					//size of the song
					struct stat results;
					if (stat((*songToDel).filename, &results) == 0)
						delTrackTotalByte += results.st_size;
					return (1);
				}
			}
		}
	}
	return (0);
}

int SonyDb::getNbPlaylist()
{
	return(this->playlist.size());
}

bool SonyDb::addPlaylist(Playlist *p)
{
	if (this->copying)
		playlist_temporary.push_back(*p);
	else
		playlist.push_back(*p);
	return (true);
}

bool SonyDb::addSong(Song *songToAdd)
{
	//check how much space is needed
	struct stat results;
	if (stat((*songToAdd).filename, &results) == 0)
		addTrackTotalByte += results.st_size;

	if (this->copying)
	{
		//add the song to the temporary list
		songToAdd->sonyDbOrder = 0;
		songs_temporary.push_back(*songToAdd);
		nbTrackToAdd++;
	}
	else
	{
		//check if song is already present on device
		for (vector<Song>::iterator song = songs.begin(); song != songs.end(); song++)
		{
			//song is the same
			if ((STRCMP2_NULLOK((*song).album, (songToAdd)->album) == 0) &&
					(STRCMP2_NULLOK((*song).artist, (songToAdd)->artist) == 0) &&
					(STRCMP2_NULLOK((*song).title, (songToAdd)->title) == 0))
			{
				if ((*song).statusOfSong != ADD_TO_DEVICE)
					(*song).statusOfSong = ON_DEVICE;
				free(songToAdd->filename);
				songToAdd->filename = strdup((*song).filename);
				return (false);
			}
		}

		//add the song
		nbTrackToAdd++;
		songToAdd->sonyDbOrder = 0; //changed when copied to the device in writeTracks
		songs.push_back(*songToAdd);
	}

	return (true);
}




/*****************************************************************************************/
/** read/write functions (common header) */

bool SonyDb::getHeader(sonyFileHeader *header, FILE *f)
{
	//read header file
	if ( fread(header, sizeof(sonyFileHeader), 1, f ) != 1)
	{
		fprintf(fp, "error could not read file header (fileheader)\n");
		fflush(fp);
		return false;
	}
	/*  else
		{
		fprintf(fp, "header\n: magic:%08x, cte: %08x ,count:%08x \n", header->magic, header->cte, header->count);

		}
		*/

	return true;
}

bool SonyDb::getObjectPointer(sonyObjectPointer *Opointer, FILE *f)
{
	//read object pointer
	if ( fread(Opointer, sizeof(sonyObjectPointer), 1, f ) != 1 )
	{
		fprintf(fp, "error could not read file header (filePointer)\n");
		fflush(fp);
		return false;
	}
	else
	{
		//fprintf(fp, "objectPointer: magic:%x length:%x, offset:%x\n", Opointer->magic, Opointer->length, Opointer->offset);
		//get values from big endian
		Opointer->length = UINT32_SWAP_BE_LE(Opointer->length);
		Opointer->offset = UINT32_SWAP_BE_LE(Opointer->offset);
		//fprintf(fp, "sizeof: %i, objectPointer\n: magic:%s length:%08x, offset:%08x\n",sizeof(sonyObjectPointer), Opointer->magic, Opointer->length, Opointer->offset);
	}
	return true;
}

bool SonyDb::getObject(sonyObject *obj, FILE *f)
{
	//read object
	if ( fread(obj, sizeof(sonyObject), 1, f ) !=1 )
	{
		fprintf(fp, "error could not read file header (object)\n");
		fflush(fp);
		return false;
	}
	else
	{
		//fprintf(fp, "object: magic:%x size :%x, count:%x \n", obj->magic, obj->size, obj->count);
		//get values from big endian
		obj->size = UINT16_SWAP_BE_LE(obj->size);
		obj->count = UINT16_SWAP_BE_LE(obj->count);
		//fprintf(fp, "sizeof: %i, object\n: magic:%s size :%08x, count:%08x \n", sizeof(sonyObject), obj->magic, obj->size, obj->count);
	}   
	return true;
}

// write a file header
bool SonyDb::writeHeader(sonyFileHeader *h, FILE *f)
{
	return(writeHeader(h, f, 1));
}

// write a file header specifying the object pointer count
bool SonyDb::writeHeader(sonyFileHeader *h, FILE *f, int count)
{
	h->cte[0] = 0x01;
	h->cte[1] = 0x01;
	h->cte[2] = 0x00;
	h->cte[3] = 0x00;
	h->count = count;
	for (int i = 0; i < 7; i++)
		h->padding[i]= 0x00;

	if (fwrite(h, sizeof(sonyFileHeader), 1, f) == 1)
		return (true);
	else
		return (false);
}

// write an object pointer header
bool SonyDb::writeObjectPointer(sonyObjectPointer *p, FILE *f)
{
	p->padding = 0x00000000;

	if (fwrite(p, sizeof(sonyFileHeader), 1, f) == 1)
		return (true);
	else
		return (false);
	return (true);
}

//internal write an object header
bool SonyDb::writeObject(sonyObject *obj, FILE *f)
{
	if (fwrite(obj, sizeof(sonyFileHeader), 1, f) == 1)
		return (true);
	else
		return (false);
}


/*****************************************************************************************/

//internal write a track header
bool SonyDb::writeTrackHeader(sonyTrack *t, FILE *f)
{
	if (fwrite(t, sizeof(sonyTrack), 1, f) != 1)
	{
		fprintf(fp, "error could not write file header (object)\n");
		fflush(fp);
		return false;
	}
	return (true);
}

//internal write a track tag
bool SonyDb::writeTrackTag(sonyTrackTag *tt, char *input, FILE *f)
{  
	//init
	int size = TAGSIZE;
	utf16char *tagRecord;

	//to utf16
	tagRecord = ansi_to_utf16(input, (size - 6), true);

	//write tracktag (tagtype + encoding)
	if (fwrite(tt, sizeof(sonyTrackTag), 1, f) != 1) return (false);

	//write tag (data itself)
	if (fwrite(tagRecord, (size - 6), 1, f) != 1) return (false);
	free(tagRecord);

	return (true);
}

void SonyDb::setTable(char *TableFileName, int type)
{
	this->decodeTableFilename = strdup(TableFileName);
	this->codeType = type;

	if (type == ENCODING_USE_KEY)
	{
		FILE *t = fopen(decodeTableFilename, "rb");
		fseek(t, 0x0a, SEEK_SET);
		fread(&(this->DvId), sizeof(uint32), 1, t);
		this->DvId = UINT32_SWAP_BE_LE(this->DvId);
		fclose(t);
	}
}



//ported from gym
bool SonyDb::loadCodeTable(int id)
{
	FILE *t;
	uint8 *header1 = (uint8*)malloc(sizeof(uint8) * 134); //deserialize manually ...
	uint8 *header2 = (uint8*)malloc(sizeof(uint8) * 10);
	uint8 *tmp = (uint8*)malloc(sizeof(uint8) * 256);

	t = fopen(decodeTableFilename, "rb");
	// place cursor in place...
	fseek(t, 0x4B0 * id, SEEK_SET);
	fread(header1, sizeof(uint8), 134, t);

	for (int i = 0; i < 4; i++)
	{
		fread(tmp, sizeof(uint8), 256, t);
		for (int j = 0; j < 256; j++)
			codeTable[(i*256) + j] = tmp[j];
		fread(header2, sizeof(uint8), 10, t);
	}

	free(header1);
	free(header2);
	free(tmp);
	fclose(t);
	return true;
}

//add mp3 file to device
bool SonyDb::addOMA(Song *s, int destination)
{
	uint32    key = 0xFFFFFFFF;
	bool      isVBR = false;

	this->copyPercent = 0;
	if (s->statusOfSong != ADD_TO_DEVICE)
		return false;

	char *filename = GetOMAFilename(destination);

	fprintf(fp, "sending %s to %s\n", s->filename, filename);
	fflush(fp);

	//createDirectory if necessary
	createDir(destination);

	//load decode table for this track
	if (this->codeType == ENCODING_USE_TABLE)
		this->loadCodeTable(destination - 1);

	//or use key
	if (this->codeType == ENCODING_USE_KEY)
		key = ( 0x2465 + destination * 0x5296E435 ) ^ this->DvId;


	FILE *fout = fopen(filename, "wb");
	if (fout == NULL)
	{
		fprintf(fp, "error can't open file %s\n", filename);
		fflush(fp);
		return false;
	}

	FILE *fin = fopen(s->filename, "rb");
	float finSize = 0;
	if (fin == NULL)
	{
		fprintf(fp, "error can't open file %s\n", s->filename);
		fflush(fp);
		fclose(fout);
		return false;
	}

	//get input file size
	struct stat results;
	if (stat(s->filename, &results) == 0)
		finSize = results.st_size;

	uint8	    *header = (uint8*)malloc(sizeof(uint8) * 11);
	utf16char	    *tagRecord;
	char	    *tmpTag = (char*)malloc(sizeof(char) * 256);
	int	    tagLength;
	int	    headerLength = 0;
	long	    startPoint = 0;
	int	    nbFrames = 0; // estimated number of frames

	memset(header, 0, 11);
	if (fread(header, 1, 10, fin) != 10)
		return false;
	if (STRNCMP_NULLOK((char*)header, "ID3", 3) == 0)
	{
		//skip Idv2 tags
		startPoint = ((header[6] << 21) + (header[7] << 14) + (header[8] << 7) + header[9]) + 10;

	}
	else
	{
		//no idv2 tag found
		startPoint = 0;
	}

	//go to the first frame
	fseek(fin, startPoint, SEEK_SET);

	memset(header, 0, 11);
	//skip zeros
	while (header[0] == 0)
	{
		if(fread(header, sizeof(uint8), 1, fin) != 1)
		{
			fclose(fout);
			fclose(fin);
			return false;
		}
	}

	if (header[0] == 0xFF)
	{
		if (fread(header, sizeof(uint8), 3, fin) != 3)
		{
			fclose(fout);
			fclose(fin);
			return false;
		}

		//get mpeg type, layer type and bitrate
		if ((header[0] & 0xE0) == 0xE0) //we found the first frame
		{
			//encoding =  mpeg version(2bits), layer version(2bits), bitrate(4bits)
			s->encoding = ( ((header[0] & 0x1E) << 3) + ((header[1] & 0xF0) >> 4) );

			// 00 - MPEG Version 2.5 (unofficial extension of MPEG 2)
			// 01 - reserved
			// 10 - MPEG Version 2 (ISO/IEC 13818-3)
			// 11 - MPEG Version 1 (ISO/IEC 11172-3) 
			uint8 mpegVersion = (header[0] & 0x18) >> 3;

			//00 - reserved
			//01 - Layer III
			//10 - Layer II
			//11 - Layer I
			uint8 layerVersion = (header[0] & 0x06) >> 1;

			uint8 samplingRateIndex = (header[1] & 0xC) >> 2;

			if (((mpegVersion * 3) + samplingRateIndex >= 12) || ((mpegVersion * 3) + layerVersion >= 16))
			{
				//header is invalid
				nbFrames = 0;
			}
			else
			{
				int  SAMPLING_RATES[] = {11025, 12000, 8000, 0, 0, 0, 22050, 24000, 16000, 44100, 48000, 32000};
				int  SAMPLE_PER_FRAME[] = {0,576,1152,384,0,0,0,0,0,576,1152,384,0,1152,1152,384};

				//sample per frame 0=reserved
				//          MPG2.5 res        MPG2   MPG1 
				//reserved  0      0          0      0
				//Layer III 576    0          576    1152 	
				//Layer II  1152   0          1152   1152 	
				//Layer I   384    0          384    384 

				int samplingRate = SAMPLING_RATES[(mpegVersion * 3) + samplingRateIndex];
				int samplePerFrame = SAMPLE_PER_FRAME[(mpegVersion * 4) + layerVersion];
				nbFrames = (s->songlen * samplingRate) / samplePerFrame;
			}

			//skip the the frame header
			fseek(fin, sizeof(uint8) * 32, SEEK_CUR);

			//check if first is "XING" for VBR files
			memset(header, 0, 11);
			if (fread(header, sizeof(uint8), 4, fin) != 4)
			{
				fclose(fout);
				fclose(fin);
				return false;
			}

			if (STRNCMP_NULLOK((char*)header, "XING", 4) == 0)
				isVBR = true;
		}
		else
		{
			fprintf(fp, "File format invalid, could not find first frame%s\n", s->filename);
			fflush(fp);
			//return false;
		}
	}
	else
	{
		fprintf(fp, "File format invalid, could not find first frame%s\n", s->filename);
		fflush(fp);
		//return false;
	}



	//go back to the start of mp3 data
	fseek(fin, startPoint, SEEK_SET);

	//write the OMA headers :
	//format is :
	//"ea3"0x03 (4bytes), sizeOfTotaltags (6bytes)
	//TIT2(4bytes) sizeOfTag(4bytes) 2flags (2bytes) 0x02(=utf16be format?) titleOftheSong
	//...
	//not all size are coded in synchsafe format


	//idv2 header
	memset(header, 0, 11);
	header[0] = 'e';
	header[1] = 'a';
	header[2] = '3';
	header[3] = 0x03;
	//...zeros
	header[8] = 0x17;
	header[9] = 0x76; //size of tag header in Synchsafe format (=3072 bytes - 10 of header)
	if (fwrite(header, sizeof(uint8), 10, fout) != 10)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}

	//title tag
	memset(header, 0, 11);
	memcpy(header, "TIT2", 4);
	tagLength = strlen(s->title);
	tagRecord = ansi_to_utf16(s->title, tagLength + 1, true);
	tagLength = (tagLength * 2) + 1;
	header[4] = NOT_SYNCHSAFE_B1(tagLength);//size of the title
	header[5] = NOT_SYNCHSAFE_B2(tagLength);
	header[6] = NOT_SYNCHSAFE_B3(tagLength);
	header[7] = NOT_SYNCHSAFE_B4(tagLength);
	header[8] = 0;   //flag 1
	header[9] = 0;  //flag 2
	header[10] = 0x02;  //
	tagLength = (tagLength - 1 )/ 2;
	if (fwrite(header, sizeof(uint8), 11, fout) != 11)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (11 * sizeof(uint8));
	if (fwrite(tagRecord, sizeof(utf16char), tagLength, fout) != tagLength)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (tagLength * sizeof(utf16char));
	free(tagRecord);


	//artist tag
	memset(header, 0, 11);
	memcpy(header, "TPE1", 4);
	tagLength = strlen(s->artist);
	tagRecord = ansi_to_utf16(s->artist, tagLength + 1, true);
	tagLength = (tagLength * 2) + 1;
	header[4] = NOT_SYNCHSAFE_B1(tagLength);//size of the title
	header[5] = NOT_SYNCHSAFE_B2(tagLength);
	header[6] = NOT_SYNCHSAFE_B3(tagLength);
	header[7] = NOT_SYNCHSAFE_B4(tagLength);
	header[8] = 0;   //flag 1
	header[9] = 0;  //flag 2
	header[10] = 0x02;  //
	tagLength = (tagLength - 1 )/ 2;
	if (fwrite(header, sizeof(uint8), 11, fout) != 11)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (11 * sizeof(uint8));
	if (fwrite(tagRecord, sizeof(utf16char), tagLength, fout) != tagLength)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (tagLength * sizeof(utf16char));
	free(tagRecord);

	//album tag
	memset(header, 0, 11);
	memcpy(header, "TALB", 4);
	tagLength = strlen(s->album);
	tagRecord = ansi_to_utf16(s->album, tagLength + 1, true);
	tagLength = (tagLength * 2) + 1;
	header[4] = NOT_SYNCHSAFE_B1(tagLength);//size of the title
	header[5] = NOT_SYNCHSAFE_B2(tagLength);
	header[6] = NOT_SYNCHSAFE_B3(tagLength);
	header[7] = NOT_SYNCHSAFE_B4(tagLength);
	header[8] = 0;  //flag 1
	header[9] = 0;  //flag 2
	header[10] = 0x02;  //
	tagLength = (tagLength - 1 )/ 2;
	if (fwrite(header, sizeof(uint8), 11, fout) != 11)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (11 * sizeof(uint8));
	if (fwrite(tagRecord, sizeof(utf16char), tagLength, fout) != tagLength)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (tagLength * sizeof(utf16char));
	free(tagRecord);

	//genre tag
	memset(header, 0, 11);
	memcpy(header, "TCON", 4);
	tagLength = strlen(s->genre);
	tagRecord = ansi_to_utf16(s->genre, tagLength + 1, true);
	tagLength = (tagLength * 2) + 1;
	header[4] = NOT_SYNCHSAFE_B1(tagLength);//size of the title
	header[5] = NOT_SYNCHSAFE_B2(tagLength);
	header[6] = NOT_SYNCHSAFE_B3(tagLength);
	header[7] = NOT_SYNCHSAFE_B4(tagLength);
	header[8] = 0;   //flag 1
	header[9] = 0;  //flag 2
	header[10] = 0x02;  //
	tagLength = (tagLength - 1 )/ 2;
	if (fwrite(header, sizeof(uint8), 11, fout) != 11)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (11 * sizeof(uint8));
	if (fwrite(tagRecord, sizeof(utf16char), tagLength, fout) != tagLength)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (tagLength * sizeof(utf16char));
	free(tagRecord);

	//track number tag //space are suppose to be replaced by 0x00 not 0x20
	sprintf(tmpTag, "OMG_TRACK %02i", s->track_nr);
	memset(header, 0, 11);
	memcpy(header, "TXXX", 4);
	tagLength = strlen(tmpTag);
	tagRecord = ansi_to_utf16(tmpTag, tagLength + 1, true);
	tagLength = (tagLength * 2) + 1;
	header[4] = NOT_SYNCHSAFE_B1(tagLength);//size of the title
	header[5] = NOT_SYNCHSAFE_B2(tagLength);
	header[6] = NOT_SYNCHSAFE_B3(tagLength);
	header[7] = NOT_SYNCHSAFE_B4(tagLength);
	header[8] = 0;   //flag 1
	header[9] = 0;  //flag 2
	header[10] = 0x02;  //
	tagLength = (tagLength - 1 )/ 2;
	if (fwrite(header, sizeof(uint8), 11, fout) != 11)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (11 * sizeof(uint8));
	if (fwrite(tagRecord, sizeof(utf16char), tagLength, fout) != tagLength)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (tagLength * sizeof(utf16char));
	free(tagRecord);

	//fixme date of transfert? tag //space are suppose to be replaced by 0x00 not 0x20
	sprintf(tmpTag, "OMG_TRLDA 2001/01/01 00:00:00");
	memset(header, 0, 11);
	memcpy(header, "TXXX", 4);
	tagLength = strlen(tmpTag);
	tagRecord = ansi_to_utf16(tmpTag, tagLength + 1, true);
	tagLength = (tagLength * 2) + 1;
	tagLength = (strlen(tmpTag) + 1) * 2;
	header[4] = NOT_SYNCHSAFE_B1(tagLength);//size of the title
	header[5] = NOT_SYNCHSAFE_B2(tagLength);
	header[6] = NOT_SYNCHSAFE_B3(tagLength);
	header[7] = NOT_SYNCHSAFE_B4(tagLength);
	header[8] = 0;   //flag 1
	header[9] = 0;  //flag 2
	header[10] = 0x02;  //
	tagLength = (tagLength - 1 )/ 2;
	if (fwrite(header, sizeof(uint8), 11, fout) != 11)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (11 * sizeof(uint8));
	if (fwrite(tagRecord, sizeof(utf16char), tagLength, fout) != tagLength)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (tagLength * sizeof(utf16char));
	free(tagRecord);

	//track length tag
	sprintf(tmpTag, "%i", s->songlen * 1000);
	memset(header, 0, 11);
	memcpy(header, "TLEN", 4);
	tagLength = strlen(tmpTag);
	tagRecord = ansi_to_utf16(tmpTag, tagLength + 1, true);
	tagLength = (tagLength * 2) + 1;
	header[4] = NOT_SYNCHSAFE_B1(tagLength);//size of the title
	header[5] = NOT_SYNCHSAFE_B2(tagLength);
	header[6] = NOT_SYNCHSAFE_B3(tagLength);
	header[7] = NOT_SYNCHSAFE_B4(tagLength);
	header[8] = 0;   //flag 1
	header[9] = 0;  //flag 2
	header[10] = 0x02;  //
	tagLength = (tagLength - 1 )/ 2;
	if (fwrite(header, sizeof(uint8), 11, fout) != 11)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (11 * sizeof(uint8));
	if (fwrite(tagRecord, sizeof(utf16char), tagLength, fout) != tagLength)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (tagLength * sizeof(utf16char));
	free(tagRecord);

	memset(header, 0, 11);
	//fill the rest with 0
	while (headerLength < 3062)
	{
		headerLength += sizeof(uint8);
		if (fwrite(header, sizeof(uint8), 1, fout) != 1)
		{
			fclose(fout);
			fclose(fin);
			return false;
		}
	}

	//write second header fixme (some stuff are missing here... important?)
	uint8 *header2 = (uint8*)malloc(sizeof(uint8) * 16);
	headerLength = 0;
	memset(header2, 0, 16);
	//first line
	memcpy(header2, "EA3", 3);
	header2[3] = 0x02;
	header2[4] = 0;   //size of 2nd header
	header2[5] = 0x60;//size of 2nd header
	header2[6] = 0xff;// + same value as in 05CIDLST.DAT
	if (this->codeType == ENCODING_USE_NONE)
	{
		header2[7] = 0xff;// cp or sonicstage? or encoded not encoded? 505 =e  1000 =f
	}
	else
	{
		header2[7] = 0xfe;// cp or sonicstage? or encoded not encoded? 505 =e  1000 =f
	} 
	header2[12] = 0x01;
	header2[13] = 0x0F;
	header2[14] = 0x50;
	header2[15] = 0x00;// - same value as in 05CIDLST.DAT
	if (fwrite(header2, sizeof(uint8), 16, fout) != 16)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (16 * sizeof(uint8));

	//second line
	memset(header2, 0, 16);
	header2[1] = 0x04;// + same value as in 05CIDLST.DAT fixme
	//zeros...   
	header2[5] = 0x01; //fixme 01
	header2[6] = 0x02; //fixme 02
	header2[7] = 0x03; //fixme 03
	header2[8] = 0xc8;
	header2[9] = 0xd8;
	header2[10] = 0x36;
	header2[11] = 0xd8;
	header2[12] = 0x11; //fixme 11
	header2[13] = 0x22; //fixme 22
	header2[14] = 0x33; //fixme 33
	header2[15] = 0x44;// - same value as in 05CIDLST.DAT
	if (fwrite(header2, sizeof(uint8), 16, fout) != 16)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (16 * sizeof(uint8));

	//third line
	memset(header2, 0, 16); 

	header2[0] = 0x03;// 3 = MP3
	header2[1] =(isVBR) ? 0x90 : 0x80 ;// VBR = 90, CBR = 80
	header2[2] = s->encoding;// mpeg version(2bits), layer version(2bits), bitrate(4bits)
	header2[3] = 0x10;//?? fixme

	//tracklength
	uint32 trackLengh = s->songlen * 1000; 
	header2[4] = (uint8) (((trackLengh) & (uint32) 0xff000000U) >> 24);
	header2[5] = (uint8) (((trackLengh) & (uint32) 0x00ff0000U) >> 16);
	header2[6] = (uint8) (((trackLengh) & (uint32) 0x0000ff00U) >>  8);
	header2[7] = (uint8) ((trackLengh) & (uint32)  0x000000ffU);

	//number of frames
	header2[8] = (uint8) (((nbFrames) & (uint32) 0xff000000U) >> 24);
	header2[9] = (uint8) (((nbFrames) & (uint32) 0x00ff0000U) >>  16);
	header2[10] = (uint8) (((nbFrames) & (uint32) 0x0000ff00U) >>  8);
	header2[11] = (uint8) ((nbFrames) & (uint32)  0x000000ffU); 

	//padding
	header2[12] = 0x00;
	header2[13] = 0x00;
	header2[14] = 0x00;
	header2[15] = 0x00;
	if (fwrite(header2, sizeof(uint8), 16, fout) != 16)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	headerLength += (16 * sizeof(uint8));

	//padding
	memset(header2, 0, 16);
	if (fwrite(header2, sizeof(uint8), 16, fout) != 16)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	if (fwrite(header2, sizeof(uint8), 16, fout) != 16)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}
	if (fwrite(header2, sizeof(uint8), 16, fout) != 16)
	{
		fclose(fout);
		fclose(fin);
		return false;
	}

	free(header2);
	free(header);
	free(tmpTag);


	int   BLOCK_SIZE = 32767;
	int   blockNumber = 1;
	uint8 *inputData = (uint8 *)malloc(sizeof(uint8) * BLOCK_SIZE);
	uint8 *outputData = (uint8 *)malloc(sizeof(uint8) * BLOCK_SIZE);
	int   nbRead;
	long  position = 0;

	// input file -> decode -> output file
	while ((nbRead = fread(inputData, 1, BLOCK_SIZE, fin)) != 0)
	{ 
		for (int i = 0; i < nbRead; i++)
		{	  
			if (this->codeType == ENCODING_USE_NONE)//just copy without decoding
				outputData[i] = inputData[i]; 
			if (this->codeType == ENCODING_USE_TABLE) //keyEncodeTable.dat
				outputData[i] = codeTable[((position % 4) * 256) + inputData[i]];
			if (this->codeType == ENCODING_USE_KEY) //DvId.dat
			{
				if ((position % 4) == 0) outputData[i] = ((inputData[i]) ^ ((key & 0xFF000000) >> 24));
				if ((position % 4) == 1) outputData[i] = ((inputData[i]) ^ ((key & 0x00FF0000) >> 16));
				if ((position % 4) == 2) outputData[i] = ((inputData[i]) ^ ((key & 0x0000FF00) >> 8));
				if ((position % 4) == 3) outputData[i] = ((inputData[i]) ^ (key & 0x000000FF));
			}
			position++;
		}
		if (fwrite(outputData, sizeof(uint8), nbRead, fout) != nbRead)
		{
			fclose(fout);
			fclose(fin);
			return false;
		}
		blockNumber++;
		totalByteLeftToWrite -= BLOCK_SIZE;

		if ((finSize > 0) && (BLOCK_SIZE > 0))
			this->copyPercent += 100 / (finSize / BLOCK_SIZE);
	}
	this->copyPercent = 100;
	//s->statusOfSong = ON_DEVICE;

	free(inputData);
	free(outputData);

	fclose(fout);
	fclose(fin);
	return (true);
}

//returns the number of tracks to be added/deleted/kept
int  SonyDb::getNumberOfTracks()
{
	return (songs.size());
}

//returns the copy progress for the current file
int  SonyDb::getCopyPercent()
{
	return (int)(this->copyPercent);
}

//some code is from GYM
bool SonyDb::getOMA(Song *s, char *destination)
{
	if (s->statusOfSong == ADD_TO_DEVICE)
		return (false);

	char	    filename[512];
	uint8	    *header = (uint8*)malloc(sizeof(uint8) * 11);
	char	    *tmpTag = (char*)malloc(sizeof(char) * 256);
	int	    tagLength;
	int	    headerLength = 0;
	uint32	    key = 0xFFFFFFFF;

	//open the file
	sprintf(filename, "%s%s - %s - %02i - %s.mp3", destination, s->album, s->artist, s->track_nr, s->title);

	fprintf(fp, "getting file from %s to %s\n", s->filename, filename);
	fflush(fp);

	FILE *fout = fopen(filename, "wb");
	if (fout == NULL)
	{
		fprintf(fp, "error can't open file %s\n", filename);fflush(fp);
		return false;
	}

	FILE *fin = fopen(s->filename, "rb");
	if (fin == NULL)
	{
		fprintf(fp, "error can't open file %s\n", s->filename);fflush(fp);
		return false;
	}

	//load decode table for this track
	if (this->codeType == ENCODING_USE_TABLE)
		this->loadCodeTable(s->sonyDbOrder - 1);

	//or use key
	if (this->codeType == ENCODING_USE_KEY)
		key = ( 0x2465 + s->sonyDbOrder * 0x5296E435 ) ^ this->DvId;

	//reserve 10bytes for the header
	memset(header, 0, 11);
	fwrite(header, sizeof(uint8), 10, fout);

	//title tag
	memset(header, 0, 11);
	memcpy(header, "TIT2", 4);
	tagLength = strlen(s->title) + 1;
	header[4] = SYNCHSAFE_B1(tagLength);//size of the title
	header[5] = SYNCHSAFE_B2(tagLength);
	header[6] = SYNCHSAFE_B3(tagLength);
	header[7] = SYNCHSAFE_B4(tagLength);
	fwrite(header, sizeof(uint8), 11, fout);
	headerLength += (11 * sizeof(uint8));
	fwrite(s->title, sizeof(uint8), (tagLength - 1), fout);
	headerLength += ((tagLength - 1) * sizeof(uint8));


	//artist tag
	memset(header, 0, 11);
	memcpy(header, "TPE1", 4);
	tagLength = strlen(s->artist) + 1;
	header[4] = SYNCHSAFE_B1(tagLength);//size of the title
	header[5] = SYNCHSAFE_B2(tagLength);
	header[6] = SYNCHSAFE_B3(tagLength);
	header[7] = SYNCHSAFE_B4(tagLength);
	fwrite(header, sizeof(uint8), 11, fout);
	headerLength += (11 * sizeof(uint8));
	fwrite(s->artist, sizeof(uint8), (tagLength - 1), fout);
	headerLength += ((tagLength - 1) * sizeof(uint8));


	//album tag
	memset(header, 0, 11);
	memcpy(header, "TALB", 4);
	tagLength = strlen(s->album) + 1;
	header[4] = SYNCHSAFE_B1(tagLength);//size of the title
	header[5] = SYNCHSAFE_B2(tagLength);
	header[6] = SYNCHSAFE_B3(tagLength);
	header[7] = SYNCHSAFE_B4(tagLength);
	fwrite(header, sizeof(uint8), 11, fout);
	headerLength += (11 * sizeof(uint8));
	fwrite(s->album, sizeof(uint8), (tagLength - 1), fout);
	headerLength += ((tagLength - 1) * sizeof(uint8));

	//track number tag
	sprintf(tmpTag, "%02i", s->track_nr);
	memset(header, 0, 11);
	memcpy(header, "TRCK", 4);
	tagLength = strlen(tmpTag) + 1;
	header[4] = SYNCHSAFE_B1(tagLength);//size of the title
	header[5] = SYNCHSAFE_B2(tagLength);
	header[6] = SYNCHSAFE_B3(tagLength);
	header[7] = SYNCHSAFE_B4(tagLength);
	fwrite(header, sizeof(uint8), 11, fout);
	headerLength += (11 * sizeof(uint8));
	fwrite(tmpTag, sizeof(uint8), (tagLength - 1), fout);
	headerLength += ((tagLength - 1) * sizeof(uint8));

	//year tag
	if (s->year > 0)
	{
		sprintf(tmpTag, "%i", s->year);
		memset(header, 0, 11);
		memcpy(header, "TYER", 4);
		tagLength = strlen(tmpTag) + 1;
		header[4] = SYNCHSAFE_B1(tagLength);//size of the title
		header[5] = SYNCHSAFE_B2(tagLength);
		header[6] = SYNCHSAFE_B3(tagLength);
		header[7] = SYNCHSAFE_B4(tagLength);
		fwrite(header, sizeof(uint8), 11, fout);
		headerLength += (11 * sizeof(uint8));
		fwrite(tmpTag, sizeof(uint8), (tagLength - 1), fout);
		headerLength += ((tagLength - 1) * sizeof(uint8));
	}

	//genre tag
	memset(header, 0, 11);
	memcpy(header, "TCON", 4);
	tagLength = strlen(s->genre) + 1;
	header[4] = SYNCHSAFE_B1(tagLength);//size of the title
	header[5] = SYNCHSAFE_B2(tagLength);
	header[6] = SYNCHSAFE_B3(tagLength);
	header[7] = SYNCHSAFE_B4(tagLength);
	fwrite(header, sizeof(uint8), 11, fout);
	headerLength += (11 * sizeof(uint8));
	fwrite(s->genre, sizeof(uint8), (tagLength - 1), fout);
	headerLength += ((tagLength - 1) * sizeof(uint8));

	memset(header, 0, 11);
	//fill the rest with 0
	while ((headerLength %16) != 0)
	{
		headerLength += sizeof(uint8);
		fwrite(header, sizeof(uint8), 1, fout);
	}
	//return to the start of the file to write the header;
	fseek(fout, 0, SEEK_SET);
	memset(header, 0, 11);
	header[0] = 'I';
	header[1] = 'D';
	header[2] = '3';
	header[3] = 0x03;
	//...zeros
	header[6] = SYNCHSAFE_B1(headerLength);
	header[7] = SYNCHSAFE_B2(headerLength);
	header[8] = SYNCHSAFE_B3(headerLength);
	header[9] = SYNCHSAFE_B4(headerLength);
	fwrite(header, sizeof(uint8), 10, fout);

	//return after the header
	fseek(fout, headerLength, SEEK_CUR);

	free(header);
	free(tmpTag);

	//skip the oma tags
	fseek(fin, 0xC60, SEEK_SET);

	int   BLOCK_SIZE = 32767;
	int   blockNumber = 1;
	uint8 *inputData = (uint8 *)malloc(sizeof(uint8) * BLOCK_SIZE);
	uint8 *outputData = (uint8 *)malloc(sizeof(uint8) * BLOCK_SIZE);
	int   nbRead;
	long  position = 0;

	// input file -> decode -> output file
	while ((nbRead = fread(inputData, 1, BLOCK_SIZE, fin)) != 0)
	{ 
		for (int i = 0; i < nbRead; i++)
		{
			if (this->codeType == ENCODING_USE_NONE) //just copy without decoding
				outputData[i] = inputData[i]; 
			if (this->codeType == ENCODING_USE_TABLE) //decodeKeys.dat
				outputData[i] = codeTable[((position % 4) * 256) + inputData[i]];
			if (this->codeType == ENCODING_USE_KEY) //DvId.dat
			{
				if ((position % 4) == 0) outputData[i] = ((inputData[i]) ^ ((key & 0xFF000000) >> 24));
				if ((position % 4) == 1) outputData[i] = ((inputData[i]) ^ ((key & 0x00FF0000) >> 16));
				if ((position % 4) == 2) outputData[i] = ((inputData[i]) ^ ((key & 0x0000FF00) >> 8));
				if ((position % 4) == 3) outputData[i] = ((inputData[i]) ^ (key & 0x000000FF));
			}
			position++;
		}
		fwrite(outputData, sizeof(uint8), nbRead, fout);
		blockNumber++;
	}

	free(inputData);
	free(outputData);

	fclose(fout);
	fclose(fin);
	//sprintf(tmp, "del %s\n", filename);
	//system(tmp);
	return (true);
}

//delete OMA file from device
void SonyDb::deleteOMA(char *filename)
{
#ifdef _WIN32
	DeleteFile(filename);
#else
	unlink(filename);
#endif
}


void SonyDb::createDir(int value)
{
	char *dirName = (char*)calloc(sizeof(char), 256);

	sprintf(dirName, "%s/OMGAUDIO/10F%02x", getDriveLetter(), value >> 8);
#ifdef _WIN32
	CreateDirectory(dirName, NULL);
#else
#ifdef __MINGW32__
	mkdir(dirName);
#else
	mkdir(dirName, 0755);
#endif
#endif
	free(dirName);
}

bool SonyDb::writeTracks()
{
	//already copying
	if (this->copying)
		return false;

	//nothing to do
	//if ((nbTrackToAdd <= 0) && (nbTrackToDel <= 0) && (getNbPlaylist() <= 0))

	this->copying = true;

	//concider all files are copied for size problems
	usedSpaceDisk = usedSpaceDisk + addTrackTotalByte - delTrackTotalByte;
	totalByteLeftToWrite = addTrackTotalByte;
	addTrackTotalByte = 0;
	delTrackTotalByte = 0;
	nbTrackToAdd = 0;
	nbTrackToDel = 0;


	vector<Song *> songlist;
	vector<Song *> addList;
	vector<Song *>::iterator iteAddList;
	bool res = false;

	//create list of add & del
	for (vector<Song>::iterator i = songs.begin(); i != songs.end(); i++)
	{
		if ((*i).statusOfSong == ADD_TO_DEVICE)
			addList.push_back(&(*i));
		//create directory if necessary
	}

	iteAddList = addList.begin();
	int nbElementsLeftToAdd = addList.size();

	this->copyIndex = 0;
	//create ordered list
	int test = songs.size();
	for (vector<Song>::iterator j = songs.begin(); j != songs.end(); j++)
	{ 
		//skip
		if ((*j).statusOfSong == ADD_TO_DEVICE)
			continue;

		//keep
		if ((*j).statusOfSong == ON_DEVICE)
		{
			this->copyIndex++; //progress bar index
			songlist.push_back(&(*j));
			continue;
		}

		//replace
		if (((*j).statusOfSong == REMOVE_FROM_DEVICE) || ((*j).statusOfSong == EMPTYTRACK))
		{
			if ((*j).statusOfSong == REMOVE_FROM_DEVICE)
			{
				this->copyIndex++; //progress bar index
				deleteOMA((*j).filename);
			}

			//add front of addList change filenumber and filename
			if (nbElementsLeftToAdd > 0)
			{
				this->copyIndex++; //progress bar index
				if (addOMA((*iteAddList), (*j).sonyDbOrder))//replace oma file by new mp3
				{ 
					if ((*iteAddList)->filename) free((*iteAddList)->filename);
					(*iteAddList)->filename = strdup((*j).filename);
					(*iteAddList)->sonyDbOrder = (*j).sonyDbOrder;
					(*j).statusOfSong = -1;
					//(*iteAddList)->statusOfSong = ON_DEVICE; //removed otherwise get added twice
					songlist.push_back((*iteAddList));
					iteAddList++;
					nbElementsLeftToAdd--;
				}
				else
				{
					fprintf(fp,"Could not write file : %s\n", (*iteAddList)->filename);
					fflush(fp);
					iteAddList++;
					nbElementsLeftToAdd--;
				}
			}
			else
			{
				if ((*j).statusOfSong == EMPTYTRACK)
				{
					//keep the empty track for now
					songlist.push_back(&(*j));
				}
				else
				{
					//nothing else to add, so add a blank element instead of deletion
					//ex : tracks 1 2 3 4 5 -> deletion of 2 and 5 becomes -> 1 'blank' 3 4
					//otherwise we would have to reencode 3 and 4 with the correct bit mask... 
					free((*j).album);
					free((*j).artist);
					free((*j).genre);
					free((*j).title);
					free((*j).filename);

					(*j).album = strdup("");
					(*j).artist = strdup("");
					(*j).genre = strdup("");
					(*j).title = strdup("");
					(*j).filename = strdup("");
					(*j).statusOfSong = EMPTYTRACK;
					(*j).songlen = 0;
					(*j).track_nr = 0;
					(*j).year = 0;
					songlist.push_back(&(*j));
				}
			}
		}
	}

	//add the rest of the add list at the end incrementing filenumber and filename
	while (nbElementsLeftToAdd > 0)
	{
		this->copyIndex++; //progress bar index
		if (addOMA((*iteAddList), lastTrackIndex)) //send the mp3
		{
			(*iteAddList)->filename = GetOMAFilename(lastTrackIndex);
			(*iteAddList)->sonyDbOrder = lastTrackIndex;
			(*iteAddList)->statusOfSong = ON_DEVICE;
			songlist.push_back((*iteAddList));
			lastTrackIndex++;
			nbElementsLeftToAdd--;
			iteAddList++;
		}
		else
		{
			fprintf(fp,"Could not write file : %s\n", (*iteAddList)->filename);
			fflush(fp);
			nbElementsLeftToAdd--;
			iteAddList++;
		}
	}
	int test2 = songlist.size();

	//remove empty tracks at the end of the list
	while ((songlist.size() > 0) && (songlist.back()->statusOfSong == EMPTYTRACK))
	{
		songlist.pop_back();
		if (this->lastTrackIndex -1 > 0)
			this->lastTrackIndex--;
	}

	//write the new database to the device
	res = writeDatabase(songlist);
	if (res)
		fprintf(fp,"Writing database files: OK\n");
	else
		fprintf(fp,"Writing database files: FAILED\n");
	fflush(fp);

	songlist.clear();
	addList.clear();

	this->copying = false;
	return (res);
}


//write the database to the device (overwrite existing db)
bool SonyDb::writeDatabase(vector<Song *> songsToSend)
{
	vector<Song *>     listArtist;
	vector<Song *>     listAlbum;
	vector<Song *>     listGenre;

	//create the lists
	bool alreadyAddedArtist = false;
	bool alreadyAddedAlbum = false;
	bool alreadyAddedGenre = false;

	//for each song
	int index = 1;
	for (vector<Song*>::iterator song = songsToSend.begin(); song != songsToSend.end(); song++)
	{
		alreadyAddedArtist = false;
		alreadyAddedAlbum = false;
		alreadyAddedGenre = false;

		//search in artist list
		for (vector<Song*>::iterator artist = listArtist.begin(); artist != listArtist.end(); artist++)
		{
			if (STRCMP2_NULLOK((*song)->artist, (*artist)->artist) == 0)
			{
				alreadyAddedArtist = true;
				break;
			}
		}

		//search in album list
		for (vector<Song*>::iterator album = listAlbum.begin(); album != listAlbum.end(); album++)
		{
			if (STRCMP2_NULLOK((*song)->album, (*album)->album) == 0)
			{
				alreadyAddedAlbum = true;
				break;
			}
		}	

		//search in the genre list
		for (vector<Song*>::iterator genre = listGenre.begin(); genre != listGenre.end(); genre++)
		{
			if (STRCMP2_NULLOK((*song)->genre, (*genre)->genre) == 0)
			{
				alreadyAddedGenre = true;
				break;
			}
		}

		//add it if not already in the list 
		if (!alreadyAddedArtist) listArtist.push_back((*song));
		if (!alreadyAddedGenre) listGenre.push_back((*song));
		if (!alreadyAddedAlbum) listAlbum.push_back((*song));
	}

	//resort the lists alphabetically
	sort(listArtist.begin(), listArtist.end(), sortByArtistName);
	sort(listGenre.begin(), listGenre.end(), sortByGenreName);
	sort(listAlbum.begin(), listAlbum.end(), sortByAlbumName);

	//to sort our songs by track number (if same album)
	bool first;
	int firstIndex = -1;
	int endIndex = -1;

	//now that we have listArtist listAlbum and listGenre
	//resort our songs
	vector<Song *> songsSortedByAlbum;
	for (vector<Song *>::iterator album = listAlbum.begin(); album != listAlbum.end(); album++)
	{
		first = true;
		for (vector<Song *>::iterator song = songsToSend.begin(); song != songsToSend.end(); song++)
		{
			if (STRCMP2_NULLOK((*song)->album, (*album)->album) == 0)
			{
				songsSortedByAlbum.push_back((*song));

				//for sorting by track number
				if (first)
				{
					first = false;
					firstIndex = endIndex + 1;
					endIndex = firstIndex;
				}
				else
				{
					endIndex++;
				}
			}
		}

		//sort by track number
		if (endIndex > firstIndex) 
			sort(songsSortedByAlbum.begin() + firstIndex, songsSortedByAlbum.begin() + endIndex + 1, sortByTrackNumber);
	}

	firstIndex = -1;
	endIndex = -1;
	vector<Song *> songsSortedByArtist;
	for (vector<Song *>::iterator artist = listArtist.begin(); artist != listArtist.end(); artist++)
	{
		first = true;
		for (vector<Song *>::iterator song = songsToSend.begin(); song != songsToSend.end(); song++)
		{

			if (STRCMP2_NULLOK((*song)->artist, (*artist)->artist) == 0)
			{
				songsSortedByArtist.push_back((*song));

				//for sorting by track number
				if (first)
				{
					first = false;
					firstIndex = endIndex + 1;
					endIndex = firstIndex;
				}
				else
				{
					endIndex++;
				}
			}
		}

		//sort by track number
		if (endIndex > firstIndex)
			sort(songsSortedByArtist.begin() + firstIndex, songsSortedByArtist.begin() + endIndex + 1, sortByTrackNumber);
	}

	firstIndex = -1;
	endIndex = -1;
	vector<Song *> songsSortedByGenre;
	for (vector<Song *>::iterator genre = listGenre.begin(); genre != listGenre.end(); genre++)
	{
		first = true;
		for (vector<Song *>::iterator song = songsToSend.begin(); song != songsToSend.end(); song++)
		{
			if (STRCMP2_NULLOK((*song)->genre, (*genre)->genre) == 0)
			{
				songsSortedByGenre.push_back((*song));

				//for sorting by track number
				if (first)
				{
					first = false;
					firstIndex = endIndex + 1;
					endIndex = firstIndex;
				}
				else
				{
					endIndex++;
				}
			}
		}

		//sort by track number
		if (endIndex > firstIndex)
			sort(songsSortedByGenre.begin() + firstIndex, songsSortedByGenre.begin() + endIndex + 1, sortByTrackNumber);
	}

	//write the files
	if (!(write_00GTRLST())) return false;

	if (!(write_01TREEXX(songsSortedByAlbum, listAlbum, 1))) return false;
	if (!(write_01TREEXX(songsSortedByArtist, listArtist, 2))) return false;
	if (!(write_01TREEXX(songsSortedByAlbum, listAlbum, 3))) return false;
	if (!(write_01TREEXX(songsSortedByGenre, listGenre, 4))) return false;

	// danger
	if (!(write_02TREINF(songsToSend))) return false;

	if (!(write_03GINFXX(listAlbum, 1))) return false;
	if (!(write_03GINFXX(listArtist, 2))) return false;
	if (!(write_03GINFXX(listAlbum, 3))) return false;
	if (!(write_03GINFXX(listGenre, 4))) return false;

	if (!(write_04CNTINF(songsToSend))) return false;

	if (!(write_05CIDLST(songsToSend))) return false;

	//write_TrackNumber(songsToSend);

	//sort & create the playlists
	vector<Song *>     listPlaylist;
	vector<Song *>	   songsInPlaylist;

	int nbSongs;

	//sort the playlist by alphabetical order
	sort(playlist.begin(), playlist.end(), sortPlaylist);

	//scan all playlist
	for (vector<Playlist>::iterator pl = playlist.begin(); pl != playlist.end(); pl++)
	{
		//all songs in playlist
		nbSongs = 0;
		for (vector<Song *>::iterator songInPl = (*pl).songs.begin(); songInPl != (*pl).songs.end(); songInPl++)
		{
			//all songs on device (check if present and get the file number)
			for (vector<Song *>::iterator tmpSong = songsToSend.begin(); tmpSong != songsToSend.end(); tmpSong++)
			{
				if ((*tmpSong)->statusOfSong != EMPTYTRACK)
				{
					if ((STRCMP2_NULLOK((*tmpSong)->album, (*songInPl)->album) == 0) &&
							(STRCMP2_NULLOK((*tmpSong)->artist, (*songInPl)->artist) == 0) &&
							(STRCMP2_NULLOK((*tmpSong)->title, (*songInPl)->title) == 0))
					{
						(*songInPl)->sonyDbOrder = (*tmpSong)->sonyDbOrder;
						(*songInPl)->statusOfSong = (*tmpSong)->statusOfSong;
						songsInPlaylist.push_back((*songInPl));
						nbSongs++;
						break;
					}
				}
			}
		}

		//playlist is not empty
		if (nbSongs > 0)
		{
			//add the playlist
			Song *s = new Song();
			s->album = (*pl).name;
			s->artist = strdup(" ");
			s->genre = strdup(" ");
			s->title = strdup(" ");
			s->filename = strdup(" ");
			s->track_nr = nbSongs; //for writing the file later
			listPlaylist.push_back(s);
		}
		else
		{
			fprintf(fp, "Ignoring Playlist %s because all tracks are missing\n", (*pl).name);
			fflush(fp);
		}
	}

	if (listPlaylist.size() > 0)
	{
		//write the playlists 
		write_01TREEXX(songsInPlaylist , listPlaylist, 22); //write_01TREE22
		write_03GINFXX(listPlaylist ,22); //write_03GINF22
	}
	else
	{
		write_01TREE22();
		write_03GINF22();
	}
	while (listPlaylist.size() > 0)
	{
		Song *s = listPlaylist.back();
		listPlaylist.pop_back();
		free(s->album);
		free(s->artist);
		free(s->genre);
		free(s->title);
		free(s->filename);
		free(s);
	}

	songsInPlaylist.clear();

	listArtist.clear();
	listAlbum.clear();
	listGenre.clear();

	return (true);
}

//read the track number directly from the omg header
int SonyDb::getTrackNumber(char *filename)
{
	FILE *fin = fopen(filename, "rb");
	if (fin == NULL)
	{
		fprintf(fp, "error can't open file %s\n", filename);
		fflush(fp);
		return (-1);
	}
	//fprintf(fp, "1 Opening file %s ok\n", filename);fflush(fp);
	uint8     tmpTag[512];
	uint32    tagLength = 0;

	//read the id3 header
	if (fread(&tmpTag, sizeof(uint8), 10, fin) != 10)
	{
		fprintf(fp, "error can't read file header : %s\n", filename);
		fflush(fp);
		fclose(fin);
		return (-1);
	}
	//fprintf(fp, "2 Reading of the id3 header ok\n");fflush(fp);
	memset(tmpTag, 0, 512);
	for (int i = 0; i < 5; i++)
	{
		//fprintf(fp, "Reading tag %i: ", i);fflush(fp);
		if (fread(&tmpTag, sizeof(uint8), 11, fin) != 11)
		{
			fprintf(fp, "error can't read file tag %i : %s\n", filename, i);
			fflush(fp);
			fclose(fin);
			return (-1);
		}

		if ((tmpTag[4] == 0) && (tmpTag[5] == 0) && (tmpTag[6] == 0) && (tmpTag[7] == 0))
		{
			//tag not found
			fprintf(fp, "Tag not present: %s\n", filename);
			fflush(fp);
			fclose(fin);
			return (-1);
		}
		else
		{
			//not synch safe!?
			//tagLength = ((tmpTag[4] << 21) + (tmpTag[5] << 14) + (tmpTag[6] << 7) + tmpTag[7]) - 1;
			tagLength = ((tmpTag[4] << 24) + (tmpTag[5] << 16) + (tmpTag[6] << 8) + tmpTag[7]) - 1;

			if ((STRNCMP_NULLOK((char*)tmpTag, "TXXX", 4) == 0))
			{
				memset(tmpTag, 0, 512);
				if (fread(&tmpTag, sizeof(uint8), tagLength, fin) != tagLength)
				{
					fprintf(fp, "error can't read file 4 : %s\n", filename);
					fflush(fp);
					fclose(fin);
					return (-1);
				}
				tmpTag[19] = 20;//quick fix : replace '*' by a space in "OMG_TRACK*XXXX"
				char *res1 = utf16_to_ansi((uint16*)tmpTag, tagLength, true);
				//fprintf(fp, "TRACK NUMBER : >%s<\n", res1);fflush(fp);
				char *res2 = res1 + 10; //remove OMG_TRACK 
				int res3 = atoi(res2);
				free(res1);
				fclose(fin);
				return (res3);
			}
			else
			{
				//fprintf(fp, "Skipping Tag \n");fflush(fp);
				if (fseek(fin, tagLength, SEEK_CUR) != 0)
				{
					fprintf(fp, "error can't seek in file : %s\n", filename);
					fflush(fp);
					fclose(fin);
					return (-1);
				}
			}	  
		}
	}
	fclose(fin);
	return (-1);
}

//read all playlist on the device
int  SonyDb::readAllPlaylist()
{
	freeAllPlaylist();

	FILE *f;
	Song *s;

	char fileName[512];    
	//open the file
	sprintf(fileName, "%s/OMGAUDIO/03GINF22.DAT", getDriveLetter());
	f = fopen(fileName, "rb");
	if (f == NULL)
	{
		fprintf(fp, "error can't open file 03GINF22.DAT\n");fflush(fp);
		return 0;
	}

	//read the headers
	sonyFileHeader header;
	sonyObjectPointer Opointer;
	sonyObject obj;

	if (!getHeader(&header, f))
	{
		fclose(f);
		return false;
	}
	if (!getObjectPointer(&Opointer, f))
	{
		fclose(f);
		return false;
	}
	if (!getObject(&obj, f))
	{
		fclose(f);
		return false;
	}

	vector<Song*> listOfPlaylist;

	for (int index = 1; index <= obj.count; index++)
	{
		s = new Song();

		if (getTrack(f, s) )
			listOfPlaylist.push_back(s);
	}
	fclose(f);

	int plIndex = 1;
	for (vector<Song *>::iterator song = listOfPlaylist.begin(); song != listOfPlaylist.end(); song++)
	{
		Playlist *p = new Playlist();
		p->name = strdup((*song)->title);
		p->index = plIndex++;
		addPlaylist(p);
	}

	//sort the playlist by index
	sort(playlist.begin(), playlist.end(), sortByPlaylistIndex);

	while (listOfPlaylist.size() > 0)
	{
		s = listOfPlaylist.back();
		if (s->title) free(s->title);
		free(s);
		listOfPlaylist.pop_back();
	}

	//open the file
	sprintf(fileName, "%s/OMGAUDIO/01TREE22.DAT", getDriveLetter());
	f = fopen(fileName, "rb");
	if (f == NULL)
	{
		fprintf(fp, "error can't open file 01TREE22.DAT\n");fflush(fp);
		return 0;
	}

	//read the headers
	sonyFileHeader header2;
	sonyObjectPointer Opointer2;
	sonyObjectPointer Opointer3;
	sonyObject obj2;
	sonyObject obj3;

	if (!getHeader(&header2, f))
	{
		fclose(f);
		return false;
	}
	if (!getObjectPointer(&Opointer2, f))
	{
		fclose(f);
		return false;
	}
	if (!getObjectPointer(&Opointer3, f))
	{
		fclose(f);
		return false;
	}
	if (!getObject(&obj2, f))
	{
		fclose(f);
		return false;
	}

	uint16 index1;
	uint16 index2;
	uint16 cte1;
	uint16 cte2;

	vector<uint16> p1;
	vector<uint16> p2;

	for (int i = 0; i < obj2.count; i++)
	{
		fread(&index1, sizeof(uint16), 1, f);
		fread(&cte1, sizeof(uint16), 1, f);
		fread(&index2, sizeof(uint16), 1, f);
		fread(&cte2, sizeof(uint16), 1, f);
		index1 = UINT16_SWAP_BE_LE(index1);
		index2 = UINT16_SWAP_BE_LE(index2);
		p1.push_back(index1);
		p2.push_back(index2);
	}

	fseek(f, Opointer3.offset, SEEK_SET);
	if (!getObject(&obj3, f))
	{
		fclose(f);
		return false;
	}

	vector<Playlist>::iterator pla = playlist.begin();

	//for each track
	int nbTrack = 0;
	int catIndex = 0;
	for (int j = 0; j < obj3.count; j++)
	{
		fread(&cte1, sizeof(uint16), 1, f);
		cte1 = UINT16_SWAP_BE_LE(cte1);

		if (nbTrack == 0)
		{
			//find the next playlist
			for (vector<Playlist>::iterator tmp = playlist.begin(); tmp != playlist.end(); tmp++)	
			{
				int index = (int)(p1.front()+catIndex);
				if ((*tmp).index == index)
				{
					pla = tmp; //found the playlist
					if ((p2.begin()+(catIndex + 1) >= p2.end()))
						nbTrack = -1;
					else
						nbTrack = p2.at(catIndex + 1) - p2.at(catIndex); //number of track to read for this playlist
					//if nbTrack is negative it will take all the rest of the tracks for this playlist
					catIndex++;
					break;
				}
			}
		}	
		nbTrack--;
		for (vector<Song>::iterator so = songs.begin(); so != songs.end(); so++)
		{
			if ((*so).sonyDbOrder == cte1)
				(*pla).songs.push_back(&(*so));
		}
	}  
	fclose(f);

	while (playlist_temporary.size() > 0)
	{
		Playlist p = playlist_temporary.back();
		addPlaylist(&p);
		playlist_temporary.pop_back();
	}

	return (playlist.size());
}

//read all tracks from the device to a song vector
int SonyDb::readAllTracks()
{
	updateDiskSpaceInfo();

	if (this->copying)
	{
		return (songs_temporary.size());
	}
	else
	{
		freeAllTracks();
		nbTrackToAdd = 0;
		nbTrackToDel = 0;
		addTrackTotalByte = 0;
		delTrackTotalByte = 0;

		FILE *f, *f2;
		Song *s;
		bool trackNumberAvailable = false;

		char tmp[512];
		//open the file
		if (!getDriveLetter())
		{
			fprintf(fp, "error can't open file 04CNTINF.DAT\n");fflush(fp);
			return 0;
		}
		sprintf(tmp, "%s/OMGAUDIO/04CNTINF.DAT", getDriveLetter());
		f = fopen(tmp, "rb");
		if (f == NULL)
		{
			fprintf(fp, "error can't open file 04CNTINF.DAT\n");fflush(fp);
			return 0;
		}

		//try to open TRACKS.DAT
		sprintf(tmp, "%s/OMGAUDIO/TRACKS.DAT", getDriveLetter());
		f2 = fopen(tmp, "rb");
		if (f2 == NULL)
		{
			trackNumberAvailable = false;
		}
		else
		{
			trackNumberAvailable = true;
		}


		//read the headers
		sonyFileHeader header;
		sonyObjectPointer Opointer;
		sonyObject obj;

		lastTrackIndex = 1;

		if (!getHeader(&header, f))
		{
			this->trackListLoaded = true;
			fclose(f);
			return false;
		}
		if (!getObjectPointer(&Opointer, f))
		{
			this->trackListLoaded = true;
			fclose(f);
			return false;
		}
		if (!getObject(&obj, f))
		{
			this->trackListLoaded = true;
			fclose(f);
			return false;
		}

		char *oldAlbum = NULL;
		int  tracknum = 1;

		for (int index = 1; index <= obj.count; index++)
		{
			s = new Song();

			if (getTrack(f, s))
			{
				//dummy track number in case we can't read it from the tags
				if (STRCMP2_NULLOK(s->album, oldAlbum) == 0)
				{
					tracknum++;
				}
				else
				{
					if (STRCMP2_NULLOK(s->album, "") != 0)
					{
						oldAlbum = s->album;
						tracknum = 1;
					}
				}

				if (!(s->album)) s->album = strdup("");
				if (!(s->artist)) s->album = strdup("");
				if (!(s->genre)) s->album = strdup("");
				if (!(s->title)) s->album = strdup("");

				if ((STRCMP2_NULLOK(s->album, "") == 0) && 
						(STRCMP2_NULLOK(s->artist, "") == 0) &&
						(STRCMP2_NULLOK(s->genre, "") == 0) &&
						(STRCMP2_NULLOK(s->title, "") == 0))
				{
					//Empty slot
					s->filename = strdup("");
					s->statusOfSong = EMPTYTRACK;
				}
				else
				{    
					//adding additionnal info
					s->filename = GetOMAFilename(index);
					//fprintf(fp, "file number %s\n", s->filename);fflush(fp);
					if (trackNumberAvailable)
					{
						fread(&tmp, 1, 9, f2);
						s->track_nr = atoi(tmp);
					}
					else
					{
						int tmp = getTrackNumber(s->filename);
						if (tmp != -1)
							s->track_nr = tmp;
						else
							s->track_nr = tracknum;
					}
					s->year = 0;
					s->statusOfSong = ON_DEVICE;
				}
				s->sonyDbOrder = index;
				lastTrackIndex++;
				songs.push_back(*s);
			}
		}
		fclose(f);
		if (trackNumberAvailable)
			fclose(f2);

		while (songs_temporary.size() > 0)
		{
			Song s2 = songs_temporary.back();
			addSong(&s2);
			songs_temporary.pop_back();
		}

		this->trackListLoaded = true;
		//fprintf(fp, "All Done\n");fflush(fp);
		return (songs.size());
	}
}


bool SonyDb::getTrack(FILE *f, Song *output)
{
	sonyTrack t;

	//get the track info
	if (fread(&t, sizeof(sonyTrack), 1, f) != 1)
	{
		fprintf(fp, "error could not read file header\n");fflush(fp);
		return false;
	}
	else
	{
		if (t.nbTagRecords == 0) //last track reached
			return false;

		//get values from big endian
		t.trackEncoding = UINT32_SWAP_BE_LE(t.trackEncoding);
		t.trackLength = UINT32_SWAP_BE_LE(t.trackLength);
		t.nbTagRecords = UINT16_SWAP_BE_LE(t.nbTagRecords);
		t.sizeTagRecords = UINT16_SWAP_BE_LE(t.sizeTagRecords);
		//fprintf(fp, "\nTrack :\n, length : %i, nbFrames : %i , nbTags : %i, tagSize : %i\n", t.trackLength, t.trackEncoding, t.nbTagRecords, t.sizeTagRecords);
		//fflush(fp);
	}   

	output->encoding = t.trackEncoding;


	//length is in ms
	output->songlen = t.trackLength / 1000;

	//read the tags from the 04CNTINF.DAT file
	char tagType[4];
	char encoding[2];
	utf16char *tagRecord = (utf16char*) malloc(sizeof(utf16char) * t.sizeTagRecords);

	for (int i = 1; i <= t.nbTagRecords; i++)
	{  
		//read type
		if (fread(tagType, 4, 1, f) != 1)
			return false;

		//read encoding
		if (fread(encoding, 2, 1, f)!= 1)
			return false;

		//read the tag
		if (fread(tagRecord, t.sizeTagRecords - 6, 1, f) != 1)
			return (false);

		if (STRNCMP_NULLOK(tagType, "TIT2", 4) == 0)
		{
			//utf16 version
			output->wTitle = tagRecord;

			//ansi version
			output->title = utf16_to_ansi((utf16char*)tagRecord, t.sizeTagRecords, true);
			//fprintf(fp, "Read title : %s\n", output->title);fflush(fp);
			continue;
		}

		if (STRNCMP_NULLOK(tagType, "TPE1", 4) == 0)
		{
			//utf16 version
			output->wArtist = tagRecord;

			//ansi version
			output->artist = utf16_to_ansi((utf16char*)tagRecord, t.sizeTagRecords, true);
			continue;
		}

		if (STRNCMP_NULLOK(tagType, "TALB", 4) == 0)
		{
			//utf16 version
			output->wAlbum = tagRecord;

			//ansi version
			output->album = utf16_to_ansi((utf16char*)tagRecord, t.sizeTagRecords, true);
			continue;
		}

		if (STRNCMP_NULLOK(tagType, "TCON", 4) == 0)
		{
			//utf16 version
			output->wGenre = tagRecord;

			//ansi version
			output->genre = utf16_to_ansi((utf16char*)tagRecord, t.sizeTagRecords, true);
			continue;
		}
	}
	return (true);
}


/*****************************************************************************************/
/** misc functions : detect devices etc...*/

//free disk space info
static void commaValue(__int64 val0, char *dest)
{
	int val = (int)val0;
	if ((val>=1024) || (val0 <= -1024))
	{
		sprintf(dest,"%d.%02d GB",val/1024,(val%1024)/10);
	} else
		sprintf(dest,"%d MB",val);
}

//free disk space info
void SonyDb::updateDiskSpaceInfo()
{
#ifdef _WIN32
	char drive[2];

	strcpy(drive, this->getDriveLetter());

	ULARGE_INTEGER free={0,};
	ULARGE_INTEGER total={0,};
	ULARGE_INTEGER freeb={0,};
	GetDiskFreeSpaceEx(drive, &free, &total, &freeb);
	usedSpaceDisk = total.QuadPart - freeb.QuadPart;
	freeSpaceDisk = freeb.QuadPart;
	totalDiskSpaceValue = total.QuadPart;

	unsigned int totalmb = (unsigned int)((total.QuadPart)/(1024*1024));
	commaValue(totalmb, totalDiskSpace);
#else
	strcpy(totalDiskSpace, "100 MB");
#endif
}


char *SonyDb::getFreeDiskSpaceAfterApply()
{
	__int64 tmp = freeSpaceDisk - addTrackTotalByte + delTrackTotalByte;
	commaValue((int)(tmp/(1024*1024)), freeDiskSpaceAfterApply);
	return (this->freeDiskSpaceAfterApply);
}


char *SonyDb::getTotalDiskSpace()
{
	return (this->totalDiskSpace);
}


char *SonyDb::getTotalUsedSpaceAfterApply()
{
	__int64 tmp = usedSpaceDisk + addTrackTotalByte - delTrackTotalByte;
	commaValue((int)(tmp/(1024*1024)), totalUsedSpaceAfterApply);
	return (this->totalUsedSpaceAfterApply);
}

char *SonyDb::getSizeTrackToAdd()
{
	commaValue((int)(addTrackTotalByte/(1024*1024)), addTrackTotalByteString);
	return (addTrackTotalByteString);
}

char *SonyDb::getSizeTrackToDel()
{
	commaValue((int)(delTrackTotalByte/(1024*1024)), delTrackTotalByteString);
	return (delTrackTotalByteString);
}

//return the disk space missing to apply the modifications
char *SonyDb::getNeededSpace()
{
	updateDiskSpaceInfo();
	int res;
	__int64 totalSpaceUsed;

	totalSpaceUsed = usedSpaceDisk + addTrackTotalByte + DATABASE_HEADER_SIZE - delTrackTotalByte;
	res = (int)((totalSpaceUsed - totalDiskSpaceValue) / (1024 * 1024));
	commaValue(res, neededSpace);
	return (neededSpace);
}

//return a positive value if there is not enough space
int  SonyDb::getNeededSpaceValue()
{
	updateDiskSpaceInfo();
	int res;
	__int64 totalSpaceUsed;

	totalSpaceUsed = usedSpaceDisk + addTrackTotalByte + DATABASE_HEADER_SIZE - delTrackTotalByte;
	res = (int)((totalSpaceUsed - totalDiskSpaceValue) / (1024 * 1024));

	return (res);
}


//return the devices letter
char* SonyDb::getDriveLetter()
{
	return (this->driveLetter);
}

//return the device name
char *SonyDb::getDeviceName()
{
	return (this->deviceName);
}

//is the device currently copying or downloading a file?
bool SonyDb::isCopying()
{
	return (this->copying);
}

//check if the device is still present
bool SonyDb::isPresent()
{
	if (this->driveLetter != 0)
		return (detectPlayer(this->driveLetter));
	else
		return false;
}

//find the first device available from A to Z
bool SonyDb::detectPlayer()
{    
#ifdef _WIN32
	ULONG uDriveMask = _getdrives(); //get all valid drives
	char letter[3];
	string path = "/OMGAUDIO/04CNTINF.DAT";
	FILE *stream;
	strcpy(letter, "A:");

	if (uDriveMask == 0)
	{
		fprintf(fp, "_getdrives() failed with failure code: %d\n", GetLastError());
		fflush(fp);
	}
	else
	{ 
		//no popup for insert cd in cd drives
		SetErrorMode(SEM_FAILCRITICALERRORS);
		while (uDriveMask)
		{
			//assume A & B are floppy drive (avoid annoying sounds :-) )
			if ((letter[0] != 'A') && (letter[0] != 'B') && (uDriveMask & 1))
			{
				if( (stream  = fopen( (letter + path).c_str(), "r" )) != NULL )
				{
					fclose(stream);
					this->driveLetter = strdup(letter);
					sprintf(deviceName, "%sSony Walkman", getDriveLetter());
					SetErrorMode(0); //restore error mode to normal
					return (true);
				}
			}
			letter[0]++;
			uDriveMask >>= 1;
		}
	}

	SetErrorMode(0); //restore error mode to normal
#else
	char* detect_letter[] = {
		"/media/usbdisk",
		"/media/usbdisk1",
		"/media/WALKMAN",
		NULL
	};
	char** ptr = detect_letter;
	while(*ptr)
		if (detectPlayer(*ptr++))
			return (true);
#endif
	return (false);
}

//search for a device starting at the letter specified order is A to Z
bool SonyDb::detectPlayer(char* letter)
{
	FILE *stream;
#ifdef _WIN32
	ULONG uDriveMask = _getdrives(); //get all valid drives
	string path = "/OMGAUDIO/04CNTINF.DAT";
	char drive[3] = {0};
	strncpy(drive, letter, 2);

	if (uDriveMask == 0)
	{
		fprintf(fp, "_getdrives() failed with failure code: %d\n", GetLastError());
		fflush(fp);
	}
	else
	{ 
		//no popup for insert cd in cd drives
		SetErrorMode(SEM_FAILCRITICALERRORS);
		for (int i = 0; i < (drive[0] - 'A'); i++)
		{
			uDriveMask >>= 1;
		}

		while (uDriveMask)
		{
			//assume A & B are floppy drive (avoid annoying sounds :-) )
			if ((drive[0] != 'A') && (drive[0] != 'B') && (uDriveMask & 1))
			{
				if( (stream  = fopen( (drive + path).c_str(), "r" )) != NULL )
				{
					fclose(stream);
					this->driveLetter = strdup(drive);
					sprintf(deviceName, "%s Sony Walkman", getDriveLetter());
					SetErrorMode(0); //restore error mode to normal
					return (true);
				}
			}
			drive[0]++;
			uDriveMask >>= 1;
		}
	}

	SetErrorMode(0); //restore error mode to normal
#else
	string path = letter;
	path += "/OMGAUDIO/04CNTINF.DAT";
	if((stream = fopen(path.c_str(), "r")) != NULL)
	{
		fclose(stream);
		this->driveLetter = strdup(letter);
		sprintf(deviceName, "%s Sony Walkman", getDriveLetter());
		return (true);
	}
#endif
	return (false);
}

//get real file name from id number
char *SonyDb::GetOMAFilename(int id)
{
	char *buffer = (char*)calloc(sizeof(char), 256);
	sprintf(buffer, "%s/OMGAUDIO/10F%02x/1%07x.OMA", getDriveLetter(), id >> 8, id);
	return (buffer);
}




/*  file writers  */
//this one is special...
bool SonyDb::write_00GTRLST()
{
	//write the 00GTRLST.DAT file
	FILE *f;
	char filename[512];

	//open the file
	if (!getDriveLetter())
	{
		fprintf(fp, "error can't open file 04CNTINF.DAT\n");
		fflush(fp);
		return 0;
	}
	sprintf(filename, "%s/OMGAUDIO/00GTRLST.DAT", getDriveLetter());
	f = fopen(filename, "wb");
	if (f == NULL)
	{
		fprintf(fp, "error can't open file %s\n", filename);
		fflush(fp);
		return false;
	}

	//write the headers
	sonyFileHeader header;
	sonyObjectPointer Opointer;
	sonyObject obj;

	//file header
	header.magic[0] = 'G';
	header.magic[1] = 'T';
	header.magic[2] = 'L';
	header.magic[3] = 'T';
	writeHeader(&header, f, 2);

	//object pointer 1
	Opointer.magic[0] = 'S';
	Opointer.magic[1] = 'Y';
	Opointer.magic[2] = 'S';
	Opointer.magic[3] = 'B';
	Opointer.offset = UINT32_SWAP_BE_LE(0X0030);
	Opointer.length = UINT32_SWAP_BE_LE(0x0070);
	writeObjectPointer(&Opointer, f);

	//object pointer 2
	Opointer.magic[0] = 'G';
	Opointer.magic[1] = 'T';
	Opointer.magic[2] = 'L';
	Opointer.magic[3] = 'B';
	Opointer.offset = UINT32_SWAP_BE_LE(0X00A0);
	Opointer.length = UINT32_SWAP_BE_LE(0x0AB0);
	writeObjectPointer(&Opointer, f);

	uint8 t[16];
	for (int i = 0; i < 16; i++)
		t[i] = 0;

	//write the SYSB object header
	obj.magic[0] = 'S';
	obj.magic[1] = 'Y';
	obj.magic[2] = 'S';
	obj.magic[3] = 'B';
	obj.count = UINT16_SWAP_BE_LE(1);
	obj.size = UINT16_SWAP_BE_LE(80);
	obj.padding[0] = 0x00D0;
	obj.padding[1] = 0;
	writeObject(&obj, f);

	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);

	//write the GTLB object header
	obj.magic[0] = 'G';
	obj.magic[1] = 'T';
	obj.magic[2] = 'L';
	obj.magic[3] = 'B';
	obj.count = UINT16_SWAP_BE_LE(34);
	obj.size = UINT16_SWAP_BE_LE(80);
	obj.padding[0] = UINT32_SWAP_BE_LE(0x0005);
	obj.padding[1] = 0x0003;
	writeObject(&obj, f);



	//first block
	t[1] = 0x01;
	t[3] = 0x01;
	fwrite(&t, sizeof(uint8), 16, f);
	t[3] = 0;
	fwrite(&t, sizeof(uint8), 16, f);
	t[1] = 0;
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);

	//2nd
	t[1] = 0x02;
	t[3] = 0x03;
	fwrite(&t, sizeof(uint8), 16, f);
	t[1] = 0x01;
	t[3] = 0;
	t[4] = 'T';
	t[5] = 'P';
	t[6] = 'E';
	t[7] = '1';
	fwrite(&t, sizeof(uint8), 16, f);
	t[1] = 0;
	t[4] = 0;
	t[5] = 0;
	t[6] = 0;
	t[7] = 0;
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);

	//3rd
	t[1] = 0x03;
	t[3] = 0x03;
	fwrite(&t, sizeof(uint8), 16, f);
	t[1] = 0x01;
	t[3] = 0;
	t[4] = 'T';
	t[5] = 'A';
	t[6] = 'L';
	t[7] = 'B';
	fwrite(&t, sizeof(uint8), 16, f);
	t[1] = 0;
	t[4] = 0;
	t[5] = 0;
	t[6] = 0;
	t[7] = 0;
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);


	//4th
	t[1] = 0x04;
	t[3] = 0x03;
	fwrite(&t, sizeof(uint8), 16, f);
	t[1] = 0x01;
	t[3] = 0;
	t[4] = 'T';
	t[5] = 'C';
	t[6] = 'O';
	t[7] = 'N';
	fwrite(&t, sizeof(uint8), 16, f);
	t[1] = 0;
	t[4] = 0;
	t[5] = 0;
	t[6] = 0;
	t[7] = 0;
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);

	//5th
	t[1] = 0x22;
	t[3] = 0x02;
	fwrite(&t, sizeof(uint8), 16, f);
	t[1] = 0;
	t[3] = 0;
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);
	fwrite(&t, sizeof(uint8), 16, f);

	for (int j = 5; j < 34 ; j++)
	{
		t[1] = (uint8) j;
		fwrite(&t, sizeof(uint8), 16, f);
		t[1] = 0;
		fwrite(&t, sizeof(uint8), 16, f);
		fwrite(&t, sizeof(uint8), 16, f);
		fwrite(&t, sizeof(uint8), 16, f);
		fwrite(&t, sizeof(uint8), 16, f);
	}

	fclose(f);
	return (true);
}


bool SonyDb::write_01TREEXX(vector<Song *> songs, vector<Song *> list, int type)
{
	//write the 01TREEXX.DAT file
	FILE *f;
	char filename[512];
	int nbSongs = 0;
	int nbCat = 0;

	//open the file
	sprintf(filename, "%s/OMGAUDIO/01TREE%02i.DAT", getDriveLetter(), type);
	f = fopen(filename, "wb");
	if (f == NULL)
	{
		fprintf(fp, "error can't open file %s\n", filename);
		fflush(fp);
		return false;
	}


	//number of non empty tracks
	for (vector<Song *>::iterator count = songs.begin(); count != songs.end(); count++)
	{
		if ((*count)->statusOfSong != EMPTYTRACK)
			nbSongs++;
	}

	////nbSongs = nbsongs * 2 (because of all songs)
	if (useAllTags)
		nbSongs = nbSongs * 2; //"all x"

	//number of non empty categories
	for (vector<Song *>::iterator count2 = list.begin(); count2 != list.end(); count2++)
	{
		if ((*count2)->statusOfSong != EMPTYTRACK)
			nbCat++;
	}

	//nbcat = nbcat + 1 (for the "all x" categorie)
	if (useAllTags)
		nbCat++; //"all x" categorie

	//write the headers
	sonyFileHeader header;
	sonyObjectPointer Opointer;
	sonyObject obj;

	memcpy( header.magic, "TREE", 4 );
	writeHeader(&header, f, 2);

	//GPLB object pointer
	memcpy( Opointer.magic, "GPLB", 4 );
	Opointer.offset = UINT32_SWAP_BE_LE(0X0030);
	Opointer.length = UINT32_SWAP_BE_LE(16400);// 8 * 2048 + 16 of header
	writeObjectPointer(&Opointer, f);

	//TPLB object pointer
	memcpy( Opointer.magic, "TPLB", 4 );
	Opointer.offset = UINT32_SWAP_BE_LE(0X4040);
	Opointer.length = UINT32_SWAP_BE_LE(16 + (nbSongs * 2)  + (16 - (nbSongs * 2 ) % 16)); //TPLB are 2 byte long + 16 of TPLB header
	writeObjectPointer(&Opointer, f);

	//write the GPLB object header
	memcpy( obj.magic, "GPLB", 4 );
	obj.count = UINT16_SWAP_BE_LE(nbCat);
	obj.size = UINT16_SWAP_BE_LE(8);
	obj.padding[0] = UINT32_SWAP_BE_LE(UINT16_SWAP_BE_LE(obj.count));
	obj.padding[1] = 0;	
	writeObject(&obj, f);

	//write the GPLB object
	uint16 index1 = 0;
	uint16 cte1 = UINT16_SWAP_BE_LE(0x0100);
	uint16 index2;
	uint16 cte2 = 0x0000;

	int index = 1;
	int indexTPLB = 1;
	vector<Song *>::iterator pointerTPLB = songs.begin();

	//*debug
	//for (vector<Song *>::iterator sort2 = songs.begin(); sort2 != songs.end(); sort2++)
	//{
		//fprintf(fp, "%i artist:%s, album:%s, title:%s Track:%i\n", (*sort2)->sonyDbOrder, (*sort2)->artist, (*sort2)->album ,(*sort2)->title, (*sort2)->track_nr);
		//fflush(fp);
	//}
	//fprintf(fp, "\n");
	int oldTPLB = 0;
	//debug */

	//add "all tracks" indexes first
	if (useAllTags)
	{
		index1 = UINT16_SWAP_BE_LE(index); index++; //fucking side effect :-P
		index2 = UINT16_SWAP_BE_LE(indexTPLB);
		fwrite(&index1, sizeof(uint16), 1, f);
		fwrite(&cte1, sizeof(uint16), 1, f);
		fwrite(&index2, sizeof(uint16), 1, f);
		fwrite(&cte2, sizeof(uint16), 1, f);
		indexTPLB += (nbSongs / 2);
	}

	//scan the list (by album, by artist, by genre)
	for (vector<Song *>::iterator sort = list.begin(); sort != list.end(); sort++)
	{
		//skip empty tracks
		if ((*sort)->statusOfSong == EMPTYTRACK)
		{
			while ((pointerTPLB != songs.end()) && ((*pointerTPLB)->statusOfSong == EMPTYTRACK))
			{
				pointerTPLB++;
			}
			continue;
		}

		index1 = UINT16_SWAP_BE_LE(index); index++; //fucking side effect :-P
		index2 = UINT16_SWAP_BE_LE(indexTPLB);
		fwrite(&index1, sizeof(uint16), 1, f);
		fwrite(&cte1, sizeof(uint16), 1, f);
		fwrite(&index2, sizeof(uint16), 1, f);
		fwrite(&cte2, sizeof(uint16), 1, f);

		//find the next TPLB index

		if (type == 22)
		{
			//*debug
			//fprintf(fp, " (%i tracks)\nPlaylist : %s \t, from file : %02i",indexTPLB - oldTPLB, (*sort)->album, indexTPLB);
			oldTPLB = indexTPLB;
			//debug */
			while ((pointerTPLB != songs.end()) && ( (*sort)->track_nr-- > 0))
			{
				pointerTPLB++;
				indexTPLB++;
			}
		}

		if ((type == 1) || (type == 3))
		{
			//*debug
			//fprintf(fp, " (%i tracks)\nalbum : %s \t, from file : %02i",indexTPLB - oldTPLB, (*sort)->album, indexTPLB);
			oldTPLB = indexTPLB;
			//debug */
			while ((pointerTPLB != songs.end()) && (STRCMP2_NULLOK((*pointerTPLB)->album, (*sort)->album) == 0))
			{
				pointerTPLB++;
				indexTPLB++;
			}
		}

		if (type == 2)
		{
			//*debug
			//fprintf(fp, " (%i tracks)\nartist : %s \t, from file : %02i",indexTPLB - oldTPLB, (*sort)->artist, indexTPLB);
			oldTPLB = indexTPLB;
			//debug */
			while ((pointerTPLB != songs.end()) && (STRCMP2_NULLOK((*pointerTPLB)->artist, (*sort)->artist) == 0))
			{
				pointerTPLB++;
				indexTPLB++;
			}
		}

		if (type == 4)
		{
			//*debug
			//fprintf(fp, " (%i tracks)\ngenre : %s \t, from file : %02i",indexTPLB - oldTPLB, (*sort)->genre, indexTPLB);
			oldTPLB = indexTPLB;
			//debug */
			while ((pointerTPLB != songs.end()) && (STRCMP2_NULLOK((*pointerTPLB)->genre, (*sort)->genre) == 0))
			{
				pointerTPLB++;
				indexTPLB++;
			}
		}
	}
	//fprintf(fp, "\n\n");//debug

	//fill the rest with zeros 
	index--;
	int last = (16384 - (index * 8));
	uint8 cte3 = 0;
	for (int i = 0; i < last; i++)
		fwrite(&cte3, sizeof(uint8), 1, f);

	//write the TPLB object header
	memcpy( obj.magic, "TPLB", 4 );
	obj.count = UINT16_SWAP_BE_LE(nbSongs);
	obj.size = UINT16_SWAP_BE_LE(2);
	obj.padding[0] = UINT32_SWAP_BE_LE(UINT16_SWAP_BE_LE(obj.count));
	obj.padding[1] = 0;
	writeObject(&obj, f);

	index = 0;
	index1 = 0;

	//do this twice for "all x" indexes
	if (useAllTags)
	{
		for (vector<Song *>::iterator song = songs.begin(); song != songs.end(); song++)
		{
			if ((*song)->statusOfSong == EMPTYTRACK)
				continue;

			index++;
			index1 = UINT16_SWAP_BE_LE((*song)->sonyDbOrder);
			//fprintf(fp, "%i ", (*song)->sonyDbOrder);//debug
			fwrite(&index1, sizeof(uint16), 1, f);
		}
	}

	//rest of indexes
	for (vector<Song *>::iterator song = songs.begin(); song != songs.end(); song++)
	{
		if ((*song)->statusOfSong == EMPTYTRACK)
			continue;

		index++;
		index1 = UINT16_SWAP_BE_LE((*song)->sonyDbOrder);
		//fprintf(fp, "%i ", (*song)->sonyDbOrder);//debug
		fwrite(&index1, sizeof(uint16), 1, f);
	}
	//fprintf(fp, "\n\n");//debug

	//so that we have 8 byte round file
	index1 = 0;
	while (index % 8 != 0)
	{
		fwrite(&index1, sizeof(uint16), 1, f);
		index++;
	}

	fclose(f);
	return (true);
}

//empty playlist
bool SonyDb::write_01TREE22()
{
	//write the 01TREE22.DAT file
	FILE *f;
	char filename[512];

	//open the file
	sprintf(filename, "%s/OMGAUDIO/01TREE22.DAT", getDriveLetter());
	f = fopen(filename, "wb");
	if (f == NULL)
	{
		fprintf(fp, "error can't open file %s\n", filename);
		fflush(fp);
		return false;
	}

	//write the headers
	sonyFileHeader header;
	sonyObjectPointer Opointer;
	sonyObject obj;

	memcpy( header.magic, "TREE", 4 );
	writeHeader(&header, f, 2);

	//GPLB object pointer
	memcpy( Opointer.magic, "GPLB", 4 );
	Opointer.offset = UINT32_SWAP_BE_LE(0X0030);
	Opointer.length = UINT32_SWAP_BE_LE(16);
	writeObjectPointer(&Opointer, f);

	//TPLB object pointer
	memcpy( Opointer.magic, "TPLB", 4 );
	Opointer.offset = UINT32_SWAP_BE_LE(0X0040);
	Opointer.length = UINT32_SWAP_BE_LE(16);
	writeObjectPointer(&Opointer, f);

	//write the GPLB object header
	memcpy( obj.magic, "GPLB", 4 );
	obj.count = 0;
	obj.size = UINT16_SWAP_BE_LE(8);
	obj.padding[0] = UINT32_SWAP_BE_LE(UINT16_SWAP_BE_LE(obj.count));
	obj.padding[1] = 0;
	writeObject(&obj, f);

	//write the TPLB object header
	memcpy( obj.magic, "TPLB", 4 );
	obj.count = UINT16_SWAP_BE_LE(0);
	obj.size = UINT16_SWAP_BE_LE(2);
	obj.padding[0] = UINT32_SWAP_BE_LE(UINT16_SWAP_BE_LE(obj.count));
	obj.padding[1] = 0;
	writeObject(&obj, f);

	fclose(f);
	return (true);
}

bool SonyDb::write_02TREINF(vector<Song *> songs)
{
	//write the write_02TREINF.DAT file
	FILE *f;
	char filename[512];
	int nbTags = 0;

	//open the file
	sprintf(filename, "%s/OMGAUDIO/02TREINF.DAT", getDriveLetter());
	f = fopen(filename, "wb");
	if (f == NULL)
	{
		fprintf(fp, "error can't open file %s\n", filename);
		fflush(fp);
		return false;
	}


	//write the headers
	sonyFileHeader header;
	sonyObjectPointer Opointer;
	sonyObject obj;

	memcpy( header.magic, "GTIF", 4 );
	writeHeader(&header, f);

	memcpy( Opointer.magic, "GTFB", 4 );
	Opointer.offset = UINT32_SWAP_BE_LE(0x0020);
	Opointer.length = UINT32_SWAP_BE_LE(0x2410);
	writeObjectPointer(&Opointer, f);

	memcpy( obj.magic, "GTFB", 4 );
	obj.count = UINT16_SWAP_BE_LE(34);
	obj.size = UINT16_SWAP_BE_LE(0x0090);;
	obj.padding[0] = 0;
	obj.padding[1] = 0;
	writeObject(&obj, f);

	//write the tags
	int nbWriten = 0; 
	sonyTrack t;
	sonyTrackTag tt;

	//not really a sonytrack, just using the same struct
	t.fileType[0] = 0; 
	t.fileType[1] = 0;
	t.fileType[2] = 0;
	t.fileType[3] = 0;
	t.trackLength = 0;
	t.trackEncoding = 0; //?? unknown value related to the album (this is not really trackEncoding)
	memcpy( tt.tagType, "TIT2", 4 );
	t.nbTagRecords = UINT16_SWAP_BE_LE(1);
	t.sizeTagRecords = UINT16_SWAP_BE_LE(TAGSIZE);

	//encoding
	tt.tagEncoding[0] = 0x00;
	tt.tagEncoding[1] = 0x02;

	for (int i = 0; i < 64; i++)
	//for (int i = 0; i < songs.size()-1; i++)
	{
		if (i >= 4)
		{
			tt.tagType[0] = 0;
			tt.tagType[1] = 0;
			tt.tagType[2] = 0;
			tt.tagType[3] = 0;
			tt.tagEncoding[0] = 0;
			tt.tagEncoding[1] = 0;
			t.nbTagRecords = 0;
			t.sizeTagRecords = 0;
			t.trackLength = 0;
		}
		//else
			//t.trackLength = UINT32_SWAP_BE_LE(songs[i]->songlen * 1000);

		if (!(writeTrackHeader(&t, f)))
			return (false);

		if (!(writeTrackTag(&tt, "", f)))
			return (false);
	}
	fclose(f);
	return (true);
}

bool SonyDb::write_03GINFXX(vector<Song *> list, int type)
{
	//write the 03GINFXX.DAT file
	FILE *f;
	char filename[512];
	int nbTags = 0;
	int nbCat = 0;

	//open the file
	sprintf(filename, "%s/OMGAUDIO/03GINF%02i.DAT", getDriveLetter(), type);
	f = fopen(filename, "wb");
	if (f == NULL)
	{
		fprintf(fp, "error can't open file %s\n", filename);
		fflush(fp);
		return false;
	}

	if (type == 22)
		type = 1;

	//number of non empty categories
	for (vector<Song *>::iterator count2 = list.begin(); count2 != list.end(); count2++)
	{
		if ((*count2)->statusOfSong != EMPTYTRACK)
			nbCat++;
	}

	//for "all x" categorie
	if (useAllTags)
		nbCat++;

	if (type == 1)
		nbTags = 6;
	else
		nbTags = 1;

	//write the headers
	sonyFileHeader header;
	sonyObjectPointer Opointer;
	sonyObject obj;

	memcpy( header.magic, "GPIF", 4 );
	writeHeader(&header, f);

	memcpy( Opointer.magic, "GPFB", 4 );
	Opointer.length = UINT32_SWAP_BE_LE((((TAGSIZE * nbTags) + 16) * nbCat)+16);//basically the size of the next block
	obj.size = UINT16_SWAP_BE_LE((TAGSIZE * nbTags) + 16); //(128 * nbtag) + tag header (same thing here)
	Opointer.offset = 0x20000000;
	writeObjectPointer(&Opointer, f);

	memcpy( obj.magic, "GPFB", 4 );
	obj.count = UINT16_SWAP_BE_LE(nbCat);
	obj.padding[0] = 0;
	obj.padding[1] = 0;
	writeObject(&obj, f);

	//write the tags
	int nbWriten = 0; 
	sonyTrack t;
	sonyTrackTag tt;

	t.fileType[0] = 0;
	t.fileType[1] = 0;
	t.fileType[2] = 0;
	t.fileType[3] = 0;
	t.trackLength = 0;
	t.trackEncoding = 0;//UINT32_SWAP_BE_LE(); //fixme with the correct number of frames
	t.nbTagRecords = UINT16_SWAP_BE_LE(nbTags);
	t.sizeTagRecords = UINT16_SWAP_BE_LE(TAGSIZE);

	//encoding
	tt.tagEncoding[0] = 0x00;
	tt.tagEncoding[1] = 0x02;

	//push front of the list the "all x" categorie
	if (useAllTags)
	{
		Song *s = new Song();
		if (type == 1)
		{
			s->album = "All Albums";
			s->artist = "MLSONYALL";
			s->genre = "MLSONYALL";
		}
		else
		{
			s->album = "All Albums";
			s->artist = "All Artists";
			s->genre = "All Genre";	       
		}
		list.insert(list.begin(), s);
		s->statusOfSong = ADD_TO_DEVICE;
	}

	for (vector<Song *>::iterator i = list.begin(); i != list.end(); i++)
	{
		if ((*i)->statusOfSong == EMPTYTRACK)
			continue;

		if (!(writeTrackHeader(&t, f)))
			return (false);

		//list of : genre
		if (type == 4)
		{
			//artist tag
			memcpy( tt.tagType, "TIT2", 4 );
			if (!(writeTrackTag(&tt, (*i)->genre, f)))
				return (false);
		}

		//list of : albums
		if (type == 3)
		{
			//artist tag
			memcpy( tt.tagType, "TIT2", 4 );
			if (!(writeTrackTag(&tt, (*i)->album, f)))
				return (false);
		}

		//list of : artist
		if (type == 2)
		{
			//artist tag
			memcpy( tt.tagType, "TIT2", 4 );
			if (!(writeTrackTag(&tt, (*i)->artist, f)))
				return (false);
		}

		//list of: album artist genre
		if (type == 1)
		{
			//album tag
			memcpy( tt.tagType, "TIT2", 4 );
			if (!(writeTrackTag(&tt, (*i)->album, f)))
				return (false);

			//artist tag
			memcpy( tt.tagType, "TPE1", 4 );
			if (!(writeTrackTag(&tt, (*i)->artist, f)))
				return (false);

			//genre tag
			memcpy( tt.tagType, "TCON", 4 );
			if (!(writeTrackTag(&tt, (*i)->genre, f)))
				return (false);

			//tsop tag
			memcpy( tt.tagType, "TSOP", 4 );
			if (!(writeTrackTag(&tt, "", f)))
				return (false);

			//picp tag
			memcpy( tt.tagType, "", 4 );
			if (!(writeTrackTag(&tt, "", f)))
				return (false);

			//pic0 tag
			memcpy( tt.tagType, "PIC0", 4 );
			if (!(writeTrackTag(&tt, "", f)))
				return (false);
		}
	}
	//Remove "all x" categorie
	if (useAllTags)
	{
		Song *s = *(list.begin());
		free(s);
		list.erase(list.begin());
	}

	fclose(f);
	return (true);
}


bool SonyDb::write_04CNTINF(vector<Song *> songsToSend)
{
	//write the 04CNTINF.DAT file
	FILE *f;
	char filename[512];
	//open the file
	sprintf(filename, "%s/OMGAUDIO/04CNTINF.DAT", getDriveLetter());
	f = fopen(filename, "wb");
	if (f == NULL)
	{
		fprintf(fp, "error can't open file 04CNTINF.DAT\n");
		fflush(fp);
		return false;
	}

	//write the headers
	sonyFileHeader header;
	sonyObjectPointer Opointer;
	sonyObject obj;

	memcpy( header.magic, "CNIF", 4 );
	writeHeader(&header, f);

	memcpy( Opointer.magic, "CNFB", 4 );
	Opointer.length = UINT32_SWAP_BE_LE((((TAGSIZE * 5) + 16) * songsToSend.size())+16);
	Opointer.offset = 0x20000000;
	writeObjectPointer(&Opointer, f);

	memcpy( obj.magic, "CNFB", 4 );
	obj.count = UINT16_SWAP_BE_LE(songsToSend.size());
	obj.size = UINT16_SWAP_BE_LE((TAGSIZE * 5) + 16); //(128 * nbtag) + tag header
	obj.padding[0] = 0;
	obj.padding[1] = 0;
	writeObject(&obj, f);

	//write the tags
	int nbWriten = 0;
	sonyTrack t;
	sonyTrackTag tt;

	t.fileType[0] = 0x00;
	t.fileType[1] = 0x00;
	t.fileType[2] = 0xff;
	if (this->codeType == ENCODING_USE_NONE)
	{
		t.fileType[3] = 0xff; //encrypted or not
	}
	else
	{
		t.fileType[3] = 0xfe; //encrypted or not
	}


	t.nbTagRecords = UINT16_SWAP_BE_LE(5);
	t.sizeTagRecords = UINT16_SWAP_BE_LE(TAGSIZE);

	//encoding
	tt.tagEncoding[0] = 0x00;
	tt.tagEncoding[1] = 0x02;

	for (vector<Song *>::iterator i = songsToSend.begin(); i != songsToSend.end(); i++)
	{
		t.trackEncoding = (*i)->encoding;
		t.trackLength = UINT32_SWAP_BE_LE((*i)->songlen * 1000);


		if (!(writeTrackHeader(&t, f)))
			return (false);

		//title tag
		memcpy( tt.tagType, "TIT2", 4 );
		if (!(writeTrackTag(&tt, (*i)->title, f)))
			return (false);

		//artist tag
		memcpy( tt.tagType, "TPE1", 4 );
		if (!(writeTrackTag(&tt, (*i)->artist, f)))
			return (false);

		//album tag
		memcpy( tt.tagType, "TALB", 4 );
		if (!(writeTrackTag(&tt, (*i)->album, f)))
			return (false);

		//genre tag
		memcpy( tt.tagType, "TCON", 4 );
		if (!(writeTrackTag(&tt, (*i)->genre, f)))
			return (false);

		//tsop tag
		memcpy( tt.tagType, "TSOP", 4 );
		if (!(writeTrackTag(&tt, "", f)))
			return (false);
	}
	fclose(f);
	return (true);
}


bool SonyDb::write_05CIDLST(vector<Song *> songsToSend)
{
	FILE *f;
	char filename[512];
	int nbTags = 0;

	//open the file
	sprintf(filename, "%s/OMGAUDIO/05CIDLST.DAT", getDriveLetter());
	f = fopen(filename, "wb");
	if (f == NULL)
	{
		fprintf(fp, "error can't open file %s\n", filename);
		fflush(fp);
		return false;
	}

	//write the headers
	sonyFileHeader header;
	sonyObjectPointer Opointer;
	sonyObject obj;

	memcpy( header.magic, "CIDL", 4 );
	writeHeader(&header, f);

	memcpy( Opointer.magic, "CILB", 4 );
	Opointer.length = UINT32_SWAP_BE_LE(((32 + 16) * songsToSend.size())+16);
	Opointer.offset = UINT32_SWAP_BE_LE(0x0020);
	writeObjectPointer(&Opointer, f);

	memcpy( obj.magic, "CILB", 4 );
	obj.count = UINT16_SWAP_BE_LE(songsToSend.size());
	obj.size = UINT16_SWAP_BE_LE(32 + 16);
	obj.padding[0] = 0;
	obj.padding[1] = 0;
	writeObject(&obj, f);

	uint8 t[16];
	uint8 tt[32];

	t[0] = 0; //WWWWTTTTFFFFFF??????
	t[1] = 0;
	t[2] = 0;
	t[3] = 0;
	t[4] = 0x01;
	t[5] = 0x0F;
	t[6] = 0x50;
	t[7] = 0x00;
	t[8] = 0x00;
	t[9] = 0x04;
	t[10] = 0;
	t[11] = 0;
	t[12] = 0;
	t[13] = 0x01; //fixme 01
	t[14] = 0x02; //fixme 02
	t[15] = 0x03; //fixme 03

	tt[0] = 0xc8; //value is different in NAW3000?!
	tt[1] = 0xd8;
	tt[2] = 0x36;
	tt[3] = 0xd8;

	for (int i = 4; i < 32; i++)
		tt[i] = 0;

	for (vector<Song *>::iterator song = songsToSend.begin(); song != songsToSend.end(); song++)
	{
		if (fwrite(&t, 16, 1, f) != 1)
			return (false);
		tt[4] = 0x11; //fixme 11
		tt[5] = 0x22; //fixme 22
		tt[6] = 0x33; //fixme 33 
		tt[7] = 0x44; //fixme 44
		if (fwrite(&tt, 32, 1, f) != 1)
			return (false);
	}

	fclose(f);
	return (true);
}

//write empty playlist file
bool SonyDb::write_03GINF22()
{
	//write the 03GINF22.DAT file
	FILE *f;
	char filename[512];
	int nbTags = 0;

	//open the file
	sprintf(filename, "%s/OMGAUDIO/03GINF22.DAT", getDriveLetter());
	f = fopen(filename, "wb");
	if (f == NULL)
	{
		fprintf(fp, "error can't open file %s\n", filename);
		fflush(fp);
		return false;
	}

	//write the headers
	sonyFileHeader header;
	sonyObjectPointer Opointer;
	sonyObject obj;

	memcpy( header.magic, "GPIF", 4 );
	writeHeader(&header, f);

	memcpy( Opointer.magic, "GPFB", 4 );
	Opointer.length = UINT32_SWAP_BE_LE(16);
	Opointer.offset = 0x20000000;
	writeObjectPointer(&Opointer, f);

	memcpy( obj.magic, "GPFB", 4 );
	obj.count = 0;//UINT16_SWAP_BE_LE(list.size());
	obj.size = UINT16_SWAP_BE_LE(784); //wtf?
	obj.padding[0] = 0;
	obj.padding[1] = 0;
	writeObject(&obj, f);

	fclose(f);
	return (true);
}

bool SonyDb::write_TrackNumber(vector<Song *> songsToSend)
{
	FILE *f;
	char tmp[512];

	//open the file
	sprintf(tmp, "%s/OMGAUDIO/TRACKS.DAT", getDriveLetter());
	f = fopen(tmp, "wb");
	if (f == NULL)
	{
		fprintf(fp, "error can't open file %s\n", tmp);
		fflush(fp);
		return false;
	}

	for (vector<Song *>::iterator song = songsToSend.begin(); song != songsToSend.end(); song++)
	{
		if ((*song)->statusOfSong == EMPTYTRACK)
			continue;
		sprintf(tmp, "%8i\n", (*song)->track_nr);
		fwrite(tmp, 1, 9, f);
	}

	fclose(f);
	return (true);
}


/* TOOLS */
utf16char *ansi_to_utf16(const char  *str, long len, bool endian)
{

	utf16char *dest=(utf16char*)malloc(sizeof(utf16char) * len);
	memset(dest, 0, sizeof(utf16char) * len);

	int j;
	for (j = 0; j < len; j++)
		dest[j] = 0;

	if(!str) return dest; //Return an empty buffer of the size needed

	wchar_t *wdest=(wchar_t*)malloc(sizeof(wchar_t) * (len+1));
	memset(wdest, 0, sizeof(wchar_t)*len);

#ifdef _WIN32
	int num = MultiByteToWideChar(CP_ACP,0,str,-1,(WCHAR*)wdest,strlen(str)+1);
#else
	mbstowcs(wdest, str, 2048);
#endif

	for (j = 0; j < len; j++)
	{
		if (wdest[j] != 0)
			dest[j] = wdest[j];
		else
			dest[j] = 0;
	}
	free(wdest);

	//endianness
	if (endian)
	{
		for (int i = 0; i < len; i++)
			dest[i] = UINT16_SWAP_BE_LE(dest[i]);
	}
	return dest;
}

char *utf16_to_ansi(const utf16char *str, long len, bool endian)
{
	if(!str) return NULL;
	char dest[2048]="";
	char * d=dest;

	wchar_t *src = (wchar_t*)malloc(sizeof(wchar_t) * (len+1));
	memset(src, 0, sizeof(wchar_t) * (len+1));

	for (int j = 0; j < len; j++)
	{
		if (str[j] != 0)
			src[j] = str[j];
		else
			src[j] = 0;
	}

	//endianness
	if (endian)
	{
		for (int i = 0; i < len; i++)
			src[i] = UINT16_SWAP_BE_LE(src[i]);
	}
	memset(d, 0, 2048);

#ifdef _WIN32
	WideCharToMultiByte(CP_ACP,0,(WCHAR*)src,-1,d,sizeof(dest)-1,NULL,NULL);
#else
	wcstombs(d, src, 2048);
#endif

	dest[2047]=0;
	return strdup(dest);
}



//#define RETIFNZ(v) if ((v)<0) return use_dir?1:-1; if ((v)>0) return use_dir?-1:1;

bool sortByIndex(Song *a, Song *b)
{
	return (a->sonyDbOrder < b->sonyDbOrder);
}

bool sortByTrackNumber(Song *a, Song *b)
{
	return (a->track_nr < b->track_nr);
}

bool sortByAlbumName(Song *a, Song *b)
{
	return (STRCMP_NULLOK(a->album, b->album) < 0);
}

bool sortByArtistName(Song *a, Song *b)
{
	return (STRCMP_NULLOK(a->artist, b->artist) < 0);
}

bool sortByTitleName(Song *a, Song *b)
{
	return (STRCMP_NULLOK(a->title, b->title) < 0);
}

bool sortByGenreName(Song *a, Song *b)
{
	return (STRCMP_NULLOK(a->genre, b->genre) < 0);
}


bool sortPlaylist(Playlist s, Playlist s2)
{
	return (STRCMP_NULLOK(s.name, s2.name) < 0);
}

bool sortByPlaylistIndex(Playlist s, Playlist s2)
{
	return (s.index < s2.index);
}
