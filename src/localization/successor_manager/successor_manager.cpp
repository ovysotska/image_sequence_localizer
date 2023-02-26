/** vpr_relocalization: a library for visual place recognition in changing
** environments with efficient relocalization step.
** Copyright (c) 2017 O. Vysotska, C. Stachniss, University of Bonn
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/

#include "successor_manager/successor_manager.h"
#include "database/idatabase.h"
#include "relocalizers/irelocalizer.h"
#include "successor_manager/node.h"

#include <glog/logging.h>

#include <algorithm>
#include <fstream>
#include <iostream>
using std::vector;

SuccessorManager::SuccessorManager(iDatabase *database,
                                   iRelocalizer *relocalizer, int fanOut)
    : database_{database}, relocalizer_{relocalizer}, fanOut_{fanOut} {
  CHECK(database_ != nullptr) << "Database is not set.";
  CHECK(relocalizer_ != nullptr) << "Relocalizer is not set.";
  CHECK(fanOut_ > 0) << "Invalid fanOut value: " << fanOut
                     << ". The value should be > 0.";
}

/**
 * @brief      Sets the similar places.
 *
 * @param[in]  filename  The filename
 *
 * @return     { description_of_the_return_value }
 */
bool SuccessorManager::setSimilarPlaces(const std::string &filename) {
  std::ifstream in(filename.c_str());
  if (!in) {
    printf("[ERROR][SuccessorManager] Cannot open file %s\n", filename.c_str());
    printf("[======================= Similar places were not set\n");
    return false;
  }
  while (!in.eof()) {
    int ref_id_from, ref_id_to;
    in >> ref_id_from >> ref_id_to;
    _sameRefPlaces[ref_id_from].insert(ref_id_to);
    _sameRefPlaces[ref_id_to].insert(ref_id_from);
  }
  in.close();
  printf("[INFO][SuccessorManager] Similar Places were set\n");
  return true;
}

/**
 * @brief      Gets the successors.
 *
 * @param[in]  quId   The qu identifier
 * @param[in]  refId  The reference identifier
 *
 * @return     The successors.
 */
std::unordered_set<Node> SuccessorManager::getSuccessors(const Node &node) {
  _successors.clear();

  LOG_IF(FATAL, node == kSourceNode)
      << "Requested to connect the source node. Robot should "
         "be lost before first image. Use 'getSuccessorsIfLost' function "
         "instead.";

  CHECK(node.quId >= 0 && node.refId >= 0)
      << "Invalid image ids, query id: " << node.quId
      << ", ref id: " << node.refId;

  // check for regular successor
  getSuccessorFanOut(node.quId, node.refId);
  // check for additional successors based on similar places
  if (!_sameRefPlaces.empty()) {
    getSuccessorsSimPlaces(node.quId, node.refId);
  }
  // printf("Successors were computed %d \n", _successors.size());
  return _successors;
}

/**
 * @brief      Gets successors based on fanout for node (quId, refId) from
 * database. Select followers based on fanOut_;
 *
 * @param[in]  quId   query index
 * @param[in]  refId  reference index
 *
 */
void SuccessorManager::getSuccessorFanOut(int quId, int refId) {
  int left_ref = std::max(refId - fanOut_, 0);
  int right_ref = std::min(refId + fanOut_, database_->refSize() - 1);

  for (int succ_ref = left_ref; succ_ref <= right_ref; ++succ_ref) {
    Node succ;
    double succ_cost = database_->getCost(quId + 1, succ_ref);
    succ.set(quId + 1, succ_ref, succ_cost);
    _successors.insert(succ);
  }
}

/**
 * @brief      Gets the successors if there are similar places.
 *
 * @param[in]  quId  query index
 */
void SuccessorManager::getSuccessorsSimPlaces(int quId, int refId) {
  auto found = _sameRefPlaces.find(refId);
  std::set<int> simPlaces;
  if (found == _sameRefPlaces.end()) {
    // no similar places for the place refId
    // do not update _successors
  } else {
    simPlaces = _sameRefPlaces[refId];
    for (int simPlace : simPlaces) {
      getSuccessorFanOut(quId, simPlace);
    }
  }
}

/**
 * @brief      Gets the successor by performing the relocalization action.
 * hashes the corresponding feature of the query image and retrieves a set of
 * potential candidates from the pre-computed hash table
 *
 * @param[in]  node  The node
 *
 * @return     The successors if lost.
 */
std::unordered_set<Node>
SuccessorManager::getSuccessorsIfLost(const Node &node) {
  _successors.clear();
  int succ_qu_id = node.quId + 1;
  std::vector<int> candidates = relocalizer_->getCandidates(succ_qu_id);

  if (candidates.empty()) {
    LOG(INFO) << "No candidate images found";
    // propagate one node as if moving
    Node succ;
    double succ_cost = database_->getCost(succ_qu_id, node.refId);
    succ.set(succ_qu_id, node.refId, succ_cost);
    _successors.insert(succ);
  } else {
    for (const auto &candId : candidates) {
      Node succ;
      double succ_cost = database_->getCost(succ_qu_id, candId);
      succ.set(succ_qu_id, candId, succ_cost);
      _successors.insert(succ);
      succ.print();
    }
  }
  return _successors;
}
