#include "TpcConditionsReference.h"

#include <ffamodules/CDBInterface.h>
#include <ffamodules/FlagHandler.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <phool/recoConsts.h>

#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>

#include "TpcPHGarfieldReference.h"
#include "TpcRunAverageConditions.h"

void TpcConditionsReference::export_phgarfield(int run, const std::string &groupName,
                                               const std::string &outputName) const

{
  if (m_cdbTag.empty())
  {
    throw std::runtime_error("An explicit CDB tag is required");
  }
  const char *group = groupName.c_str();
  const char *csv = outputName.c_str();
  const char *cdbTag = m_cdbTag.c_str();
  const double referenceField = m_referenceField;
  const double referencePressure = m_referencePressure;
  const double referenceTemperatureK = m_referenceTemperatureK;
  const double t0Bins = m_t0Bins;
  const double timeBinNs = m_timeBinNs;
  const double stepNs = m_stepNs;

  if (run <= 0 || !std::isfinite(referenceField) || referenceField <= 0 ||
      !std::isfinite(referencePressure) || referencePressure <= 0 ||
      !std::isfinite(referenceTemperatureK) || referenceTemperatureK <= 0 ||
      !std::isfinite(t0Bins) || !std::isfinite(timeBinNs) || timeBinNs <= 0 ||
      !std::isfinite(stepNs) || stepNs <= 0)
  {
    throw std::runtime_error("Invalid PHGarfield reference settings");
  }
  // The singleton owns its subsystems, including CDBInterface. Deleting it here
  // would leave CDBInterface's singleton pointer dangling in current releases.
  // This exporter must run once in a dedicated ROOT process.
  static bool exportStarted = false;
  if (exportStarted)
  {
    throw std::runtime_error("PHGarfield export requires a fresh ROOT process");
  }
  exportStarted = true;
  auto *se = Fun4AllServer::instance();
  auto *rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", run);
  rc->set_uint64Flag("TIMESTAMP", run);
  rc->set_StringFlag("CDB_GLOBALTAG", cdbTag);
  se->registerSubsystem(new FlagHandler());
  const std::string geometry = CDBInterface::instance()->getUrl("Tracking_Geometry");
  if (geometry.empty())
  {
    throw std::runtime_error("Tracking_Geometry CDB lookup failed");
  }
  auto *geo = new Fun4AllRunNodeInputManager("TPCPHGGeometry");
  geo->AddFile(geometry);
  se->registerInputManager(geo);
  auto *dummy = new Fun4AllDummyInputManager("TPCPHGConditionsEvent");
  dummy->SetRunNumber(run);
  se->registerInputManager(dummy);
  se->registerSubsystem(new TpcRunAverageConditions());
  const TpcPHGarfieldReference::Settings settings{
      referenceField, referencePressure, referenceTemperatureK, t0Bins, timeBinNs, stepNs};
  auto *qa = new TpcPHGarfieldReference(settings);
  se->registerSubsystem(qa);
  if (se->run(1) != 0)
  {
    throw std::runtime_error("PHGarfield reference InitRun failed");
  }
  const auto velocity = qa->velocity();
  for (double value : velocity)
  {
    if (!std::isfinite(value) || value <= 0)
    {
      throw std::runtime_error("No valid PHGarfield mean velocity");
    }
  }
  const auto quoted = [](const std::string &text)
  {
    std::string out = "\"";
    for (char c : text)
    {
      if (c == '\"')
      {
        out += '\"';
      }
      out += c;
    }
    return out + '\"';
  };
  std::ofstream out(csv);
  if (!out)
  {
    throw std::runtime_error("Cannot write reference CSV");
  }
  out << std::setprecision(14);
  out << "group,run,side,velocity_cm_us,t0_bins,source,cdb_tag,reference_field_"
         "Vcm,effective_field_Vcm,pressure,temperature_K,time_bin_ns,step_ns,z_"
         "pad_cm,cm_halfwidth_cm,geometry,reference_pressure,reference_"
         "temperature_K\n";
  for (int side = 0; side < 2; ++side)
  {
    out << quoted(group) << ',' << run << ',' << side << ',' << velocity[side] << ',' << t0Bins
        << ",\"PHGarfield full-run-average P/T snapshot equal-FEE baseline; CDB drift "
           "corrections disabled\","
        << quoted(cdbTag) << ',' << referenceField << ',' << qa->effectiveField() << ','
        << qa->pressure() << ',' << qa->temperatureK() << ',' << timeBinNs << ',' << stepNs << ','
        << qa->padPlane() << ',' << qa->cmHalf() << ',' << quoted(geometry) << ','
        << referencePressure << ',' << referenceTemperatureK << '\n';
  }
  out.close();
  if (!out)
  {
    throw std::runtime_error("Failed to finish PHGarfield reference CSV");
  }
  se->End();
}
