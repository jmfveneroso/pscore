#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include "config.h"

using namespace std;

int main() {
  auto start = std::chrono::system_clock::now();

  // Load params. Defined in: "./include/pscorefactory.h".
  cout << "Loading params..." << endl;

  Params params2;
  vector<unsigned int> group_ids {
    3, 11, 2, 0, 1, 9, 14, 4, 13, 16, 15, 6
  };

  for (auto& id : group_ids) {
    params2.refgroups.push_back(id);
  }

  Dataset dataset;
  dataset.load("../datasets/");

  chrono::duration<double> elapsed_seconds = chrono::system_clock::now() - start;
  cout << "==== Loaded dataset in " << elapsed_seconds.count() << " seconds." << endl;
  start = chrono::system_clock::now();

  // Rankings defined in: "./rfscpp/include/pscorefactory.h".
  Pscore pscore(dataset);
  Rankings rankings = pscore.rank(params2);

  elapsed_seconds = chrono::system_clock::now() - start;
  cout << "==== Rank by groups took " << elapsed_seconds.count() << " seconds." << endl;
  start = chrono::system_clock::now();

  params2.refgroups.clear();
  vector<unsigned int> author_ids {
    2347762, 2299648, 1635670, 184482
  };

  for (auto& id : author_ids) {
    params2.refauthors.push_back(id);
  }

  rankings = pscore.rank(params2);

  elapsed_seconds = chrono::system_clock::now() - start;
  cout << "==== Rank by authors took " << elapsed_seconds.count() << " seconds." << endl;
  start = chrono::system_clock::now();

  params2.refauthors.clear();
  vector<unsigned int> venue_ids {
    1140, 5137, 5156
  };

  for (auto& id : venue_ids) {
    params2.refvenues.push_back(id);
  }

  rankings = pscore.rank(params2);

  elapsed_seconds = chrono::system_clock::now() - start;
  cout << "==== Rank by venues took " << elapsed_seconds.count() << " seconds." << endl;

  return 0;
}
