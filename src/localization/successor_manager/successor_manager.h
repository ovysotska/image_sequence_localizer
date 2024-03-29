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

#ifndef SRC_SUCCESSOR_MANAGER_SUCCESSOR_MANAGER_H_
#define SRC_SUCCESSOR_MANAGER_SUCCESSOR_MANAGER_H_

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "database/idatabase.h"
#include "relocalizers/irelocalizer.h"
#include "successor_manager/node.h"

namespace localization::successor_manager {
/**
 * @brief      Class that communicates between database and localizer.
 *             Knows how to maintain successor.
 */
class SuccessorManager {
public:
  SuccessorManager(database::iDatabase *database,
                   relocalizers::iRelocalizer *relocalizer, int fanOut);
  ~SuccessorManager() {}

  /**
   * @brief      Introduces the notion of similar places within the reference
   * trajectory. The ids of the similar places should be pre-computed.
   *
   * @param[in]  filename  The filename
   *
   * @return     { description_of_the_return_value }
   */
  bool setSimilarPlaces(const std::string &filename);

  std::unordered_set<Node> getSuccessors(const Node &node);
  std::unordered_set<Node> getSuccessorsIfLost(const Node &node);

  void getSuccessorFanOut(int quId, int refId);
  void getSuccessorsSimPlaces(int quId, int refId);

protected:
  database::iDatabase *database_ = nullptr;
  int fanOut_ = 0;

private:
  // current successors
  std::unordered_set<Node> _successors;
  /**
   * for refId gives the vector of refIds, that represent similar places
   */
  std::unordered_map<int, std::set<int>> _sameRefPlaces;
  relocalizers::iRelocalizer *relocalizer_ = nullptr;
};
} // namespace localization::successor_manager

#endif // SRC_SUCCESSOR_MANAGER_SUCCESSOR_MANAGER_H_
