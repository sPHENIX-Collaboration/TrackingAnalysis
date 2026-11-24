#include <cmath>
#include <limits>

#include "../TpcConditionsUtils.h"
int main()
{
  using namespace TpcConditionsDetail;
  if (median({1., 3., 2.}) != 2. || median({1., 3.}) != 2.)
  {
    return 1;
  }
  if (std::isfinite(median({1., std::numeric_limits<double>::quiet_NaN()})))
  {
    return 2;
  }
  const auto result = summarize({1., std::numeric_limits<double>::quiet_NaN(), 3.});
  if (result.n != 2 || result.mean != 2.)
  {
    return 3;
  }
  if (r1Channels(0).size() != 4 || r1Channels(1).size() != 8)
  {
    return 4;
  }
  return 0;
}
