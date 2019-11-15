#ifndef __PSCORE_H__
#define __PSCORE_H__

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <cmath>
#include <chrono>
#include <queue>
#include <algorithm>
#include <fstream>
#include <stdlib.h>

using namespace std;

namespace Pscore {

typedef map< unsigned, map<unsigned, double> > GraphMap;

class Paper {
 public:
  short year;
  unsigned venueid;
  set<unsigned> authorids;
  set<unsigned> groupids;
};

class Venue {
 public:
  set<unsigned> paperids;
  unsigned id;
  string key;
};

class Author {
 public:
  unsigned id;
  string name;
  set<unsigned> paperids;
  set<unsigned> groupids;
};

class Group {
 public:
  unsigned id;
  string name;
  set<unsigned> paperids;
};

class Rankings {
  static vector<pair<unsigned, float> > get(map<unsigned, float>&, unsigned, unsigned);

 public:
  map<unsigned, float> venues;
  map<unsigned, float> authors;
  map<unsigned, float> groups;
  vector<unsigned> selectedAuthors;

  vector< pair<unsigned, float> > get_venues(unsigned, unsigned);
  vector< pair<unsigned, float> > get_authors(unsigned, unsigned);
  vector< pair<unsigned, float> > get_groups(unsigned, unsigned);
};

class Pscore {
  vector<Paper> papers_;
  vector<Venue> venues_;
  vector<Author> authors_;
  vector<Group> groups_;

  pair<short, short> years_ = { 1940, 2020 };
  int top_authors_ = 200;
  float ds_ = 0.5;
  float dt_ = 0.0;

  GraphMap Transpose(const GraphMap&);

  pair< map<unsigned, float>, map<unsigned, float> > ConvergeMarkovChain(
    const GraphMap& Nss,
    const GraphMap& Nst,
    const GraphMap& Ntt
  );

  inline bool IgnorePaper(const Paper&);
  void AddPaper(GraphMap&, unsigned, unsigned);
  map<unsigned, float> RankVenuesWithAuthorSeeds(vector<unsigned>&);
  map<unsigned, float> RankVenuesWithGroupSeeds(vector<unsigned>&);
  map<unsigned, float> RankAuthorsWithVenueSeeds(vector<unsigned>&);
  map<unsigned, float> CalculateAuthorCollaterals(map<unsigned, float>&);
  map<unsigned, float> CalculateGroupCollaterals(map<unsigned, float>&);

  inline bool IsValid(const Venue&);
  void LoadVenues(const string&);
  void LoadAuthors(const string&);
  void LoadPapers(const string&);
  void LoadAuthorPaper(const string&);
  void LoadGroups(const string&);
  void LoadGroupAuthor(const string&);

 public:
  Pscore() {}

  Rankings RankByVenues(vector<unsigned>&);
  Rankings RankByAuthors(vector<unsigned>&);
  Rankings RankByGroups(vector<unsigned>&);
  void LoadDataset(const string&);
};

} // End of namespace Pscore.

#endif /* __PSCORE_H__ */
