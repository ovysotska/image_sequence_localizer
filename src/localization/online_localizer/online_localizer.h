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

#ifndef SRC_ONLINE_LOCALIZER_ONLINE_LOCALIZER_H_
#define SRC_ONLINE_LOCALIZER_ONLINE_LOCALIZER_H_

#include <memory>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "online_localizer/ilocvisualizer.h"
#include "online_localizer/path_element.h"
#include "successor_manager/node.h"
#include "successor_manager/successor_manager.h"

namespace localization::online_localizer {

// Performs online localization.
class OnlineLocalizer {
public:
  using PredMap = std::unordered_map<int, std::unordered_map<int, Node>>;
  using AccCostsMap = std::unordered_map<int, std::unordered_map<int, double>>;

  OnlineLocalizer(successor_manager::SuccessorManager *successorManager,
                  double expansionRate, double nonMatchingCost);
  ~OnlineLocalizer() {}

  Matches findMatchesTill(int queryId);
  void writeOutExpanded(const std::string &filename) const;

protected:
  void processImage(int quId);
  void matchImage(int quId);
  std::vector<PathElement> getCurrentPath() const;

  std::vector<PathElement> getLastNmatches(int N) const;
  void updateSearch(const NodeSet &successors);
  void updateGraph(const Node &parent, const NodeSet &successors);
  Node getProminentSuccessor(const NodeSet &successors) const;
  bool predExists(const Node &node) const;
  bool nodeWorthExpanding(const Node &node) const;
  double computeAveragePathCost() const;

  bool isLost(int N, double perc) const;

  void visualize() const;

private:
  int kSlidingWindowSize_ = 5; // frames
  bool needReloc_ = false;
  double expansionRate_ = -1.0;
  double nonMatchingCost_ = -1.0;

  std::priority_queue<Node> frontier_;
  // stores parent for each node
  PredMap pred_;
  // stores the accumulative  cost for each node
  AccCostsMap accCosts_;
  Node currentBestHyp_;

  successor_manager::SuccessorManager *successorManager_ = nullptr;
  iLocVisualizer::Ptr _vis = nullptr;

  NodeSet expandedRecently_;
};
} // namespace localization::online_localizer

#endif // SRC_ONLINE_LOCALIZER_ONLINE_LOCALIZER_H_