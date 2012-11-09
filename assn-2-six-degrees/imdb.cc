using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

typedef struct{
  const void * name;
  const void * adress;
}Key;
 
imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

int cmpPlayer(const void * v1, const void * v2)
{
  
  Key * s1 = (Key*)v1;
  int * s2 = (int*)v2;
 
  char * key = (char*)(s1->name);
  char * target = (char*)(s1->adress)+*s2;

  // cout << "comparing " << key <<" with " << target <<"("<< (char*)(s1->adress)+*s2 <<")"<< endl;
  return strcmp(key, target);
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const 
{
  Key temp; temp.name = &player[0]; temp.adress = actorFile;
  Key * artista = &temp;
  void * find = bsearch(artista, (char*)actorFile + sizeof(int), *(int*)actorFile, sizeof(int), cmpPlayer);

  if(find != NULL)
    { 
      //cout << "success!" << endl;
      char * loc = (char*)actorFile + *(int*)find;
      int nameLength, totalBytes;
      char * movLoc;
      short numMovies;
      int letters = player.size()+1;

      film temp;

      if(letters%2 == 0)
	nameLength = letters;
      else
	nameLength = letters+1;
      
      numMovies = *(short*)(loc + nameLength);
      totalBytes = nameLength + sizeof(short); totalBytes = totalBytes + totalBytes%4;
      movLoc = (loc + totalBytes);
      for(int i=0; i<numMovies;i++)
	{
	  string title = (char*)movieFile + *(int*)(movLoc+sizeof(int)*i);;
	  int year = *((char*)movieFile + *(int*)(movLoc+sizeof(int)*i) +title.size()+1);
	  temp.title = title;
	  temp.year = year+1900;
	  films.push_back(temp);
	}
      return true;
    }

  return false;
}


int cmpFilms(const void * v1, const void * v2)
{
  Key * s1 = (Key*)v1;
  int * s2 = (int*)v2;
 
  char * key = &(((film*)(s1->name))->title)[0];
  char * target = (char*)(s1->adress)+*s2;

  //cout << "comparing " << key <<" with " << target << endl;
  int result =  strcmp(key, target);
  if(result == 0)
    {
      int yearTarget = *(char*)((char*)(s1->adress)+*s2+strlen(target)+1);
      yearTarget = yearTarget + 1900;
      int yearKey = ((film*)s1->name)->year;
      return yearKey-yearTarget;
    }
  return result;
}

bool imdb::getCast(const film& movie, vector<string>& players) const
{ 
  
  //cout << movie.title << endl;
  Key temp; temp.name = &movie; temp.adress = movieFile;
  Key * pelicula = &temp;
  void * find = bsearch(pelicula, (char*)movieFile + sizeof(int), *(int*)movieFile, sizeof(int),cmpFilms);

  if(find != NULL)
    {
      //cout << "success!" << endl;
      char * loc = (char*)movieFile + *(int*)find;
      int nameLength, totalBytes;
      char * actLoc;
      short numActors;
      int letters = (movie.title).size()+2;

      // film temp;

      if(letters%2 == 0)
	nameLength = letters;
      else
	nameLength = letters+1;
      
      numActors = *(short*)(loc + nameLength);
      totalBytes = nameLength + sizeof(short); totalBytes = totalBytes + totalBytes%4;
      actLoc = (loc + totalBytes);
      for(int i=0; i<numActors;i++)
	{
	  string player = (char*)actorFile + *(int*)(actLoc+sizeof(int)*i);
	  //cout << player << endl;
	  players.push_back(player);
	}
      return true;
    }

  return false; 
}



imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
