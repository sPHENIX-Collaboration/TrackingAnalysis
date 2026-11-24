#ifndef TPC_CALIBRATION_QA_CONDITIONS_UTILS_H
#define TPC_CALIBRATION_QA_CONDITIONS_UTILS_H
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>
#include <string>
#include <vector>
namespace TpcConditionsDetail
{
inline const std::vector<std::string> &r1Channels(int side)
{
  static const std::vector<std::string> south = {"S_01_R1_G4_IMon", "S_04_R1_G4_IMon",
                                                 "S_05_R1_G4_IMon", "S_10_R1_G4_IMon"};
  static const std::vector<std::string> north = {
      "N_03_R1_G4_IMon", "N_04_R1_G4_IMon", "N_06_R1_G4_IMon", "N_07_R1_G4_IMon",
      "N_08_R1_G4_IMon", "N_10_R1_G4_IMon", "N_11_R1_G4_IMon", "N_12_R1_G4_IMon"};
  return side == 0 ? south : north;
}
inline double median(std::vector<double> values)
{
  if (values.empty() ||
      !std::all_of(values.begin(), values.end(), [](double v) { return std::isfinite(v); }))
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  std::sort(values.begin(), values.end());
  const auto size = values.size();
  return size % 2 ? values[size / 2] : .5 * (values[size / 2 - 1] + values[size / 2]);
}
struct Stats
{
  std::size_t n{0};
  double mean{std::numeric_limits<double>::quiet_NaN()};
};
inline Stats summarize(const std::vector<double> &values)
{
  std::vector<double> valid;
  for (double value : values)
  {
    if (std::isfinite(value))
    {
      valid.push_back(value);
    }
  }
  return {valid.size(), valid.empty() ? std::numeric_limits<double>::quiet_NaN()
                                      : std::accumulate(valid.begin(), valid.end(), 0.) /
                                            static_cast<double>(valid.size())};
}
}  // namespace TpcConditionsDetail
#endif
