#include "pscore.h"

namespace Pscore {

class Compare {
 public:
  bool operator() (pair<unsigned, float>& l, pair<unsigned, float>& r) {
    if (l.second < r.second)
      return true;
    if (l.second == r.second)
      return l.first > r.first;
    return false;
  }
};

vector<pair<unsigned, float> > Rankings::get(
  map<unsigned, float>& scores, unsigned init, unsigned count
) {
  vector<pair<unsigned, float> > ranking;
  priority_queue< pair<unsigned, float>, vector< pair<unsigned, float> >, Compare> mypq;

  for (auto id_score : scores)
    mypq.push(id_score);

  unsigned i = 0;
  while (!mypq.empty() && i < init + count) {
    if (i >= init)
      ranking.push_back(mypq.top());
    mypq.pop();
    ++i;
  }
  return ranking;
}

vector<pair<unsigned, float> > Rankings::get_venues(unsigned init, unsigned count) {
  return Rankings::get(venues, init, count);
}

vector<pair<unsigned, float> > Rankings::get_authors(unsigned init, unsigned count) {
  return Rankings::get(authors, init, count);
}

vector<pair<unsigned, float> > Rankings::get_groups(unsigned init, unsigned count) {
  return Rankings::get(groups,init,count);
}

GraphMap Pscore::Transpose(const GraphMap &sdw) {
  GraphMap dsw;
  for (auto s = sdw.begin(); s != sdw.end(); ++s) {
    for (auto dw : s->second) {
      auto d = dw.first;
      auto w = dw.second;
      if (dsw.find(d)==dsw.end())
        dsw[d] = map<unsigned ,double>();
      dsw[d][s->first] = w;
    }
  }
  return dsw;
}

pair< map<unsigned, float>, map<unsigned, float> > Pscore::ConvergeMarkovChain(
  const GraphMap& ss,
  const GraphMap& st,
  const GraphMap& tt
) {
  double epsilon = 1e-6;
  map<unsigned, float> sources, targets;
  GraphMap ts = Transpose(st);

  vector<GraphMap> P_quads = { ss, st, ts, tt };
  vector<double> P_weights = { ds_, 1.-ds_, 1.-dt_, dt_ };

  unsigned quad_index = 0;
  for (auto & P_quad : P_quads) {
    for (auto & s : P_quad) {
      double total = 0.;

      for (auto & d : s.second)
        total += d.second;

      for (auto & d : s.second)
        d.second = P_weights[quad_index] * d.second / total;
    }
    ++quad_index;
  }

  unsigned s, t, s2, t2;
  bool conv = false;

  GraphMap Pss_transp = Transpose(P_quads[0]),
  Pst_transp = Transpose(P_quads[1]),
  Pts_transp = Transpose(P_quads[2]),
  Ptt_transp = Transpose(P_quads[3]);

  set<unsigned> sourceids,targetids;
  for (auto id : st) {
    sourceids.insert(id.first);
    sources[id.first] = 1.0 / st.size();
  }

  for (auto id : ts) {
    targetids.insert(id.first);
    targets[id.first] = 1.0 / ts.size();
  }

  unsigned iter = 0;
  while (!conv && iter < 100) {
    map<unsigned, float> sources2;
    map<unsigned, float> targets2;

    // For source, target.
    for (auto stw : Pts_transp) {
      float score = 0;
      s = stw.first;
      for (auto tw : stw.second) {
        t = tw.first;
        score += tw.second * targets[t];
      }
      sources2[s] = score;
    }

    // For s, s2.
    for (auto ss2w : Pss_transp) {
      float score = 0;
      s = ss2w.first;
      for (auto s2w : ss2w.second) {
        s2 = s2w.first;
        score += s2w.second * sources[s2];
      }
      sources2[s] += score;
    }

    // For T, S.
    for (auto tsw : Pst_transp) {
      float score = 0;
      t = tsw.first;
      for (auto sw : tsw.second) {
        s = sw.first;
        score += sw.second * sources[s];
      }
      targets2[t] = score;
    }

    // For T, T2.
    for (auto tt2w : Ptt_transp) {
      float score = 0;
      t = tt2w.first;
      for (auto t2w : tt2w.second) {
        t2 = t2w.first;
        score += t2w.second * targets[t2] ;
      }
      targets2[t] += score;
    }

    double maxdiff = 0;

    for (auto s : sourceids) {
      double diff = abs(double(sources[s] - sources2[s]));
      if (diff > maxdiff)
        maxdiff = diff;
    }

    for (auto t : targetids) {
      double diff = abs(double(targets[t] - targets2[t]));
      if (diff > maxdiff)
        maxdiff = diff;
    }

    if (maxdiff < epsilon)
      conv = true;

    iter++;
    sources = sources2;
    targets = targets2;
  }

  return make_pair(sources, targets);
}

inline bool Pscore::IgnorePaper(const Paper& paper) {
  return paper.year < years_.first || paper.year > years_.second;
}

void Pscore::AddPaper(GraphMap& source_target, unsigned s, unsigned t) {
  if (source_target[s].find(t) == source_target[s].end())
    source_target[s][t] = 0;

  source_target[s][t] += 1;
}

map<unsigned, float> Pscore::CalculateAuthorCollaterals(map<unsigned, float>& venue_scores) {
  map<unsigned, float> scores;
  vector<float> author_scores(2352275, 0); // TODO: hardcoded.

  int count = 0;
  for (auto it = venue_scores.begin(); it != venue_scores.end(); ++it) {
    unsigned venueid = it->first;
    float venuescore= it->second;
    Venue& venue = venues_[venueid];

    for (unsigned paperid : venue.paperids) {
      Paper& paper = papers_[paperid];
      if (IgnorePaper(paper)) continue;

      for (unsigned authorid : paper.authorids) {
        author_scores[authorid] += venuescore;
        count++;
      }
    }
  }

  for (unsigned int i = 0; i < 2352275; i++) {
    scores[i] = author_scores[i];
  }

  return scores;
}

map<unsigned, float> Pscore::CalculateGroupCollaterals(map<unsigned, float>& venue_scores) {
  map<unsigned, float> scores;
  for (auto it = venue_scores.begin(); it != venue_scores.end(); ++it) {
    unsigned venueid = it->first;
    float venuescore = it->second;
    Venue& venue = venues_[venueid];

    for (unsigned paperid : venue.paperids) {
      Paper & paper = papers_[paperid];
      if (IgnorePaper(paper)) continue;
      for (unsigned authorid : paper.authorids){
        float size = authors_[authorid].groupids.size();
        for (unsigned groupid : authors_[authorid].groupids){
          if (scores.find(groupid) == scores.end()){
            scores[groupid] = 0;
          }
          scores[groupid] += (venuescore / size);
        }
      }
    }
  }
  return scores;
}

map<unsigned, float> Pscore::RankVenuesWithAuthorSeeds(vector<unsigned>& author_ids) {
  map<unsigned, float> scores;
  GraphMap source_source;
  GraphMap source_target;
  GraphMap target_target;
  set<unsigned> sourceids = set<unsigned>(author_ids.begin(), author_ids.end());
  set<unsigned> targetids;

  for (unsigned authorid : author_ids) {
    Author& author = authors_[authorid];
    for (unsigned paperid : author.paperids) {
      Paper& paper = papers_[paperid];
      if (IgnorePaper(paper)) continue;

      unsigned venueid = paper.venueid;

      AddPaper(source_target, authorid, venueid);
      for (unsigned authorid2 : paper.authorids)
        if (
          sourceids.find(authorid2) != sourceids.end() &&
          authorid != authorid2 // no self loop
        ) {
          AddPaper(source_source, authorid, authorid2);
          AddPaper(source_source, authorid2, authorid);
        }
      targetids.insert(venueid);
    }
  }

  pair< map<unsigned, float>, map<unsigned, float> > stscores = ConvergeMarkovChain(
    source_source,
    source_target,
    target_target
  );

  return stscores.second;
}

map<unsigned, float> Pscore::RankVenuesWithGroupSeeds(vector<unsigned>& group_ids) {
  map<unsigned, float> scores;

  GraphMap source_source;
  GraphMap source_target;
  GraphMap target_target;
  set<unsigned> sourceids = set<unsigned>(group_ids.begin(), group_ids.end());
  set<unsigned> targetids;

  for (unsigned groupid : group_ids) {
    Group& group = groups_[groupid];
    for (unsigned paperid : group.paperids) {
      Paper& paper = papers_[paperid];
      if (IgnorePaper(paper)) continue;
      unsigned venueid = paper.venueid;

      AddPaper(source_target, groupid, venueid);
      for (unsigned groupid2 : paper.groupids)
        if (
          sourceids.find(groupid2) != sourceids.end() &&
          groupid != groupid2 
        ) {
          AddPaper(source_source, groupid, groupid2);
          AddPaper(source_source, groupid2, groupid);
        }
      targetids.insert(venueid);
    }
  }

  pair< map<unsigned, float>, map<unsigned, float> > stscores = ConvergeMarkovChain(
    source_source,
    source_target,
    target_target
  );

  return stscores.second;
}

map<unsigned, float> Pscore::RankAuthorsWithVenueSeeds(vector<unsigned>& venue_ids) {
  map<unsigned, float> scores;

  GraphMap source_source;
  GraphMap source_target;
  GraphMap target_target;
  set<unsigned> sourceids = set<unsigned>(venue_ids.begin(), venue_ids.end());
  set<unsigned> targetids;

  for (unsigned venueid : venue_ids) {
    Venue& venue = venues_[venueid];
    for (unsigned paperid : venue.paperids){
      Paper& paper = papers_[paperid];
      if (IgnorePaper(paper)) continue;
      for (unsigned authorid : paper.authorids){
        AddPaper(source_target, venueid, authorid);

        Author& author = authors_[authorid];
        for (unsigned paperid2 : author.paperids){
          Paper& paper2 = papers_[paperid2];
          unsigned& venueid2 = paper2.venueid;
          if (
            sourceids.find(venueid2) != sourceids.end() && // venueid2 is a reputation source
            venueid != venueid2 // no self loop
          ) {
            AddPaper(source_source, venueid, venueid2);
            AddPaper(source_source, venueid2, venueid);
          }
        }

        targetids.insert(authorid);
      }
    }
  }

  pair< map<unsigned, float>, map<unsigned, float> > stscores = ConvergeMarkovChain(
    source_source,
    source_target,
    target_target
  );

  return stscores.second;
}

Rankings Pscore::RankByVenues(vector<unsigned>& venue_ids) {
  Rankings rankings;

  // V <=> A
  map<unsigned, float> temp_authors = RankAuthorsWithVenueSeeds(venue_ids);

  // A* <=> V
  vector< pair<unsigned, float> > sorted_authors;
  for (pair<unsigned, float> author : temp_authors){
    sorted_authors.push_back(author);
  }

  sort(sorted_authors.begin(), sorted_authors.end(), [](
    const std::pair<unsigned, float>& e1, const std::pair<unsigned, float>& e2
  ) {
    return e1.second > e2.second;
  });

  unsigned int limit = ((unsigned int) top_authors_ <= (unsigned int) sorted_authors.size()) ? 
              top_authors_ :
              (unsigned int) sorted_authors.size();

  vector<unsigned> selected_authors(limit);
  rankings.selectedAuthors = vector<unsigned>(limit);

  for (unsigned int it = 0; it < limit; it++){
    vector<pair<unsigned, float> >::const_iterator author = sorted_authors.begin()+it;
    selected_authors[it] = author->first;
    rankings.selectedAuthors[it] = author->first;
  }

  rankings.venues = RankVenuesWithAuthorSeeds(selected_authors);

  // V --> A
  rankings.authors = CalculateAuthorCollaterals(rankings.venues);

  // V --> G
  rankings.groups = CalculateGroupCollaterals(rankings.venues);
  return rankings;
}

Rankings Pscore::RankByAuthors(vector<unsigned>& author_ids) {
  Rankings rankings;

  // A <=> V
  rankings.venues = RankVenuesWithAuthorSeeds(author_ids);

  // V --> A
  rankings.authors = CalculateAuthorCollaterals(rankings.venues);

  // V --> G
  rankings.groups = CalculateGroupCollaterals(rankings.venues);
  return rankings;
}

Rankings Pscore::RankByGroups(vector<unsigned>& group_ids) {
  Rankings rankings;

  // G <=> V
  rankings.venues = RankVenuesWithGroupSeeds(group_ids);

  // V --> A
  rankings.authors = CalculateAuthorCollaterals(rankings.venues);

  // V --> G
  rankings.groups  = CalculateGroupCollaterals(rankings.venues);
  return rankings;
}

inline bool Pscore::IsValid(const Venue& venue) {
  // Ignore corr.
  return venue.key != "journals/corr" && 
         !venue.key.empty() && 
         (venue.key[0] == 'c' || venue.key[0] == 'j');
}

void Pscore::LoadVenues(const string& filename) {
  string line;
  unsigned id = 0;

  ifstream f(filename.c_str());
  while (getline(f,line)) ++id;

  f.close();
  venues_.resize(id);

  id = 0;
  ifstream f2(filename.c_str());
  while (getline(f2,line)) {
    venues_[id].id = id;
    venues_[id].key = line.substr(0, line.find(","));
    ++id;
  }
  f2.close();
}

void Pscore::LoadAuthors(const string& filename) {
  string line;
  unsigned id = 0;

  ifstream f(filename.c_str());
  while (getline(f,line)) {
    ++id;
  }
  f.close();
  authors_.resize(id);

  id = 0;
  ifstream f2(filename.c_str());
  while (getline(f2,line)) {
    authors_[id].id = id;
    authors_[id].name = line;
    authors_[id].groupids = set<unsigned>({});
    ++id;
  }
  f2.close();
}

void Pscore::LoadPapers(const string& filename) {
  char pkey[256], year_str[5];
  string line;
  unsigned id = 0;
  short year;
  unsigned venueid;

  ifstream f(filename.c_str());
  while (getline(f,line)) {
    ++id;
  }
  f.close();
  papers_.resize(id);

  id = 0;
  ifstream f2(filename.c_str());
  while (getline(f2, line)) {
    sscanf(line.c_str(),"%s\t%s\t%u",pkey,year_str,&venueid);

    year = -1;
    if (string(year_str) != "")
      year = atoi(year_str);

    Paper& paper = papers_[id];
    paper.year = year;
    paper.venueid = venueid;

    Venue& venue = venues_[venueid];
    if (IsValid(venue)) {
      venue.paperids.insert(id);
    }

    ++id;
  }
  f.close();
}

void Pscore::LoadAuthorPaper(const string& filename) {
  string line;
  unsigned authorid;
  unsigned paperid;

  ifstream f(filename.c_str());
  while (getline(f,line)) {
    sscanf(line.c_str(), "%u,%u", &authorid, &paperid);

    Author& author = authors_[authorid];
    Paper& paper = papers_[paperid];

    // Ignore invalid venues and invalid authors.
    if (IsValid(venues_[paper.venueid])) {
      author.paperids.insert(paperid);
      paper.authorids.insert(authorid);
    }
  }
  f.close();
}

void Pscore::LoadGroups(const string& filename) {
  string line;
  unsigned id = 0;

  ifstream f(filename.c_str());
  while (getline(f,line)) {
    ++id;
  }
  f.close();
  groups_.resize(id);

  id = 0;
  ifstream f2(filename.c_str());
  getline(f2, line);
  while (getline(f2, line)) {
    groups_[id].id = id;
    groups_[id].name = line.substr(0, line.find(","));
    ++id;
  }
  f2.close();
}

void Pscore::LoadGroupAuthor(const string& filename) {
  string line; int count_ignored = 0;

  map<string, unsigned> group_name_id;
  map<string, unsigned> author_name_id;

  for (auto& group : groups_) group_name_id [group.name] = group.id;
  for (auto& author: authors_) author_name_id[author.name] = author.id;

  ifstream f(filename.c_str());
  while (getline(f,line)) {
    size_t sep = line.find(",");
    string author_name = line.substr(0,sep);
    string group_name  = line.substr(sep+1);

    unsigned groupid = (unsigned)-1;
    if (group_name_id.find(group_name)!=group_name_id.end())
      groupid = group_name_id[group_name];

    unsigned authorid = (unsigned)-1;
    if (author_name_id.find(author_name)!=author_name_id.end())
      authorid = author_name_id[author_name];

    if (groupid == (unsigned)-1 || authorid == (unsigned)-1) {
      count_ignored += 1;
      continue;
    } else {
      authors_[authorid].groupids.insert(groupid);
    }

    Group& group = groups_[groupid];
    Author& author = authors_[authorid];

    for (unsigned paperid : author.paperids) {
      group.paperids.insert(paperid);
      papers_[paperid].groupids.insert(groupid);
    }
  }
  f.close();
}

void Pscore::LoadDataset(const string& p) {
  string path = p;
  LoadVenues(path + "/dblp/venues.csv");
  LoadAuthors(path + "/dblp/authors.csv");
  LoadPapers(path + "/dblp/papers.csv");
  LoadAuthorPaper(path + "/dblp/authorpaper.csv");
  LoadGroups(path + "/dblpgroups/groups.csv");
  LoadGroupAuthor(path + "/dblpgroups/groupauthor.csv");
}

}; // End of namespace Pscore.
