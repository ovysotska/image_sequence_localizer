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

#include "online_localizer/path_element.h"
#include "localization_protos.pb.h"

#include <glog/logging.h>

#include <fstream>
#include <string>

namespace localization::online_localizer {

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

void PathElement::print() const {
  std::string status;
  switch (state) {
  case HIDDEN: {
    status = "hidden";
    break;
  }
  case REAL: {
    status = "real";
    break;
  }
  }
  printf("[PathElement] %d %d : %s\n", quId, refId, status.c_str());
}

}; // namespace localization::online_localizer