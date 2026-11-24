#ifndef QA_TPCCALIBRATIONQA_CONDITIONSREFERENCE_H
#define QA_TPCCALIBRATIONQA_CONDITIONSREFERENCE_H

#include <string>

class TpcConditionsReference
{
 public:
  void set_cdb_tag(const std::string &value) { m_cdbTag = value; }
  void set_reference_field(double value) { m_referenceField = value; }
  void set_reference_pressure(double value) { m_referencePressure = value; }
  void set_reference_temperature(double value) { m_referenceTemperatureK = value; }
  void set_t0_bins(double value) { m_t0Bins = value; }
  void set_time_bin_ns(double value) { m_timeBinNs = value; }
  void set_step_ns(double value) { m_stepNs = value; }

  // Initializes run nodes once in a dedicated ROOT process; leaves the server
  // alive until process exit to preserve CDBInterface singleton lifetime.
  // Run in a separate ROOT process from any active event reconstruction.
  void export_phgarfield(int run, const std::string &group, const std::string &output) const;

 private:
  std::string m_cdbTag;
  double m_referenceField{376.85};
  double m_referencePressure{30.0968906};
  double m_referenceTemperatureK{299.98646566};
  double m_t0Bins{8.};
  double m_timeBinNs{56.8};
  double m_stepNs{5.};
};
#endif
