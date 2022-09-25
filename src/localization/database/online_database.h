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

/* Updated by O. Vysotska in 2022 */

#ifndef SRC_DATABASE_ONLINE_DATABASE_H_
#define SRC_DATABASE_ONLINE_DATABASE_H_

#include "database/idatabase.h"
#include "features/feature_buffer.h"
#include "features/feature_factory.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief      Database for loading and matching features. Saves the computed
 * matching costs.
 */
class OnlineDatabase : public iDatabase {
public:
  OnlineDatabase(const std::string &queryFeaturesDir,
                 const std::string &refFeaturesDir, const FeatureType &type,
                 int bufferSize);

  int refSize() override { return refFeaturesNames_.size(); }
  double getCost(int quId, int refId) override;

  double computeMatchingCost(int quId, int refId);

  std::string getQuFeatureName(int id) const;
  std::string getRefFeatureName(int id) const;
  iFeature::ConstPtr getQueryFeature(int quId);

protected:
  std::vector<std::string> quFeaturesNames_, refFeaturesNames_;
  // TODO(olga): Maybe temporary here.
  FeatureType featureType_;

private:
  std::unique_ptr<FeatureBuffer> refBuffer_, queryBuffer_;
  std::unordered_map<int, std::unordered_map<int, double>> costs_;
};

#endif // SRC_DATABASE_ONLINE_DATABASE_H_
