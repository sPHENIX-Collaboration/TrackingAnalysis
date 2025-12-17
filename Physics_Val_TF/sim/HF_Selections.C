#include <fun4all/Fun4AllServer.h>

#include <calotrigger/TriggerRunInfoReco.h>
#include <globalvertex/GlobalVertexReco.h>

#include <kfparticleqa/QAKFParticle.h>
#include <kfparticleqa/QAKFParticleTrackPtAsymmetry.h>

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wundefined-internal"

#include <kfparticle_sphenix/KFParticle_sPHENIX.h>

#pragma GCC diagnostic pop

//#include <dstarreco/DstarReco.h>

R__LOAD_LIBRARY(libkfparticle_sphenix.so)
R__LOAD_LIBRARY(libcalotrigger.so)
//R__LOAD_LIBRARY(libDstarReco.so)
R__LOAD_LIBRARY(libkfparticleqa.so)

namespace HeavyFlavorReco
{
  int VERBOSITY_HF = 0;

  std::string output_dir = "/sphenix/tg/tg01/hf/hjheng/HF-analysis/"; //Top dir of where the output nTuples will be written
  std::string kfp_header = "outputKFParticle_";
  std::string processing_folder = "inReconstruction/";
  std::string trailer = ".root";

  // https://wiki.bnl.gov/sPHENIX/index.php/KFParticle
  std::string pipi_decay_descriptor = "K_S0 -> pi^+ pi^-"; //See twiki on how to set this
  std::string pipi_reconstruction_name = "pipi_reco"; //Used for naming output folder, file and node
  std::string pipi_output_reco_file;
  std::string pipi_output_dir;

  std::string Kpi_decay_descriptor = "[D0 -> K^- pi^+]cc"; //See twiki on how to set this
  std::string Kpi_reconstruction_name = "Kpi_reco"; //Used for naming output folder, file and node
  std::string Kpi_output_reco_file;
  std::string Kpi_output_dir;

  std::string Kpipi_decay_descriptor = "[D+ -> K^- pi^+ pi^+]cc"; //See twiki on how to set this
  std::string Kpipi_reconstruction_name = "Kpipi_reco"; //Used for naming output folder, file and node
  std::string Kpipi_output_reco_file;
  std::string Kpipi_output_dir;

  std::string KKpi_decay_descriptor = "[D_s+ -> {phi -> K^+ K^-} pi^+]cc"; //See twiki on how to set this
  //std::string KKpi_decay_descriptor = "[D_s+ -> K^+ K^- pi^+]cc"; //See twiki on how to set this
  std::string KKpi_reconstruction_name = "KKpi_reco"; //Used for naming output folder, file and node
  std::string KKpi_output_reco_file;
  std::string KKpi_output_dir;

  std::string pKpi_decay_descriptor = "[Lambda_c+ -> proton^+ K^- pi^+]cc"; //See twiki on how to set this
  std::string pKpi_reconstruction_name = "pKpi_reco"; //Used for naming output folder, file and node
  std::string pKpi_output_reco_file;
  std::string pKpi_output_dir;
  
  std::string mb_decay_descriptor = "K_S0 -> pi^+ pi^-"; //See twiki on how to set this
  std::string mb_reconstruction_name = "mb_reco"; //Used for naming output folder, file and node
  float mb_mass_range[2] = {0.4,0.6};
  //std::string mb_decay_descriptor = "[D0 -> K^- pi^+]cc"; //See twiki on how to set this
  //std::string mb_reconstruction_name = "mb_reco_D0"; //Used for naming output folder, file and node
  //float mb_mass_range[2] = {1.75, 1.95};
  //std::string mb_decay_descriptor = "[D+ -> K^- pi^+ pi^+]cc"; //See twiki on how to set this
  //std::string mb_reconstruction_name = "mb_reco_Dplus"; //Used for naming output folder, file and node
  //float mb_mass_range[2] = {1.5,2.2};
  //std::string mb_decay_descriptor = "[D_s+ -> {phi -> K^+ K^-} pi^+]cc"; //See twiki on how to set this
  //std::string mb_reconstruction_name = "mb_reco_Ds"; //Used for naming output folder, file and node
  // float mb_mass_range[2] = {1.5,2.2};
  //std::string mb_decay_descriptor = "[Lambda_c+ -> proton^+ K^- pi^+]cc"; //See twiki on how to set this
  //std::string mb_reconstruction_name = "mb_reco_LambdaC"; //Used for naming output folder, file and node
  //float mb_mass_range[2] = {2.1,2.6};
  std::string mb_output_reco_file;
  std::string mb_output_dir;
  
  bool use_pid = false;
  bool save_tracks_to_DST = true;
  bool dont_use_global_vertex = true;
  bool require_track_and_vertex_match = true;
  bool save_all_vtx_info = true;
  bool constrain_phi_mass = true;
  bool use_2D_matching = true;
  bool get_trigger_info = false; // false for simulation
  bool get_detector_info = true;
  bool get_dEdx_info = true;
  float pid_frac = 0.4;
  bool constrain_lambda_mass = true;
};  // namespace HeavyFlavorReco'

using namespace HeavyFlavorReco;

void init_kfp_dependencies()
{
  //dE/dx needs TRKR_CLUSTER and CYLINDERCELLGEOM_SVTX which need to be in the DST or loaded from a geo file
  Fun4AllServer *se = Fun4AllServer::instance();

  GlobalVertexReco* gblvertex = new GlobalVertexReco();
  gblvertex->Verbosity(VERBOSITY_HF);
  se->registerSubsystem(gblvertex);

  if (get_trigger_info)
  {
    TriggerRunInfoReco *triggerruninforeco = new TriggerRunInfoReco();
    se->registerSubsystem(triggerruninforeco);
  }
}

void create_hf_directories(std::string reconstruction_name, std::string &final_output_dir, std::string &output_reco_file)
{
  std::string output_file_name = kfp_header + reconstruction_name + trailer;
  final_output_dir = output_dir + reconstruction_name + "/";
  std::string output_reco_dir = final_output_dir + processing_folder;
  output_reco_file = output_reco_dir + output_file_name;

  std::string makeDirectory = "mkdir -p " + output_reco_dir;
  system(makeDirectory.c_str());
}

void reconstruct_mb_mass()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(mb_reconstruction_name);
  kfparticle->Verbosity(1);

  //kfparticle->setTrackMapNodeName("HFSelected_SvtxTrackMap");
  kfparticle->setDecayDescriptor(mb_decay_descriptor);

  //kfparticle->usePID(use_pid);
  //kfparticle->setPIDacceptFraction(pid_frac);
  kfparticle->dontUseGlobalVertex(dont_use_global_vertex);
  kfparticle->requireTrackVertexBunchCrossingMatch(require_track_and_vertex_match);
  kfparticle->getAllPVInfo(save_all_vtx_info);
  kfparticle->allowZeroMassTracks();
  kfparticle->use2Dmatching(use_2D_matching);
  kfparticle->getTriggerInfo(get_trigger_info);
  kfparticle->getDetectorInfo(get_detector_info);
  kfparticle->saveDST(save_tracks_to_DST);
  kfparticle->doTruthMatching(true);
  kfparticle->saveParticleContainer(false);
  kfparticle->magFieldFile("FIELDMAP_TRACKING");

  //PV to SV cuts
  kfparticle->constrainToPrimaryVertex(true);
  kfparticle->setMotherIPchi2(1000);
  kfparticle->setFlightDistancechi2(-1.);
  kfparticle->setMinDIRA(-1.1); //was .95
  kfparticle->setDecayLengthRange(-0.1, FLT_MAX); //was 0.1 min

  kfparticle->setDecayLengthRange_XY(-10000, FLT_MAX);
  kfparticle->setDecayTimeRange_XY(-10000, FLT_MAX);
  kfparticle->setDecayTimeRange(-10000, FLT_MAX);
  kfparticle->setMinDecayTimeSignificance(-1e5);
  kfparticle->setMinDecayLengthSignificance(-1e5);
  kfparticle->setMinDecayLengthSignificance_XY(-1e5);
  kfparticle->setMaximumDaughterDCA_XY(100);

  //Track parameters
  kfparticle->setMinimumTrackPT(0.0);
  kfparticle->setMinimumTrackIPchi2(-1.);
  kfparticle->setMinimumTrackIP(-1.);
  kfparticle->setMinimumTrackIPchi2_XY(-1.);
  kfparticle->setMinimumTrackIP_XY(-1000.);
  kfparticle->setMaximumTrackchi2nDOF(1000.);
  kfparticle->setMinINTThits(0);
  kfparticle->setMinMVTXhits(0);
  kfparticle->setMinTPChits(5);

  //Vertex parameters
  kfparticle->setMaximumVertexchi2nDOF(200.);
  kfparticle->setMaximumDaughterDCA(100.); //5 mm

  //Parent parameters
  kfparticle->setMotherPT(0);
  kfparticle->setMinimumMass(mb_mass_range[0]); //Check mass ranges
  kfparticle->setMaximumMass(mb_mass_range[1]);
  kfparticle->setMaximumMotherVertexVolume(0.1);

  kfparticle->setOutputName(mb_output_reco_file);
  kfparticle->saveOutput();

  se->registerSubsystem(kfparticle);
}

/*
void reconstruct_pipi_mass()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(pipi_reconstruction_name);
  kfparticle->Verbosity(INT_MAX);

  kfparticle->setTrackMapNodeName("HFSelected_SvtxTrackMap");
  kfparticle->setDecayDescriptor(pipi_decay_descriptor);

  //kfparticle->usePID(use_pid);
  //kfparticle->setPIDacceptFraction(pid_frac);
  kfparticle->dontUseGlobalVertex(dont_use_global_vertex);
  kfparticle->requireTrackVertexBunchCrossingMatch(require_track_and_vertex_match);
  kfparticle->getAllPVInfo(save_all_vtx_info);
  kfparticle->allowZeroMassTracks();
  kfparticle->use2Dmatching(use_2D_matching);
  kfparticle->getTriggerInfo(get_trigger_info);
  kfparticle->getDetectorInfo(get_detector_info);
  kfparticle->saveDST(save_tracks_to_DST);
  kfparticle->setContainerName(pipi_reconstruction_name);
  kfparticle->magFieldFile("FIELDMAP_TRACKING");

  //PV to SV cuts
  kfparticle->constrainToPrimaryVertex(true);
  kfparticle->setMotherIPchi2(100);
  kfparticle->setFlightDistancechi2(-1.);
  kfparticle->setMinDIRA(-1);
  //kfparticle->setMinDIRA(0.999);
  kfparticle->setDecayLengthRange(0.1, FLT_MAX);

  //Track parameters
  kfparticle->setMinimumTrackPT(0.0);
  kfparticle->setMinimumTrackIPchi2(-1.);
  kfparticle->setMinimumTrackIP(-1.);
  kfparticle->setMaximumTrackchi2nDOF(100.);
  kfparticle->setMinTPChits(25);

  //Vertex parameters
  kfparticle->setMaximumVertexchi2nDOF(20);
  kfparticle->setMaximumDaughterDCA(0.5); //5 mm

  //Parent parameters
  kfparticle->setMotherPT(0);
  kfparticle->setMinimumMass(0.40); //Check mass ranges
  kfparticle->setMaximumMass(0.60);
  kfparticle->setMaximumMotherVertexVolume(0.1);

  kfparticle->setOutputName(pipi_output_reco_file);

  se->registerSubsystem(kfparticle);
}
*/

void reconstruct_pipi_mass()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(pipi_reconstruction_name);
  kfparticle->Verbosity(1);

  kfparticle->setTrackMapNodeName("HFSelected_SvtxTrackMap");
  kfparticle->setDecayDescriptor(pipi_decay_descriptor);

  //kfparticle->usePID(use_pid);
  //kfparticle->setPIDacceptFraction(pid_frac);
  kfparticle->dontUseGlobalVertex(dont_use_global_vertex);
  kfparticle->requireTrackVertexBunchCrossingMatch(require_track_and_vertex_match);
  kfparticle->getAllPVInfo(save_all_vtx_info);
  kfparticle->allowZeroMassTracks();
  kfparticle->use2Dmatching(use_2D_matching);
  kfparticle->getTriggerInfo(get_trigger_info);
  kfparticle->getDetectorInfo(get_detector_info);
  kfparticle->saveDST(save_tracks_to_DST);
  kfparticle->doTruthMatching(true);
  kfparticle->saveParticleContainer(true);
  kfparticle->setContainerName(pipi_reconstruction_name); // Note: needed for QA
  kfparticle->magFieldFile("FIELDMAP_TRACKING");

  //PV to SV cuts
  kfparticle->constrainToPrimaryVertex(true);
  kfparticle->setMotherIPchi2(1000);
  kfparticle->setFlightDistancechi2(-1.);
  kfparticle->setMinDIRA(-1.1); //was .95
  kfparticle->setDecayLengthRange(-0.1, FLT_MAX); //was 0.1 min

  kfparticle->setDecayLengthRange_XY(-10000, FLT_MAX);
  kfparticle->setDecayTimeRange_XY(-10000, FLT_MAX);
  kfparticle->setDecayTimeRange(-10000, FLT_MAX);
  kfparticle->setMinDecayTimeSignificance(-1e5);
  kfparticle->setMinDecayLengthSignificance(-1e5);
  kfparticle->setMinDecayLengthSignificance_XY(-1e5);
  kfparticle->setMaximumDaughterDCA_XY(100);

  //Track parameters
  kfparticle->setMinimumTrackPT(0.0);
  kfparticle->setMinimumTrackIPchi2(-1.);
  kfparticle->setMinimumTrackIP(-1.);
  kfparticle->setMinimumTrackIPchi2_XY(-1.);
  kfparticle->setMinimumTrackIP_XY(-1000.);
  kfparticle->setMaximumTrackchi2nDOF(1000.);
  kfparticle->setMinINTThits(0);
  kfparticle->setMinMVTXhits(0);
  kfparticle->setMinTPChits(5);

  //Vertex parameters
  kfparticle->setMaximumVertexchi2nDOF(200.);
  kfparticle->setMaximumDaughterDCA(100.); //5 mm

  //Parent parameters
  kfparticle->setMotherPT(0);
  kfparticle->setMinimumMass(0.40); //Check mass ranges
  kfparticle->setMaximumMass(0.60);
  kfparticle->setMaximumMotherVertexVolume(0.1);

  kfparticle->setOutputName(pipi_output_reco_file);
  kfparticle->saveOutput();

  se->registerSubsystem(kfparticle);

  QAKFParticle *kfpqa = new QAKFParticle("QAKFParticle_K_S0","K_S0",0.4,0.6);
  kfpqa->setKFParticleNodeName(pipi_reconstruction_name);
  kfpqa->enableTrackPtAsymmetry(true); 
  kfpqa->Verbosity(1);
  se->registerSubsystem(kfpqa);
}

/*
void reconstruct_Kpi_mass()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(Kpi_reconstruction_name);
  kfparticle->Verbosity(VERBOSITY_HF);

  kfparticle->setDecayDescriptor(Kpi_decay_descriptor);

  kfparticle->usePID(use_pid);
  kfparticle->setPIDacceptFraction(pid_frac);
  kfparticle->dontUseGlobalVertex(dont_use_global_vertex);
  kfparticle->requireTrackVertexBunchCrossingMatch(require_track_and_vertex_match);
  kfparticle->getAllPVInfo(save_all_vtx_info);
  kfparticle->allowZeroMassTracks();
  kfparticle->use2Dmatching(use_2D_matching);
  kfparticle->getTriggerInfo(get_trigger_info);
  kfparticle->getDetectorInfo(get_detector_info);
  kfparticle->saveDST(save_tracks_to_DST);
  kfparticle->setContainerName(Kpi_reconstruction_name);
  kfparticle->magFieldFile("FIELDMAP_TRACKING");

  //PV to SV cuts
  kfparticle->constrainToPrimaryVertex(true);
  kfparticle->setMotherIPchi2(100);
  kfparticle->setFlightDistancechi2(-1.);
  kfparticle->setMinDIRA(0.985);
  kfparticle->setDecayLengthRange(0.008, FLT_MAX);

  //Track parameters
  kfparticle->setMinimumTrackPT(0.2);
  kfparticle->setMinimumTrackIPchi2(-1.);
  kfparticle->setMinimumTrackIP(-1);
  kfparticle->setMaximumTrackchi2nDOF(100.);
  kfparticle->setMinTPChits(25);
  kfparticle->setMinMVTXhits(3);

  //Vertex parameters
  kfparticle->setMaximumVertexchi2nDOF(14);
  kfparticle->setMaximumDaughterDCA(0.002); //1 mm

  //Parent parameters
  kfparticle->setMotherPT(0.75);
  kfparticle->setMinimumMass(1.83);
  kfparticle->setMaximumMass(1.89);
  kfparticle->setMaximumMotherVertexVolume(0.1);

  kfparticle->setOutputName(Kpi_output_reco_file);

  se->registerSubsystem(kfparticle);
}
*/

void reconstruct_Kpi_mass()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(Kpi_reconstruction_name);
  kfparticle->Verbosity(INT_MAX);

  kfparticle->setTrackMapNodeName("HFSelected_SvtxTrackMap");
  kfparticle->setDecayDescriptor(Kpi_decay_descriptor);

  kfparticle->usePID(use_pid);
  kfparticle->setPIDacceptFraction(pid_frac);
  kfparticle->dontUseGlobalVertex(dont_use_global_vertex);
  kfparticle->requireTrackVertexBunchCrossingMatch(require_track_and_vertex_match);
  kfparticle->getAllPVInfo(save_all_vtx_info);
  kfparticle->allowZeroMassTracks();
  kfparticle->use2Dmatching(use_2D_matching);
  kfparticle->getTriggerInfo(get_trigger_info);
  kfparticle->getDetectorInfo(get_detector_info);
  kfparticle->saveDST(save_tracks_to_DST);
  kfparticle->setContainerName(Kpi_reconstruction_name);
  kfparticle->magFieldFile("FIELDMAP_TRACKING");
  
  //PV to SV cuts
  kfparticle->constrainToPrimaryVertex(true);
  kfparticle->setMotherIPchi2(100);
  kfparticle->setFlightDistancechi2(-1.);
  kfparticle->setMinDIRA(0.88); //was .95
  kfparticle->setDecayLengthRange(-0.1, FLT_MAX); //was 0.1 min

  //Track parameters
  kfparticle->setMinimumTrackPT(0.0);
  kfparticle->setMinimumTrackIPchi2(-1.);
  kfparticle->setMinimumTrackIP(-1.);
  kfparticle->setMaximumTrackchi2nDOF(100.);
  kfparticle->setMinINTThits(0);
  kfparticle->setMinMVTXhits(0);
  kfparticle->setMinTPChits(20);

  //Vertex parameters
  kfparticle->setMaximumVertexchi2nDOF(20);
  kfparticle->setMaximumDaughterDCA(0.5); //5 mm

  //Parent parameters
  kfparticle->setMotherPT(0);
  kfparticle->setMinimumMass(1.75); //Check mass ranges
  kfparticle->setMaximumMass(1.95);
  kfparticle->setMaximumMotherVertexVolume(0.1);

  kfparticle->setOutputName(pipi_output_reco_file);

  se->registerSubsystem(kfparticle);
}

void reconstruct_Kpipi_mass()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(Kpipi_reconstruction_name);
  kfparticle->Verbosity(1);

  kfparticle->setTrackMapNodeName("HFSelected_SvtxTrackMap");
  kfparticle->setDecayDescriptor(Kpipi_decay_descriptor);

  //kfparticle->usePID(use_pid);
  //kfparticle->setPIDacceptFraction(pid_frac);
  kfparticle->dontUseGlobalVertex(dont_use_global_vertex);
  kfparticle->requireTrackVertexBunchCrossingMatch(require_track_and_vertex_match);
  kfparticle->getAllPVInfo(save_all_vtx_info);
  kfparticle->allowZeroMassTracks();
  kfparticle->use2Dmatching(use_2D_matching);
  kfparticle->getTriggerInfo(get_trigger_info);
  kfparticle->getDetectorInfo(get_detector_info);
  kfparticle->saveDST(save_tracks_to_DST);
  kfparticle->doTruthMatching(true);
  kfparticle->saveParticleContainer(false);
  kfparticle->magFieldFile("FIELDMAP_TRACKING");

  //PV to SV cuts
  kfparticle->constrainToPrimaryVertex(true);
  kfparticle->setMotherIPchi2(1000);
  kfparticle->setFlightDistancechi2(-1.);
  kfparticle->setMinDIRA(-1.1); //was .95
  kfparticle->setDecayLengthRange(-0.1, FLT_MAX); //was 0.1 min

  kfparticle->setDecayLengthRange_XY(-10000, FLT_MAX);
  kfparticle->setDecayTimeRange_XY(-10000, FLT_MAX);
  kfparticle->setDecayTimeRange(-10000, FLT_MAX);
  kfparticle->setMinDecayTimeSignificance(-1e5);
  kfparticle->setMinDecayLengthSignificance(-1e5);
  kfparticle->setMinDecayLengthSignificance_XY(-1e5);
  kfparticle->setMaximumDaughterDCA_XY(100);

  //Track parameters
  kfparticle->setMinimumTrackPT(0.0);
  kfparticle->setMinimumTrackIPchi2(-1.);
  kfparticle->setMinimumTrackIP(-1.);
  kfparticle->setMinimumTrackIPchi2_XY(-1.);
  kfparticle->setMinimumTrackIP_XY(-1000.);
  kfparticle->setMaximumTrackchi2nDOF(1000.);
  kfparticle->setMinINTThits(0);
  kfparticle->setMinMVTXhits(0);
  kfparticle->setMinTPChits(5);

  //Vertex parameters
  kfparticle->setMaximumVertexchi2nDOF(200.);
  kfparticle->setMaximumDaughterDCA(100.); //5 mm

  //Parent parameters
  kfparticle->setMotherPT(0);
  kfparticle->setMinimumMass(1.5); //Check mass ranges
  kfparticle->setMaximumMass(2.2);
  kfparticle->setMaximumMotherVertexVolume(0.1);

  kfparticle->setOutputName(Kpipi_output_reco_file);
  kfparticle->saveOutput();

  se->registerSubsystem(kfparticle);
}

void reconstruct_KKpi_mass()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(KKpi_reconstruction_name);
  kfparticle->Verbosity(1);

  kfparticle->setTrackMapNodeName("HFSelected_SvtxTrackMap");
  kfparticle->setDecayDescriptor(KKpi_decay_descriptor);

  //kfparticle->usePID(use_pid);
  //kfparticle->setPIDacceptFraction(pid_frac);
  kfparticle->dontUseGlobalVertex(dont_use_global_vertex);
  kfparticle->requireTrackVertexBunchCrossingMatch(require_track_and_vertex_match);
  kfparticle->getAllPVInfo(save_all_vtx_info);
  kfparticle->allowZeroMassTracks();
  kfparticle->use2Dmatching(use_2D_matching);
  kfparticle->getTriggerInfo(get_trigger_info);
  kfparticle->getDetectorInfo(get_detector_info);
  kfparticle->saveDST(save_tracks_to_DST);
  kfparticle->doTruthMatching(true);
  kfparticle->saveParticleContainer(false);
  kfparticle->magFieldFile("FIELDMAP_TRACKING");

  //PV to SV cuts
  kfparticle->constrainToPrimaryVertex(true);
  kfparticle->setMotherIPchi2(1000);
  kfparticle->setFlightDistancechi2(-1.);
  kfparticle->setMinDIRA(-1.1); //was .95
  kfparticle->setDecayLengthRange(-1*FLT_MAX, FLT_MAX); //was 0.1 min

  kfparticle->setDecayLengthRange_XY(-1*FLT_MAX, FLT_MAX);
  kfparticle->setDecayTimeRange_XY(-1*FLT_MAX, FLT_MAX);
  kfparticle->setDecayTimeRange(-1*FLT_MAX, FLT_MAX);
  kfparticle->setMinDecayTimeSignificance(-1e5);
  kfparticle->setMinDecayLengthSignificance(-1e5);
  kfparticle->setMinDecayLengthSignificance_XY(-1e5);
  kfparticle->setMaximumDaughterDCA_XY(100);

  //Track parameters
  kfparticle->setMinimumTrackPT(0.0);
  kfparticle->setMinimumTrackIPchi2(-1.);
  kfparticle->setMinimumTrackIP(-1.);
  kfparticle->setMinimumTrackIPchi2_XY(-1.);
  kfparticle->setMinimumTrackIP_XY(-1000.);
  kfparticle->setMaximumTrackchi2nDOF(1000.);
  kfparticle->setMinINTThits(0);
  kfparticle->setMinMVTXhits(0);
  kfparticle->setMinTPChits(5);

  //Vertex parameters
  kfparticle->setMaximumVertexchi2nDOF(200.);
  kfparticle->setMaximumDaughterDCA(100.); //5 mm

  //Parent parameters
  kfparticle->setMotherPT(0);
  kfparticle->setMinimumMass(1.5); //Check mass ranges
  kfparticle->setMaximumMass(2.2);
  kfparticle->setMaximumMotherVertexVolume(0.1);

  //Intermediate parameters
  kfparticle->constrainIntermediateMasses(constrain_phi_mass);

  std::vector<std::pair<float, float>> intermediate_mass_range;
  intermediate_mass_range.push_back(make_pair(0.98, 1.05));
  kfparticle->setIntermediateMassRange(intermediate_mass_range);

  std::vector<float> intermediate_min_pt = {0.};
  kfparticle->setIntermediateMinPT(intermediate_min_pt);

  std::vector<std::pair<float, float>> intermediate_IP_range;
  intermediate_IP_range.push_back(make_pair(-1., FLT_MAX));
  kfparticle->setIntermediateIPRange(intermediate_IP_range);

  std::vector<std::pair<float, float>> intermediate_IP_XY_range;
  intermediate_IP_XY_range.push_back(make_pair(-1., FLT_MAX));
  kfparticle->setIntermediateIPRange_XY(intermediate_IP_XY_range);

  std::vector<std::pair<float, float>> intermediate_IPchi2_range;
  intermediate_IPchi2_range.push_back(make_pair(-100000., 100000.));
  kfparticle->setIntermediateIPchi2Range(intermediate_IPchi2_range);

  std::vector<std::pair<float, float>> intermediate_IPchi2_XY_range;
  intermediate_IPchi2_XY_range.push_back(make_pair(-100000., 100000.));
  kfparticle->setIntermediateIPchi2Range_XY(intermediate_IPchi2_XY_range);

  std::vector<float> intermediate_min_dira = {-1.1};
  kfparticle->setIntermediateMinDIRA(intermediate_min_dira);

  std::vector<float> intermediate_min_FDchi2 = {-1.};
  kfparticle->setIntermediateMinFDchi2(intermediate_min_FDchi2);

  std::vector<float> intermediate_max_vertex_vol = {0.1};
  kfparticle->setIntermediateMaxVertexVolume(intermediate_max_vertex_vol);

  kfparticle->setOutputName(KKpi_output_reco_file);
  kfparticle->saveOutput();

  se->registerSubsystem(kfparticle);
}

void reconstruct_pKpi_mass()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(pKpi_reconstruction_name);
  kfparticle->Verbosity(1);

  kfparticle->setTrackMapNodeName("HFSelected_SvtxTrackMap");
  kfparticle->setDecayDescriptor(pKpi_decay_descriptor);

  //kfparticle->usePID(use_pid);
  //kfparticle->setPIDacceptFraction(pid_frac);
  kfparticle->dontUseGlobalVertex(dont_use_global_vertex);
  kfparticle->requireTrackVertexBunchCrossingMatch(require_track_and_vertex_match);
  kfparticle->getAllPVInfo(save_all_vtx_info);
  kfparticle->allowZeroMassTracks();
  kfparticle->use2Dmatching(use_2D_matching);
  kfparticle->getTriggerInfo(get_trigger_info);
  kfparticle->getDetectorInfo(get_detector_info);
  kfparticle->saveDST(save_tracks_to_DST);
  kfparticle->doTruthMatching(true);
  kfparticle->saveParticleContainer(false);
  kfparticle->magFieldFile("FIELDMAP_TRACKING");

  //PV to SV cuts
  kfparticle->constrainToPrimaryVertex(true);
  kfparticle->setMotherIPchi2(1000);
  kfparticle->setFlightDistancechi2(-1.);
  kfparticle->setMinDIRA(-1.1); //was .95
  kfparticle->setDecayLengthRange(-0.1, FLT_MAX); //was 0.1 min

  kfparticle->setDecayLengthRange_XY(-10000, FLT_MAX);
  kfparticle->setDecayTimeRange_XY(-10000, FLT_MAX);
  kfparticle->setDecayTimeRange(-10000, FLT_MAX);
  kfparticle->setMinDecayTimeSignificance(-1e5);
  kfparticle->setMinDecayLengthSignificance(-1e5);
  kfparticle->setMinDecayLengthSignificance_XY(-1e5);
  kfparticle->setMaximumDaughterDCA_XY(100);

  //Track parameters
  kfparticle->setMinimumTrackPT(0.0);
  kfparticle->setMinimumTrackIPchi2(-1.);
  kfparticle->setMinimumTrackIP(-1.);
  kfparticle->setMinimumTrackIPchi2_XY(-1.);
  kfparticle->setMinimumTrackIP_XY(-1000.);
  kfparticle->setMaximumTrackchi2nDOF(1000.);
  kfparticle->setMinINTThits(0);
  kfparticle->setMinMVTXhits(0);
  kfparticle->setMinTPChits(5);

  //Vertex parameters
  kfparticle->setMaximumVertexchi2nDOF(200.);
  kfparticle->setMaximumDaughterDCA(100.); //5 mm

  //Parent parameters
  kfparticle->setMotherPT(0);
  kfparticle->setMinimumMass(2.1); //Check mass ranges
  kfparticle->setMaximumMass(2.6);
  kfparticle->setMaximumMotherVertexVolume(0.1);

  kfparticle->setOutputName(pKpi_output_reco_file);
  kfparticle->saveOutput();

  se->registerSubsystem(kfparticle);
}

void end_kfparticle(std::string full_file_name, std::string final_path)
{
  ifstream file(full_file_name.c_str());
  if (file.good())
  {
    string moveOutput = "mv " + full_file_name + " " + final_path;
    system(moveOutput.c_str());
  }
}
