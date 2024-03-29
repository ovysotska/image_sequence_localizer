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

#include "feature_buffer.h"
#include "features/ifeature.h"

#include <iostream>
#include <limits>

#include <glog/logging.h>

namespace localization::features {

FeatureBuffer::FeatureBuffer(int size) {
  LOG_IF(FATAL, size < 0) << "Invalid featureBuffer size.";
  bufferSize = size;
  featureMap.reserve(size);
}

bool FeatureBuffer::inBuffer(int id) const { return featureMap.count(id) > 0; }

const iFeature &FeatureBuffer::getFeature(int id) const {
  return *featureMap.at(id);
}

/** internal function. deletes the first added feature from the buffer **/
void FeatureBuffer::deleteFeature() {
  featureMap.erase(ids[0]);
  ids.erase(ids.begin());
}

void FeatureBuffer::addFeature(int id, std::unique_ptr<iFeature> &&feature) {

  if (static_cast<int>(ids.size()) == bufferSize) {
    deleteFeature();
  }
  ids.push_back(id);
  LOG_IF(WARNING, featureMap.count(id) > 0)
      << "Feature with id " << id << " exists. Overwriting..";
  featureMap.emplace(id, std::move(feature));
}

} // namespace localization::features
