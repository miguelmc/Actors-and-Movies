#include <vector>
#include <list>
#include <set>
#include <string>
#include <iostream>
#include <iomanip>
#include "imdb.h"
#include "path.h"
using namespace std;

/**
 * Using the specified prompt, requests that the user supply
 * the name of an actor or actress.  The code returns
 * once the user has supplied a name for which some record within
 * the referenced imdb existsif (or if the user just hits return,
 * which is a signal that the empty string should just be returned.)
 *
 * @param prompt the text that should be used for the meaningful
 *               part of the user prompt.
 * @param db a reference to the imdb which can be used to confirm
 *           that a user's response is a legitimate one.
 * @return the name of the user-supplied actor or actress, or the
 *         empty string.
 */

static string promptForActor(const string& prompt, const imdb& db)
{
  string response;
  while (true) {
    cout << prompt << " [or <enter> to quit]: ";
    getline(cin, response);
    if (response == "") return "";
    vector<film> credits;
    if (db.getCredits(response, credits)) return response;
    cout << "We couldn't find \"" << response << "\" in the movie database. "
	 << "Please try again." << endl;
  }
}

/**
 * Serves as the main entry point for the six-degrees executable.
 * There are no parameters to speak of.
 *
 * @param argc the number of tokens passed to the command line to
 *             invoke this executable.  It's completely ignored
 *             here, because we don't expect any arguments.
 * @param argv the C strings making up the full command line.
 *             We expect argv[0] to be logically equivalent to
 *             "six-degrees" (or whatever absolute path was used to
 *             invoke the program), but otherwise these are ignored
 *             as well.
 * @return 0 if the program ends normally, and undefined otherwise.
 */

static void stall()
{
  string dummy;
  cout << "[Press enter to continue]";
  getline(cin, dummy);
}


film * movieInCommon(vector<film> &credits1, vector<film> &credits2)
{
  for(int i =0; i < (int)credits1.size(); i++)
    {
      for(int j=0; j < (int)credits2.size(); j++)
	{
	  if(credits1[i]==credits2[j])
	    return &credits1[i];
	}
    }
  return NULL;
}

bool checkIfUsed(string name, set<string> & usedNames)
{
  set<string>::iterator it = usedNames.begin();
  for(;it!=usedNames.end();it++)
    {
      if(name == *it)
	return true;
    }
  usedNames.insert(name);
  return false;
}

bool checkIfUsed(film movie, set<film> & usedFilms)
{
  set<film>::iterator it = usedFilms.begin();
  for(;it!=usedFilms.end();it++)
    {
      if(movie == *it)
	return true;
    }
  usedFilms.insert(movie);
  return false;
}

int main(int argc, const char *argv[])
{
  imdb db("/home/mike/cs107/Assignments/2/assn-2-six-degrees-data/little-endian/");//determinePathToData(argv[1])); // inlined in imdb-utils.h
  

  string source, target;
  vector<film> sourceCredits, targetCredits;
  vector<string> cast;

  list <path> camino;
  set<string> usedNames;
  set<film> usedFilms;

  while (true) {
    source = promptForActor("Actor or actress", db);
    if (source == "") break;
    target = promptForActor("Another actor or actress", db);
    if (target == "") break;
    if (source == target) {
      cout << "Good one.  This is only interesting if you specify two different people." << endl;
    } else
      {
	path curr(source);
	camino.push_back(curr);
	while(!camino.empty() && (camino.front()).getLength() < 6)
	  {
	    curr = camino.front();
	    camino.pop_front();
	    db.getCredits(curr.getLastPlayer(),sourceCredits);
	    for(int i =0; i< (int)sourceCredits.size(); i++)
	      {
		if(checkIfUsed(sourceCredits[i], usedFilms))continue;
		//usedFilms.insert(sourceCredits[i]);
		db.getCast(sourceCredits[i], cast);
		for(int j=0; j<(int)cast.size();j++)
		  {
		    if(checkIfUsed(cast[j], usedNames))continue;
		    // usedNames.insert(cast[i]);
		    path temp = curr;
		    temp.addConnection(sourceCredits[i], cast[j]);
		    if(temp.getLastPlayer() == target)
		      {
			cout << temp << endl;
			return 0;
		      }
		    else
		      camino.push_back(temp);
		  }
		cast.clear();
	      }
	    sourceCredits.clear();
	  }

	cout << endl << "No path between those two people could be found." << endl << endl;
      }

    camino.clear();
    usedNames.clear();
    usedFilms.clear();
  }
  cout << "Thanks for playing!" << endl;
  return 0;
}

