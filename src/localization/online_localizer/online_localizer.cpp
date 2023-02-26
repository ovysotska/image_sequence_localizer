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

/* Updated by O. Vysotska in 2023 */

#include "online_localizer/online_localizer.h"
#include "localization_protos.pb.h"
#include "online_localizer/path_element.h"
#include "tools/timer/timer.h"

#include <glog/logging.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace online_localizer {

using std::string;
using std::vector;

const float kMaxLostNodesRatio = 0.8; // 80%

void storeMatchesAsProto(const Matches &matches,
                         const std::string &protoFilename) {
  image_sequence_localizer::MatchingResult matching_result_proto;

  for (const auto &match : matches) {
    image_sequence_localizer::MatchingResult::Match *match_proto =
        matching_result_proto.add_matches();
    match_proto->set_query_id(match.quId);
    match_proto->set_ref_id(match.refId);
    match_proto->set_real(match.state == NodeState::HIDDEN ? 0 : 1);
  }

  std::fstream out(protoFilename,
                   std::ios::out | std::ios::trunc | std::ios::binary);
  if (!matching_result_proto.SerializeToOstream(&out)) {
    LOG(ERROR) << "Couldn't open the file " << protoFilename;
    LOG(ERROR) << "The path is NOT saved.";
    return;
  }
  out.close();
  LOG(INFO) << "The path was written to " << protoFilename;
}

OnlineLocalizer::OnlineLocalizer(SuccessorManager *successorManager,
                                 double expansionRate, double nonMatchingCost) {

  CHECK(successorManager) << "Successor manager is not set.";
  CHECK(expansionRate > 0 && expansionRate <= 1)
      << "Graph expansion rate should be in [0,1]. Obtained: " << expansionRate;
  CHECK(nonMatchingCost > 0)
      << "Non matching cost should be > 0. Obtained: " << nonMatchingCost;

  successorManager_ = successorManager;
  expansionRate_ = expansionRate;
  nonMatchingCost_ = nonMatchingCost;

  pred_[kSourceNode.quId][kSourceNode.refId] = kSourceNode;
  Node source = kSourceNode;
  source.accCost = 0.0;
  accCosts_[kSourceNode.quId][kSourceNode.refId] = source.accCost;
  frontier_.push(source);
  currentBestHyp_ = source;
}

Matches OnlineLocalizer::findMatchesTill(int queryId) {
  CHECK(queryId >= 0) << "Number of queries is <= 0: " << queryId;
  Timer timer;
  // For the first image consider lost
  // for every image in the query set
  for (int qu = 0; qu < queryId; ++qu) {
    // while the graph is not expanded till row 'qu'
    timer.start();
    processImage(qu);
    timer.stop();

    LOG(INFO) << "Matched image " << qu;
    timer.print_elapsed_time(TimeExt::MicroSec);
    LOG(INFO) << "==========================================";
    visualize();
  }
  LOG(INFO) << "Finished matching.";
  if (_vis) {
    _vis->processFinished();
  }
  return getCurrentPath();
}

void OnlineLocalizer::writeOutExpanded(const std::string &filename) const {
  image_sequence_localizer::Patch patch;
  for (const auto &node : expandedRecently_) {
    image_sequence_localizer::Patch::Element *element = patch.add_elements();
    element->set_row(node.quId);
    element->set_col(node.refId);
    element->set_similarity_value(node.idvCost);
  }
  std::fstream out(filename,
                   std::ios::out | std::ios::trunc | std::ios::binary);
  if (!patch.SerializeToOstream(&out)) {
    LOG(ERROR) << "Couldn't open the file" << filename;
    return;
  }
  out.close();
  LOG(INFO) << "Wrote patch " << filename;
}

// frontier picking up routine
void OnlineLocalizer::matchImage(int quId) {
  expandedRecently_.clear();

  std::unordered_set<Node> children;
  if (needReloc_) {
    frontier_ = std::priority_queue<Node>(); // reseting priority_queue
    LOG(INFO) << "RELOCALIZATION";

    // Starting checking graph for expansion starting from the current best
    // matching sequence hypothesis.
    Node expandedNode = currentBestHyp_;
    children = successorManager_->getSuccessorsIfLost(expandedNode);
    // Add only the most promising node to the frontier.
    // need to call update search, since it updates the current best
    // hypothesis
    updateSearch(children);
    children.clear();
    // just one most promising child is added to the graph
    children.insert(currentBestHyp_);
    updateGraph(expandedNode, children);
  } else {
    bool row_reached = false;
    // printf("[INFO][OnlineLocalizer] NOT LOST\n");
    while (!frontier_.empty() && !row_reached) {
      Node expandedNode = frontier_.top();
      frontier_.pop();
      int expanded_row = expandedNode.quId;

      if (!nodeWorthExpanding(expandedNode)) {
        continue;
      }
      children = successorManager_->getSuccessors(expandedNode);
      updateGraph(expandedNode, children);
      updateSearch(children);
      if (expanded_row == quId - 1) {
        row_reached = true;
      } else if (expanded_row >= quId) {
        LOG(FATAL) << "You have expanded the nodes higher than "
                      "current query image id. Something went wrong";
      }
    }
  }
  for (const Node &n : children) {
    expandedRecently_.insert(n);
  }
}

void OnlineLocalizer::processImage(int quId) {
  LOG(INFO) << "Checking image " << quId;
  if (quId == 0) {
    needReloc_ = true;
  }
  matchImage(quId);

  CHECK(!frontier_.empty()) << "Frontier is empty! Something bad happened.";

  // Lost if more than kMaxLostNodesRatio are hidden nodes.
  if (isLost(kSlidingWindowSize_, kMaxLostNodesRatio)) {
    needReloc_ = true;
    LOG(INFO) << "LOST";
  } else {
    needReloc_ = false;
  }
}

bool OnlineLocalizer::nodeWorthExpanding(const Node &node) const {
  if (node == kSourceNode) {
    // source node-> always worth expanding
    return true;
  }
  if (node == currentBestHyp_) {
    return true;
  }

  int row_dist = currentBestHyp_.quId - node.quId;
  CHECK(row_dist >= 0) << "Trying to expand a node further in future "
                       << node.quId << " than current best hypothesis "
                       << currentBestHyp_.quId;

  double mean_cost = computeAveragePathCost();
  double potential_cost = node.accCost + row_dist * mean_cost * expansionRate_;
  if (potential_cost <
      accCosts_.at(currentBestHyp_.quId).at(currentBestHyp_.refId)) {
    return true;
  } else {
    return false;
  }
}

void OnlineLocalizer::updateSearch(const NodeSet &successors) {
  Node possibleHyp = getProminentSuccessor(successors);

  if (possibleHyp.quId > currentBestHyp_.quId) {
    // The search is reaching a query further in sequence than current best
    // hypothesis. This is always better.
    currentBestHyp_ = possibleHyp;
  } else if (possibleHyp.quId == currentBestHyp_.quId) {
    // The alternative path to the same level as current best hypothesis as
    // found.
    double accCost_current =
        accCosts_[currentBestHyp_.quId][currentBestHyp_.refId];
    double accCost_poss = accCosts_[possibleHyp.quId][possibleHyp.refId];
    if (accCost_poss <= accCost_current) {
      // Accumulated cost of new possible hypothesis is smaller than accumulated
      // cost for the current best hypothesis. This becomes new current best
      // hypothesis.
      currentBestHyp_ = possibleHyp;
    }
  }
}

Node OnlineLocalizer::getProminentSuccessor(const NodeSet &successors) const {
  double min_cost = std::numeric_limits<double>::max();
  Node minCost_node;
  for (const Node &node : successors) {
    if (node.idvCost < min_cost) {
      min_cost = node.idvCost;
      minCost_node = node;
    }
  }
  return minCost_node;
}

double OnlineLocalizer::computeAveragePathCost() const {
  double mean_cost = 0;
  bool source_reached = false;
  int elInPath = 0;
  Node pred = currentBestHyp_;

  while (!source_reached) {
    if (pred == kSourceNode) {
      source_reached = true;
      continue;
    }
    mean_cost += pred.idvCost;
    elInPath++;
    pred = pred_.at(pred.quId).at(pred.refId);
  }
  mean_cost = mean_cost / elInPath;
  return mean_cost;
}

bool OnlineLocalizer::predExists(const Node &node) const {
  auto quId_found = pred_.find(node.quId);
  if (quId_found == pred_.end()) {
    return false;
  }
  auto node_found = quId_found->second.find(node.refId);
  if (node_found == quId_found->second.end()) {
    return false;
  }
  return true;
}

void OnlineLocalizer::updateGraph(const Node &parent,
                                  const NodeSet &successors) {
  LOG_IF(WARNING, successors.empty()) << "No successors to add to the graph. "
                                         "May lead to disconnected components";

  // For every successor
  // check if the child was visited before (The child was visited if there
  // exists a predecessor for it)
  // if yes, check if the proposed accumulated cost is smaller than existing one
  // if no set a pred for a child.
  for (Node child : successors) {
    if (predExists(child)) {
      // child was visisted before
      double prev_accCost = accCosts_[child.quId][child.refId];
      double poss_accCost = child.idvCost + parent.accCost;
      if (poss_accCost < prev_accCost) {
        LOG(INFO) << "Possible accumulated cost is smaller than previous one. "
                     "Frontier should be updated";
        LOG(FATAL)
            << "[BINGO] If you have reached this line, please contact Olga :)";
        // update pred; update accu_costs + update frontier.
        // assign an alternative parent (the one that came in a function) to a
        // child
        pred_[child.quId][child.refId] = parent;
        accCosts_[child.quId][child.refId] = poss_accCost;
        // Update the accumulated cost for a child for estimating the priority
        child.accCost = poss_accCost;
        // Create a child with higher priority ( lower accCost)
        frontier_.push(child);
      } else {
        // Child was visited before, but new cost is smaller.
      }
    } else {
      // new successor
      pred_[child.quId][child.refId] = parent;
      accCosts_[child.quId][child.refId] = child.idvCost + parent.accCost;
      child.accCost = accCosts_[child.quId][child.refId];
      frontier_.push(child);
    }
  }
}

std::vector<PathElement> OnlineLocalizer::getCurrentPath() const {
  std::vector<PathElement> path;
  bool source_reached = false;
  Node pred = currentBestHyp_;

  while (!source_reached) {
    if (pred == kSourceNode) {
      source_reached = true;
      continue;
    }
    NodeState state = pred.idvCost > nonMatchingCost_ ? HIDDEN : REAL;
    PathElement pathEl(pred.quId, pred.refId, state);
    path.push_back(pathEl);
    pred = pred_.at(pred.quId).at(pred.refId);
  }
  return path;
}

// Check N last image matches. If `ratio` of them are hidden - LOST!
/**
 * @brief      Determines if lost.
 *
 * @param[in]  N  The number of path elements to check
 * @param[in]  ratio   [0,1] ratio of the hidden nodes to be

 * @return     True if lost, False otherwise.
 */
bool OnlineLocalizer::isLost(int N, double ratio) const {
  std::vector<PathElement> path = getLastNmatches(N);
  if (path.size() < N) {
    // The path is too short to make a decision.
    return false;
  }
  int lostFactor = 0;
  for (size_t i = 0; i < path.size(); ++i) {
    if (path[i].state == HIDDEN) {
      lostFactor++;
    }
  }
  if ((double)lostFactor / path.size() > ratio) {
    LOG(INFO) << "LOST localization";
    return true;
  }
  return false;
}

std::vector<PathElement> OnlineLocalizer::getLastNmatches(int N) const {
  std::vector<PathElement> path;
  bool source_reached = false;
  Node pred = currentBestHyp_;
  int counter = N;

  while (!source_reached && counter > 0) {
    if (pred == kSourceNode) {
      source_reached = true;
      continue;
    }
    NodeState state = pred.idvCost > nonMatchingCost_ ? HIDDEN : REAL;
    PathElement pathEl(pred.quId, pred.refId, state);
    path.push_back(pathEl);
    pred = pred_.at(pred.quId).at(pred.refId);
    counter--;
  }
  return path;
}

/**
 * @brief      sends path + frontier to the visualizer
 */
void OnlineLocalizer::visualize() const {
  if (!_vis) {
    return;
  }
  // _vis->drawFrontier(frontier_);
  _vis->drawExpansion(expandedRecently_);
  std::vector<PathElement> path = getCurrentPath();
  std::reverse(path.begin(), path.end());
  _vis->drawPath(path);
}

} // namespace online_localizer