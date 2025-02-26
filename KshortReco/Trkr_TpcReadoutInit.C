#ifndef MACRO_TPCREADOUTINIT_C
#define MACRO_TPCREADOUTINIT_C

R__LOAD_LIBRARY(libtpc.so)
R__LOAD_LIBRARY(libtrack_reco.so)
R__LOAD_LIBRARY(libtpccalib.so)

#include <GlobalVariables.C>

#include <G4_TrkrVariables.C>
#include <fun4all/Fun4AllServer.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundefined-internal"
#include <tpc/TpcClusterZCrossingCorrection.h>
#pragma GCC diagnostic pop

#include <cdbobjects/CDBTTree.h>
#include <ffamodules/CDBInterface.h>

void TpcSampleInit(const int RunNumber = 41989)
{
  if(RunNumber>=41624)
  {
    TRACKING::reco_tpc_maxtime_sample = 425;
    TRACKING::reco_tpc_time_presample = 40;//120 - 80
  }
  else
    {
      TRACKING::reco_tpc_maxtime_sample = 420;
      TRACKING::reco_tpc_time_presample = 0;// 80
    }
}

void TpcReadoutInit(const int RunNumber = 41989)
{

  TRACKING::reco_tpc_is_configured = true;
  if(RunNumber<45737)
  {
    // all Ar/CF4 runs
    G4TPC::tpc_drift_velocity_reco = (8.0 / 1000) * 107.0 / 105.0; // cm/ns
  } else if(RunNumber<49515) {
    // Ar/CF4/N2 runs
    G4TPC::tpc_drift_velocity_reco = 0.007; // cm/ns
  } else {
    // Ar/CF4/iC4H10
    /* default drift velocity from Xu-Dong's study */
    G4TPC::tpc_drift_velocity_reco = 0.00713; // cm/ns
  }

  if(RunNumber>=41624)
  {
    TRACKING::reco_tpc_maxtime_sample = 425;
    TRACKING::reco_tpc_time_presample = 40;//120 - 80
  }else{
    TRACKING::reco_tpc_maxtime_sample = 420;
    TRACKING::reco_tpc_time_presample = 0;// 80
  }

  std::string tpc_dv_calib_dir = CDBInterface::instance()->getUrl("TPC_DRIFT_VELOCITY");
  if (tpc_dv_calib_dir.empty())
  {
    std::cout << "No calibrated TPC drift velocity for Run " << RunNumber << ". Use default value " << G4TPC::tpc_drift_velocity_reco << " cm/ns" << std::endl;
  }
  else
  {
    CDBTTree *cdbttree = new CDBTTree(tpc_dv_calib_dir);
    cdbttree->LoadCalibrations();
    G4TPC::tpc_drift_velocity_reco = cdbttree->GetSingleFloatValue("tpc_drift_velocity");
    std::cout << "Use calibrated TPC drift velocity for Run " << RunNumber << ": " << G4TPC::tpc_drift_velocity_reco << " cm/ns" << std::endl;
  }
  float tpc_fee_clock_period_ratio = 53.326184/50.037280;//temporary fix before we re-calibrate drift velocity
  if(RunNumber==53285) G4TPC::tpc_drift_velocity_reco = 0.006959*tpc_fee_clock_period_ratio;
  if(RunNumber==53534) G4TPC::tpc_drift_velocity_reco = 0.007017*tpc_fee_clock_period_ratio;
  if(RunNumber==53744) G4TPC::tpc_drift_velocity_reco = 0.007010*tpc_fee_clock_period_ratio;
  if(RunNumber==53756) G4TPC::tpc_drift_velocity_reco = 0.007010*tpc_fee_clock_period_ratio;
  if(RunNumber==53877) G4TPC::tpc_drift_velocity_reco = 0.007011*tpc_fee_clock_period_ratio;
  if(RunNumber==53876) G4TPC::tpc_drift_velocity_reco = 0.007011*tpc_fee_clock_period_ratio;
  if(RunNumber==53630) G4TPC::tpc_drift_velocity_reco = 0.00733*tpc_fee_clock_period_ratio;//no updated
  std::cout << "Use RE-calibrated TPC drift velocity for Run " << RunNumber << ": " << G4TPC::tpc_drift_velocity_reco << " cm/ns" << std::endl;
  TpcClusterZCrossingCorrection::_vdrift = G4TPC::tpc_drift_velocity_reco;

  if(RunNumber==53285) TRACKING::reco_t0=-5;
  if(RunNumber==53534) TRACKING::reco_t0=-4;
  if(RunNumber==53744) TRACKING::reco_t0=-4;
  if(RunNumber==53756) TRACKING::reco_t0=-4;
  if(RunNumber==53877) TRACKING::reco_t0=-4;
  if(RunNumber==53876) TRACKING::reco_t0=-4;
  if(RunNumber==53630) TRACKING::reco_t0=+2;//no updated
  std::cout << "Use t0 for Run " << RunNumber << ": " << TRACKING::reco_t0 << std::endl;
}


#endif
