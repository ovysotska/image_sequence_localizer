#include "default_relocalizer.h"

#include <glog/logging.h>

DefaultRelocalizer::DefaultRelocalizer(int fanOut, int refSize)
    : fanOut_(fanOut), refSize_(refSize) {
  CHECK(fanOut_ > 0) << "Fan out should be positive.";
  CHECK(refSize > 0) << "RefSize should be positive.";
}
std::vector<int> DefaultRelocalizer::getCandidates(int quId) {
  std::vector<int> candidateIds;
  for (int i = std::max(0, quId - fanOut_);
       i <= std::min(refSize_, quId + fanOut_); i++) {
    candidateIds.push_back(i);
  }
  return candidateIds;
}