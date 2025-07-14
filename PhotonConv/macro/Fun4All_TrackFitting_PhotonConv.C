/*
 * This macro shows a working example of doing track-to-calo matching
 * 1. load track DST and calo DST
 * 2. do track projection onto calo surface
 * 3. do calo clustering again
 *    3.1 for EMCal, use RawClusterBuilderTemplate module with detailed geo
 *    3.2 for HCal, use RawClusterBuilderTopo
 * 3. run TrackCaloMatch module to match track and calo preliminarily
 *    and put matched tracks together into a new container
 * 4. run KFParticle using selected matched tracks, do conversion photon reconstruction
 *    create a new SvtxTrack container and KFParticle container
 * 5. run Track2Calo module to get track information from KFParticle
 *    and match it with calo again, save output into TTree for analysis
 */

#include <fun4all/Fun4AllUtils.h>
#include <G4_ActsGeom.C>
#include <G4_Global.C>
#include <G4_Magnet.C>
#include <G4_Mbd.C>
#include <GlobalVariables.C>
#include <QA.C>
#include <Calo_Calib.C>
#include <Trkr_Clustering.C>
#include <Trkr_Reco.C>
#include <Trkr_RecoInit.C>
#include <Trkr_TpcReadoutInit.C>

#include <ffamodules/CDBInterface.h>

#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllServer.h>

#include <phool/recoConsts.h>

#include <cdbobjects/CDBTTree.h>

#include <tpccalib/PHTpcResiduals.h>

#include <trackingqa/InttClusterQA.h>
#include <trackingqa/MicromegasClusterQA.h>
#include <trackingqa/MvtxClusterQA.h>
#include <trackingqa/TpcClusterQA.h>
#include <tpcqa/TpcRawHitQA.h>
#include <trackingqa/TpcSeedsQA.h>

#include <trackreco/PHActsTrackProjection.h>

#include <trackingdiagnostics/TrackResiduals.h>
#include <trackingdiagnostics/TrkrNtuplizer.h>
#include <trackingdiagnostics/KshortReconstruction.h>

#include <track_to_calo/TrackCaloMatch.h>
#include <track_to_calo/TrackToCalo.h>
#include <track_to_calo/CaloOnly.h>
#include <track_to_calo/TrackOnly.h>

#include <caloreco/CaloGeomMapping.h>
#include <caloreco/RawClusterBuilderTemplate.h>
#include <caloreco/RawClusterBuilderTopo.h>

#include <stdio.h>

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wundefined-internal"

#include <kfparticle_sphenix/KFParticle_sPHENIX.h>

#pragma GCC diagnostic pop

void KFPReco(std::string module_name = "KFPReco", std::string decaydescriptor = "K_S0 -> pi^+ pi^-", bool save_output = true, std::string outfile = "KFP.root", std::string trackmapName = "SvtxTrackMap", std::string containerName = "KFParticle");

R__LOAD_LIBRARY(libkfparticle_sphenix.so)
R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libphool.so)
R__LOAD_LIBRARY(libcdbobjects.so)
R__LOAD_LIBRARY(libmvtx.so)
R__LOAD_LIBRARY(libintt.so)
R__LOAD_LIBRARY(libtpc.so)
R__LOAD_LIBRARY(libmicromegas.so)
R__LOAD_LIBRARY(libTrackingDiagnostics.so)
R__LOAD_LIBRARY(libtrackingqa.so)
R__LOAD_LIBRARY(libtpcqa.so)
R__LOAD_LIBRARY(libtrack_to_calo.so)
R__LOAD_LIBRARY(libtrack_reco.so)
R__LOAD_LIBRARY(libcalo_reco.so)
R__LOAD_LIBRARY(libcalotrigger.so)
R__LOAD_LIBRARY(libcentrality.so)
R__LOAD_LIBRARY(libmbd.so)
R__LOAD_LIBRARY(libepd.so)
R__LOAD_LIBRARY(libzdcinfo.so)
void Fun4All_TrackFitting_PhotonConv(
    const int nEvents = 10,
    const std::string trkr_trackfilename = "DST_TRKR_TRACKS_run2pp_ana494_2024p021_v001-00053877-00000.root",
    const std::string trkr_trackdir = "/sphenix/lustre01/sphnxpro/production/run2pp/physics/ana494_2024p021_v001/DST_TRKR_TRACKS/run_00053800_00053900/dst/",
    const std::string trkr_clusterfilename = "DST_TRKR_CLUSTER_run2pp_ana494_2024p021_v001-00053877-00000.root",
    const std::string trkr_clusterdir = "/sphenix/lustre01/sphnxpro/production/run2pp/physics/ana494_2024p021_v001/DST_TRKR_CLUSTER/run_00053800_00053900/dst/",
    const std::string calofilename = "DST_CALO_run2pp_ana468_2024p012_v001-00053877-00000.root",
    const std::string calodir = "/sphenix/lustre01/sphnxpro/production/run2pp/physics/ana468_2024p012_v001/DST_CALO/run_00053800_00053900/dst/",
    const std::string outfilename = "Track2Calo",
    const std::string outdir = "./root",
    const int index = 0,
    const int stepsize = 10)
{
  std::string inputTrkrTrackFile = trkr_trackdir + trkr_trackfilename;
  std::string inputTrkrClusterFile = trkr_clusterdir + trkr_clusterfilename;
  std::string inputCaloFile = calodir + calofilename;

  std::pair<int, int>
      runseg = Fun4AllUtils::GetRunSegment(trkr_trackfilename);
  int runnumber = runseg.first;
  int segment = runseg.second;

  std::cout << " run: " << runnumber
            << " samples: " << TRACKING::reco_tpc_maxtime_sample
            << " pre: " << TRACKING::reco_tpc_time_presample
            << " vdrift: " << G4TPC::tpc_drift_velocity_reco
            << std::endl;

  // distortion calibration mode
  /*
   * set to true to enable residuals in the TPC with
   * TPC clusters not participating to the ACTS track fit
   */
  G4TRACKING::SC_CALIBMODE = false;
  Enable::MVTX_APPLYMISALIGNMENT = true;
  ACTSGEOM::mvtx_applymisalignment = Enable::MVTX_APPLYMISALIGNMENT;
  TRACKING::pp_mode = true;

  string outDir = outdir + "/inReconstruction/" + to_string(runnumber) + "/";
  string makeDirectory = "mkdir -p " + outDir;
  system(makeDirectory.c_str());
  TString outfile = outDir + outfilename + "_" + runnumber + "-" + segment + "-" + index + ".root";
  std::cout<<"outfile "<<outfile<<std::endl;
  std::string theOutfile = outfile.Data();

  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", runnumber);
  rc->set_IntFlag("RUNSEGMENT", segment);

  Enable::CDB = true;
  rc->set_StringFlag("CDB_GLOBALTAG", "newcdbtag");
  rc->set_uint64Flag("TIMESTAMP", runnumber);
  std::string geofile = CDBInterface::instance()->getUrl("Tracking_Geometry");

  Fun4AllRunNodeInputManager *ingeo = new Fun4AllRunNodeInputManager("GeoIn");
  ingeo->AddFile(geofile);
  se->registerInputManager(ingeo);

  TpcReadoutInit( runnumber );
  // these lines show how to override the drift velocity and time offset values set in TpcReadoutInit
  // G4TPC::tpc_drift_velocity_reco = 0.0073844; // cm/ns
  // TpcClusterZCrossingCorrection::_vdrift = G4TPC::tpc_drift_velocity_reco;
  // G4TPC::tpc_tzero_reco = -5*50;  // ns

  G4TPC::ENABLE_MODULE_EDGE_CORRECTIONS = true;

  // to turn on the default static corrections, enable the two lines below
  G4TPC::ENABLE_STATIC_CORRECTIONS = true;
  G4TPC::USE_PHI_AS_RAD_STATIC_CORRECTIONS = false;

  //to turn on the average corrections, enable the three lines below
  //note: these are designed to be used only if static corrections are also applied
  G4TPC::ENABLE_AVERAGE_CORRECTIONS = true;
  G4TPC::USE_PHI_AS_RAD_AVERAGE_CORRECTIONS = false;
   // to use a custom file instead of the database file:
  G4TPC::average_correction_filename = CDBInterface::instance()->getUrl("TPC_LAMINATION_FIT_CORRECTION");
  G4MAGNET::magfield_rescale = 1;
  TrackingInit();

  auto hitsin_track = new Fun4AllDstInputManager("DSTin_track");
  hitsin_track->fileopen(inputTrkrTrackFile);
  se->registerInputManager(hitsin_track);

  auto hitsin_cluster = new Fun4AllDstInputManager("DSTin_cluster");
  hitsin_cluster->fileopen(inputTrkrClusterFile);
  se->registerInputManager(hitsin_cluster);

  auto hitsin_calo = new Fun4AllDstInputManager("DSTin_calo");
  hitsin_calo->fileopen(inputCaloFile);
  se->registerInputManager(hitsin_calo);

  Global_Reco();

  bool doEMcalRadiusCorr = true;
  auto projection = new PHActsTrackProjection("CaloProjection");
  float new_cemc_rad = 102.87;
  if (doEMcalRadiusCorr)
  {
    projection->setLayerRadius(SvtxTrack::CEMC, new_cemc_rad);
  }
  se->registerSubsystem(projection);

  //my calo reco
  std::cout<<"Begin my calo reco"<<std::endl;

  CaloGeomMapping *cgm = new CaloGeomMapping();
  cgm->set_detector_name("CEMC");
  cgm->set_UseDetailedGeometry(true);
  se->registerSubsystem(cgm);

  //////////////////
  // Clusters
  std::cout << "Building clusters" << std::endl;
  RawClusterBuilderTemplate *ClusterBuilder = new RawClusterBuilderTemplate("EmcRawClusterBuilderTemplate");
  ClusterBuilder->Detector("CEMC");
  ClusterBuilder->set_UseDetailedGeometry(true);
  ClusterBuilder->set_threshold_energy(0.070);  // for when using basic calibration
  std::string emc_prof = getenv("CALIBRATIONROOT");
  emc_prof += "/EmcProfile/CEMCprof_Thresh30MeV.root";
  //ClusterBuilder->set_UseAltZVertex(3); //0: GlobalVertexMap, 1: MbdVertexMap, 2: Nothing, 3: G4TruthInfo
  ClusterBuilder->LoadProfile(emc_prof);
  ClusterBuilder->set_UseTowerInfo(1);  // to use towerinfo objects rather than old RawTower
  se->registerSubsystem(ClusterBuilder);

  //For particle flow studies
  RawClusterBuilderTopo* ClusterBuilder2 = new RawClusterBuilderTopo("EMcalRawClusterBuilderTopo2");
  ClusterBuilder2->Verbosity(0);
  ClusterBuilder2->set_nodename("TOPOCLUSTER_HCAL");
  ClusterBuilder2->set_enable_HCal(true);
  ClusterBuilder2->set_enable_EMCal(false);
  //ClusterBuilder2->set_noise(0.0025, 0.006, 0.03);
  ClusterBuilder2->set_noise(0.01, 0.03, 0.03);
  ClusterBuilder2->set_significance(4.0, 2.0, 1.0);
  ClusterBuilder2->allow_corner_neighbor(true);
  ClusterBuilder2->set_do_split(true);
  ClusterBuilder2->set_minE_local_max(1.0, 2.0, 0.5);
  ClusterBuilder2->set_R_shower(0.025);
  se->registerSubsystem(ClusterBuilder2);

  TrackCaloMatch *tcm = new TrackCaloMatch("Tracks_Calo_Match");
  tcm->SetMyTrackMapName("MySvtxTrackMap");
  tcm->writeEventDisplays(false);
  tcm->EMcalRadiusUser(doEMcalRadiusCorr);
  tcm->setEMcalRadius(new_cemc_rad);
  tcm->setdphicut(0.15);
  tcm->setdzcut(10);
  tcm->setTrackPtLowCut(0.2);
  tcm->setEmcalELowCut(0.1);
  tcm->setnTpcClusters(20);
  tcm->setTrackQuality(1000);
  tcm->setRawClusContEMName("CLUSTERINFO_CEMC");
  tcm->setRawTowerGeomContName("TOWERGEOM_CEMC_DETAILED");
  se->registerSubsystem(tcm);

  TString photonconv_kfp_likesign_outfile = theOutfile + "_photonconv_kfp_likesign.root";
  std::string photonconv_kfp_likesign_string(photonconv_kfp_likesign_outfile.Data());
  KFPReco("PhotonConvKFPReco_likesign", "[gamma -> e^+ e^+]cc", false, photonconv_kfp_likesign_string, "MySvtxTrackMap", "PhotonConv_likesign");

  TString photonconv_kfp_unlikesign_outfile = theOutfile + "_photonconv_kfp_unlikesign.root";
  std::string photonconv_kfp_unlikesign_string(photonconv_kfp_unlikesign_outfile.Data());
  KFPReco("PhotonConvKFPReco_unlikesign", "gamma -> e^+ e^-", false, photonconv_kfp_unlikesign_string, "MySvtxTrackMap", "PhotonConv_unlikesign");

  TString track2calo_unlikesign_outfile = theOutfile + "_track2calo_unlikesign.root";
  std::string track2calo_unlikesign_string(track2calo_unlikesign_outfile.Data());
  TrackToCalo *ttc_unlikesign = new TrackToCalo("Tracks_And_Calo", track2calo_unlikesign_string);
  ttc_unlikesign->Verbosity(0);
  ttc_unlikesign->EMcalRadiusUser(doEMcalRadiusCorr);
  ttc_unlikesign->setEMcalRadius(new_cemc_rad);
  ttc_unlikesign->doLikesign(false);
  ttc_unlikesign->setKFPtrackMapName("PhotonConv_unlikesign_SvtxTrackMap");
  ttc_unlikesign->setKFPContName("PhotonConv_unlikesign_KFParticle_Container");
  ttc_unlikesign->anaTrkrInfo(false); // general track QA
  ttc_unlikesign->anaCaloInfo(false); // general calo QA
  ttc_unlikesign->doTrkrCaloMatching(false); // SvtxTrack match with calo
  ttc_unlikesign->doTrkrCaloMatching_KFP(true); // KFP selected trck match with calo
  ttc_unlikesign->setTrackPtLowCut(0.2);
  ttc_unlikesign->setEmcalELowCut(0.1);
  ttc_unlikesign->setnTpcClusters(20);
  ttc_unlikesign->setTrackQuality(1000);
  ttc_unlikesign->setRawClusContEMName("CLUSTERINFO_CEMC");
  ttc_unlikesign->setRawTowerGeomContName("TOWERGEOM_CEMC_DETAILED");
  se->registerSubsystem(ttc_unlikesign); 

  TString track2calo_likesign_outfile = theOutfile + "_track2calo_likesign.root";
  std::string track2calo_likesign_string(track2calo_likesign_outfile.Data());
  TrackToCalo *ttc_likesign = new TrackToCalo("Tracks_And_Calo", track2calo_likesign_string);
  ttc_likesign->Verbosity(0);
  ttc_likesign->EMcalRadiusUser(doEMcalRadiusCorr);
  ttc_likesign->setEMcalRadius(new_cemc_rad);
  ttc_likesign->doLikesign(true);
  ttc_likesign->setKFPtrackMapName("PhotonConv_likesign_SvtxTrackMap");
  ttc_likesign->setKFPContName("PhotonConv_likesign_KFParticle_Container");
  ttc_likesign->anaTrkrInfo(false); // general track QA
  ttc_likesign->anaCaloInfo(false); // general calo QA
  ttc_likesign->doTrkrCaloMatching(false); // SvtxTrack match with calo
  ttc_likesign->doTrkrCaloMatching_KFP(true); // KFP selected trck match with calo
  ttc_likesign->setTrackPtLowCut(0.2);
  ttc_likesign->setEmcalELowCut(0.1);
  ttc_likesign->setnTpcClusters(20);
  ttc_likesign->setTrackQuality(1000);
  ttc_likesign->setRawClusContEMName("CLUSTERINFO_CEMC");
  ttc_likesign->setRawTowerGeomContName("TOWERGEOM_CEMC_DETAILED");
  se->registerSubsystem(ttc_likesign); 

  se->skip(stepsize*index);
  se->run(nEvents);
  se->End();
  se->PrintTimer();
  std::cout << "CDB Files used:" << std::endl;
  CDBInterface::instance()->Print();

  ifstream file_photonconv_kfp_likesign(photonconv_kfp_likesign_string.c_str(), ios::binary | ios::ate);
  if (file_photonconv_kfp_likesign.good() && (file_photonconv_kfp_likesign.tellg() > 100))
  {
    string outputDirMove = outdir + "/Reconstructed/" + to_string(runnumber) + "/";
    string makeDirectoryMove = "mkdir -p " + outputDirMove;
    system(makeDirectoryMove.c_str());
    string moveOutput = "mv " + photonconv_kfp_likesign_string + " " + outputDirMove;
    std::cout << "moveOutput: " << moveOutput << std::endl;
    system(moveOutput.c_str());
  }

  ifstream file_photonconv_kfp_unlikesign(photonconv_kfp_unlikesign_string.c_str(), ios::binary | ios::ate);
  if (file_photonconv_kfp_unlikesign.good() && (file_photonconv_kfp_unlikesign.tellg() > 100))
  {
    string outputDirMove = outdir + "/Reconstructed/" + to_string(runnumber) + "/";
    string makeDirectoryMove = "mkdir -p " + outputDirMove;
    system(makeDirectoryMove.c_str());
    string moveOutput = "mv " + photonconv_kfp_unlikesign_string + " " + outputDirMove;
    std::cout << "moveOutput: " << moveOutput << std::endl;
    system(moveOutput.c_str());
  }

  ifstream file_track2calo_unlikesign(track2calo_unlikesign_string.c_str(), ios::binary | ios::ate);
  if (file_track2calo_unlikesign.good() && (file_track2calo_unlikesign.tellg() > 100))
  {
    string outputDirMove = outdir + "/Reconstructed/" + to_string(runnumber) + "/";
    string makeDirectoryMove = "mkdir -p " + outputDirMove;
    system(makeDirectoryMove.c_str());
    string moveOutput = "mv " + track2calo_unlikesign_string + " " + outputDirMove;
    std::cout << "moveOutput: " << moveOutput << std::endl;
    system(moveOutput.c_str());
  }

  ifstream file_track2calo_likesign(track2calo_likesign_string.c_str(), ios::binary | ios::ate);
  if (file_track2calo_likesign.good() && (file_track2calo_likesign.tellg() > 100))
  {
    string outputDirMove = outdir + "/Reconstructed/" + to_string(runnumber) + "/";
    string makeDirectoryMove = "mkdir -p " + outputDirMove;
    system(makeDirectoryMove.c_str());
    string moveOutput = "mv " + track2calo_likesign_string + " " + outputDirMove;
    std::cout << "moveOutput: " << moveOutput << std::endl;
    system(moveOutput.c_str());
  }

  delete se;
  std::cout << "Finished" << std::endl;
  gSystem->Exit(0);
}

void KFPReco(std::string module_name = "KFPReco", std::string decaydescriptor = "K_S0 -> pi^+ pi^-", bool save_output = true, std::string outfile = "KFP.root", std::string trackmapName = "SvtxTrackMap", std::string containerName = "KFParticle")
{
  auto se = Fun4AllServer::instance();
  //KFParticle setup
  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(module_name);
  kfparticle->Verbosity(0);
  kfparticle->setDecayDescriptor(decaydescriptor);

  kfparticle->setTrackMapNodeName(trackmapName);
  kfparticle->setContainerName(containerName);

  //Basic node selection and configuration
  kfparticle->magFieldFile("FIELDMAP_TRACKING");
  kfparticle->getAllPVInfo(false);
  kfparticle->allowZeroMassTracks(true);
  kfparticle->getDetectorInfo(true);
  kfparticle->useFakePrimaryVertex(false);
  kfparticle->saveDST();

  kfparticle->constrainToPrimaryVertex(true);
  kfparticle->setMotherIPchi2(FLT_MAX);
  kfparticle->setFlightDistancechi2(-1.);
  kfparticle->setMinDIRA(-1.1);
  kfparticle->setDecayLengthRange(0., FLT_MAX);
  kfparticle->setDecayTimeRange(-1*FLT_MAX, FLT_MAX);

  //Track parameters
  kfparticle->setMinMVTXhits(0);
  kfparticle->setMinINTThits(0);
  kfparticle->setMinTPChits(20);
  kfparticle->setMinimumTrackPT(0.2);
  kfparticle->setMaximumTrackPTchi2(FLT_MAX);
  kfparticle->setMinimumTrackIPchi2(-1.);
  kfparticle->setMinimumTrackIP(-1.);
  kfparticle->setMaximumTrackchi2nDOF(100.);

  //Vertex parameters
  //kfparticle->setMaximumVertexchi2nDOF(50);
  kfparticle->setMaximumVertexchi2nDOF(FLT_MAX);
  //kfparticle->setMaximumDaughterDCA(1.);
  kfparticle->setMaximumDaughterDCA(FLT_MAX);

  //Parent parameters
  kfparticle->setMotherPT(0);
  kfparticle->setMinimumMass(-1);
  kfparticle->setMaximumMass(10);
  //kfparticle->setMaximumMotherVertexVolume(0.1);
  kfparticle->setMaximumMotherVertexVolume(FLT_MAX);

  kfparticle->saveOutput(save_output);
  kfparticle->setOutputName(outfile);

  se->registerSubsystem(kfparticle);
}
