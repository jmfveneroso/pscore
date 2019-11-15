#include <boost/python.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include "pscore.h"

using namespace std;
using namespace boost::python;
namespace py = boost::python;

template<class T>
struct VecToList {
  static PyObject* convert(const std::vector<T>& vec) {
    boost::python::list* l = new boost::python::list();
    for (size_t i = 0; i < vec.size(); i++) {
      l->append(vec[i]);
    }
    return l->ptr();
  }
};

class ReputationGraph {
  std::string msg = "bla";

 public:
  void set(std::string msg) { this->msg = msg; }

  vector<int> RankByVenues() {
    auto start = std::chrono::system_clock::now();
  
    Pscore::Pscore pscore;
    pscore.LoadDataset("../datasets/");
  
    chrono::duration<double> elapsed_seconds = chrono::system_clock::now() - start;
    cout << "==== Loaded dataset in " << elapsed_seconds.count() << " seconds." << endl;
    start = chrono::system_clock::now();

    vector<unsigned int> group_ids {
      3, 11, 2, 0, 1, 9, 14, 4, 13, 16, 15, 6
    };
    Pscore::Rankings rankings = pscore.RankByGroups(group_ids);
  
    elapsed_seconds = chrono::system_clock::now() - start;
    cout << "==== Rank by groups took " << elapsed_seconds.count() << " seconds." << endl;

    vector< pair<unsigned, float> > venues = rankings.get_venues(0, 200);
  
    vector<int> venue_ids;
    for (pair<unsigned, float> v : venues) {
      venue_ids.push_back(v.first);
    }

    return venue_ids;
  }

  vector<int> RankByAuthors() {
    auto start = std::chrono::system_clock::now();
  
    Pscore::Pscore pscore;
    pscore.LoadDataset("../datasets/");
  
    chrono::duration<double> elapsed_seconds = chrono::system_clock::now() - start;
    cout << "==== Loaded dataset in " << elapsed_seconds.count() << " seconds." << endl;
    start = chrono::system_clock::now();

    vector<unsigned int> author_ids {
      2347762, 2299648, 1635670, 184482
    };
  
    Pscore::Rankings rankings = pscore.RankByAuthors(author_ids);
  
    vector< pair<unsigned, float> > venues = rankings.get_venues(0, 200);
  
    vector<int> venue_ids;
    for (pair<unsigned, float> v : venues) {
      venue_ids.push_back(v.first);
    }

    return venue_ids;
  }

  vector<int> RankByGroups() {
    auto start = std::chrono::system_clock::now();
  
    Pscore::Pscore pscore;
    pscore.LoadDataset("../datasets/");
  
    chrono::duration<double> elapsed_seconds = chrono::system_clock::now() - start;
    cout << "==== Loaded dataset in " << elapsed_seconds.count() << " seconds." << endl;

    start = chrono::system_clock::now();
    vector<unsigned int> venue_ids_input {
      1140, 5137, 5156
    };
  
    Pscore::Rankings rankings = pscore.RankByVenues(venue_ids_input);
  
    elapsed_seconds = chrono::system_clock::now() - start;
    cout << "==== Rank by venues took " << elapsed_seconds.count() << " seconds." << endl;
  
    vector< pair<unsigned, float> > venues = rankings.get_venues(0, 200);
  
    vector<int> venue_ids;
    for (pair<unsigned, float> v : venues) {
      venue_ids.push_back(v.first);
    }

    return venue_ids;
  }
};

BOOST_PYTHON_MODULE(pscore) {
  py::to_python_converter<vector<int, std::allocator<int> >, VecToList<int> >();

  class_<ReputationGraph>("Graph")
    .def("RankByVenues", &ReputationGraph::RankByVenues)
    .def("RankByAuthors", &ReputationGraph::RankByAuthors)
    .def("RankByGroups", &ReputationGraph::RankByGroups)
    .def("set", &ReputationGraph::set);
}
