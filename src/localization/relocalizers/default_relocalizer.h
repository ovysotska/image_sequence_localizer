/* By O. Vysotska in 2023 */

#ifndef SRC_RELOCALIZERS_DEFAULT_RELOCALIZER_H_
#define SRC_RELOCALIZERS_DEFAULT_RELOCALIZER_H_
#include "relocalizers/irelocalizer.h"
#include <memory>
#include <vector>

/**
 * Gives the candidates based on Fanout.
 */
class DefaultRelocalizer : public iRelocalizer {
public:
  DefaultRelocalizer(int fanOut, int refSize);
  std::vector<int> getCandidates(int quId) override;

private:
  int fanOut_ = 3;
  int refSize_ = -1;
};

#endif // SRC_RELOCALIZERS_IRELOCALIZER_H_