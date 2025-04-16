#include "MvtxMatchingEfficiency.h"

#include <g4detectors/PHG4CylinderGeomContainer.h>

#include <trackbase/ActsGeometry.h>
#include <trackbase/MvtxDefs.h>
#include <trackbase/InttDefs.h>
#include <trackbase/TrackFitUtils.h>
#include <trackbase/TrkrCluster.h>
#include <trackbase/TrkrClusterContainer.h>
#include <trackbase/TrkrClusterHitAssoc.h>
#include <trackbase/TrkrDefs.h>
#include <trackbase/TrkrHit.h>
#include <trackbase/TrkrHitSet.h>
#include <trackbase/TrkrHitSetContainer.h>
#include <trackbase_historic/SvtxTrackMap.h>
#include <trackbase_historic/TrackAnalysisUtils.h>
#include <trackbase_historic/TrackSeedContainer.h>
#include <trackbase_historic/TrackSeed.h>
#include <g4detectors/PHG4TpcCylinderGeom.h>
#include <g4detectors/PHG4TpcCylinderGeomContainer.h>


#include <fun4all/Fun4AllHistoManager.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/SubsysReco.h>
#include <fun4all/PHTFileServer.h>

#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

#include <TH1.h>
#include <TH1D.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <algorithm>
#include <set>

//____________________________________________________________________________..
MvtxMatchingEfficiency::MvtxMatchingEfficiency(const std::string &name,const std::string outputfilename)
  : SubsysReco(name), m_outputFileName(outputfilename)
{
}

//____________________________________________________________________________..
int MvtxMatchingEfficiency::InitRun(PHCompositeNode */*topNode*/)
{
  // print configuration
  std::cout << "MvtxMatchingEfficiency::InitRun "<< std::endl;

  PHTFileServer::get().open(m_outputFileName, "RECREATE");

  h_silicon_match[0] = new TH1D("h_intt_2_mvtx_111","h_intt_2_mvtx_111",100,0,10);
  h_silicon_match[1] = new TH1D("h_intt_2_mvtx_110","h_intt_2_mvtx_110",100,0,10);
  h_silicon_match[2] = new TH1D("h_intt_2_mvtx_101","h_intt_2_mvtx_101",100,0,10);
  h_silicon_match[3] = new TH1D("h_intt_2_mvtx_011","h_intt_2_mvtx_011",100,0,10);
  h_silicon_match[4] = new TH1D("h_intt_2_mvtx_2","h_intt_2_mvtx_2",100,0,10);

  h_silicon_match_etaphi[0] = new TH2D("h_intt_2_mvtx_111_etaphi","h_intt_2_mvtx_111_etaphi",44,-1.1,1.1,64,-3.2,3.2);
  h_silicon_match_etaphi[1] = new TH2D("h_intt_2_mvtx_110_etaphi","h_intt_2_mvtx_110_etaphi",44,-1.1,1.1,64,-3.2,3.2);
  h_silicon_match_etaphi[2] = new TH2D("h_intt_2_mvtx_101_etaphi","h_intt_2_mvtx_101_etaphi",44,-1.1,1.1,64,-3.2,3.2);
  h_silicon_match_etaphi[3] = new TH2D("h_intt_2_mvtx_011_etaphi","h_intt_2_mvtx_011_etaphi",44,-1.1,1.1,64,-3.2,3.2);
  h_silicon_match_etaphi[4] = new TH2D("h_intt_2_mvtx_2_etaphi","h_intt_2_mvtx_2_etaphi",44,-1.1,1.1,64,-3.2,3.2);

  h_seed_match[0] = new TH1D("h_seed_intt_2_mvtx_111","h_seed_intt_2_mvtx_111",100,0,10);
  h_seed_match[1] = new TH1D("h_seed_intt_2_mvtx_110","h_seed_intt_2_mvtx_110",100,0,10);
  h_seed_match[2] = new TH1D("h_seed_intt_2_mvtx_101","h_seed_intt_2_mvtx_101",100,0,10);
  h_seed_match[3] = new TH1D("h_seed_intt_2_mvtx_011","h_seed_intt_2_mvtx_011",100,0,10);
  h_seed_match[4] = new TH1D("h_seed_intt_2_mvtx_2","h_seed_intt_2_mvtx_2",100,0,10);

  h_seed_match_etaphi[0] = new TH2D("h_intt_2_mvtx_111_etaphi_seed","h_intt_2_mvtx_111_etaphi_seed",44,-1.1,1.1,64,-3.2,3.2);
  h_seed_match_etaphi[1] = new TH2D("h_intt_2_mvtx_110_etaphi_seed","h_intt_2_mvtx_110_etaphi_seed",44,-1.1,1.1,64,-3.2,3.2);
  h_seed_match_etaphi[2] = new TH2D("h_intt_2_mvtx_101_etaphi_seed","h_intt_2_mvtx_101_etaphi_seed",44,-1.1,1.1,64,-3.2,3.2);
  h_seed_match_etaphi[3] = new TH2D("h_intt_2_mvtx_011_etaphi_seed","h_intt_2_mvtx_011_etaphi_seed",44,-1.1,1.1,64,-3.2,3.2);
  h_seed_match_etaphi[4] = new TH2D("h_intt_2_mvtx_2_etaphi_seed","h_intt_2_mvtx_2_etaphi_seed",44,-1.1,1.1,64,-3.2,3.2);



  std::string matName[2] = {"wTPC","justSilicon"};
  for(int i = 0; i<2;i++){ 
    TString name = "h_status_"+matName[i];
    h_status[i]= new TH1I(name,name,8,0,8);
    h_status[i]->GetXaxis()->SetBinLabel(1,"Total");
    h_status[i]->GetXaxis()->SetBinLabel(2,"INTT >= 2");
    h_status[i]->GetXaxis()->SetBinLabel(3,"INTT same crossing");
    h_status[i]->GetXaxis()->SetBinLabel(4,"INTT unique clusters");
    h_status[i]->GetXaxis()->SetBinLabel(5,"MVTX unique clusters");
    h_status[i]->GetXaxis()->SetBinLabel(6,"z < 10 cm");
    h_status[i]->GetXaxis()->SetBinLabel(7,"eta < 1.1");
    h_status[i]->GetXaxis()->SetBinLabel(8,"MVTX >= 2 layer");


    name = "h_INTT_time_delta_"+matName[i];
    h_INTT_time_delta[i]= new TH1I(name,name,900,0.5,900.5);
  }

  // Create a new TTree
  tree = new TTree("TPC_tracks", "TPC_tracks");


   // Create branches
   tree->Branch("pt", &pt, "pt/F");
   tree->Branch("eta", &eta, "eta/F");
   tree->Branch("phi", &phi, "phi/F");
   tree->Branch("frac_p_z", &frac_p_z, "frac_p_z/F");
   tree->Branch("dEdx", &dEdx, "dEdx/F");
   tree->Branch("nTPC", &nTPC, "nTPC/I"); 
   tree->Branch("layers", &layers, "layers/I");  
   tree->Branch("states", &states, "states/I"); 
   tree->Branch("residual_l0", &residual_l0, "residual_l0/F"); 
   tree->Branch("residual_l1", &residual_l1, "residual_l1/F"); 
   tree->Branch("residual_l2", &residual_l2, "residual_l2/F"); 

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int MvtxMatchingEfficiency::process_event(PHCompositeNode *topNode)
{
  ievent++;
   
  SvtxTrackMap *trackmap = findNode::getClass<SvtxTrackMap>(topNode, "SvtxTrackMap");
  if (!trackmap){
    std::cout << PHWHERE
      << "SvtxTrackMap node is missing, can't collect particles"
      << std::endl;
    return -1;
  }

  TrackSeedContainer *silicon_track_map = findNode::getClass<TrackSeedContainer>(topNode, "SiliconTrackSeedContainer");
  if (!silicon_track_map){
    std::cout << PHWHERE
      << "SiliconTrackSeedContainer node is missing, can't collect particles"
      << std::endl;
    return -1;
  }

  _cluster_map = findNode::getClass<TrkrClusterContainer>(topNode, "CORRECTED_TRKR_CLUSTER");
  if (!_cluster_map){
    _cluster_map = findNode::getClass<TrkrClusterContainer>(topNode, "TRKR_CLUSTER");
  }

  if (!_cluster_map){
    std::cout << PHWHERE
      << "TrkrClusterContainer node is missing"
      << std::endl;
    return -1;
  }

  _geom_container = findNode::getClass<PHG4TpcCylinderGeomContainer>(topNode, "CYLINDERCELLGEOM_SVTX");
  if (!_geom_container)
  {
    std::cout << PHWHERE << "ERROR: Can't find node CYLINDERCELLGEOM_SVTX" << std::endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  m_tGeometry = findNode::getClass<ActsGeometry>(topNode, "ActsGeometry");
  if (!m_tGeometry)
  {
    std::cout << PHWHERE << "No acts reco geometry, bailing.";
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  // Variables to hold branch data


 

  //std::cout<<"SvtxTrackMap"<<std::endl;
  //fetch MVTX clusters
  std::vector<TrkrDefs::cluskey> cluster_vector;
  std::vector<TrkrDefs::cluskey> cluster_vector_INTT;
  for (auto it = trackmap->begin(); it != trackmap->end(); ++it) {
    for (const auto& ckey : TrackAnalysisUtils::get_cluster_keys((*it).second)){
      switch (TrkrDefs::getTrkrId(ckey)){
        case TrkrDefs::mvtxId:
          cluster_vector.push_back(ckey);
          break;
        case TrkrDefs::inttId:
          cluster_vector_INTT.push_back(ckey);
          break;
      }
    }
  }

  std::set<TrkrDefs::cluskey> duplicate_cluster_vector = findDuplicates(cluster_vector);
  //std::cout<<"mvtx track seeds: "<<cluster_vector.size()<<" duplicate track seeds "<<duplicate_cluster_vector.size()<<std::endl;
  std::set<TrkrDefs::cluskey> duplicate_cluster_vector_INTT = findDuplicates(cluster_vector_INTT);
  //std::cout<<"intt track seeds: "<<cluster_vector_INTT.size()<<" duplicate track seeds "<<duplicate_cluster_vector_INTT.size()<<std::endl;
  
  for (auto it = trackmap->begin(); it != trackmap->end(); ++it) {
    //number of cluster per subsystem
    int n_mvtx_hits = 0;
    int n_intt_hits = 0;
    int n_tpc_hits = 0;
    //fired mvtx layers 0 - innermost , 2 - outermost
    bool mvtx_l[3] = {false,false,false};

    int n_mvtx_hits_state = 0;
    bool mvtx_l_state[3] = {false,false,false};
    float mvtx_l_state_residual[3] = {-1,-1,-1};

    std::vector<float> intt_time;

    SvtxTrack *track = (*it).second;
    //std::cout<<"new track ";
    for (const auto& ckey : TrackAnalysisUtils::get_cluster_keys(track)){
      switch (TrkrDefs::getTrkrId(ckey)){
        case TrkrDefs::mvtxId:
          n_mvtx_hits++;
          mvtx_l[static_cast<int>(TrkrDefs::getLayer(ckey))]=true;
          break;
        case TrkrDefs::inttId:
          //std::cout<<static_cast<int>(TrkrDefs::getLayer(ckey))<<" "<<InttDefs::getTimeBucketId(ckey)<<" ";
          intt_time.push_back(InttDefs::getTimeBucketId(ckey));
          n_intt_hits++;
          break;
        case TrkrDefs::tpcId:
          n_tpc_hits++;
          break;
      }
    }
    //std::cout<<std::endl;

    for (auto state_it = track->begin_states(); state_it != track->end_states(); ++state_it) {
      auto clus_key = state_it->second->get_cluskey();
      switch (TrkrDefs::getTrkrId(clus_key)){
        case TrkrDefs::mvtxId:
          n_mvtx_hits_state++;
          mvtx_l_state[static_cast<int>(TrkrDefs::getLayer(clus_key))]=true;
          //std::cout<<"state   "<<state_it->second->get_x()<<" "<<state_it->second->get_y()<<" "<<state_it->second->get_z()<<std::endl;
          TrkrCluster* cluster = _cluster_map->findCluster(clus_key);
          Acts::Vector3 pos = m_tGeometry->getGlobalPosition(clus_key,cluster);
          //std::cout<<"cluster "<<pos.x()<<" "<<pos.y()<<" "<<pos.z()<<std::endl;

          //std::cout<<"residual: "<<TMath::Sqrt((state_it->second->get_x() - pos.x())*(state_it->second->get_x() - pos.x()) + (state_it->second->get_y() - pos.y())*(state_it->second->get_y() - pos.y()) + (state_it->second->get_z() - pos.z())*(state_it->second->get_z() - pos.z()))<<std::endl;

          mvtx_l_state_residual[static_cast<int>(TrkrDefs::getLayer(clus_key))] = TMath::Sqrt((state_it->second->get_x() - pos.x())*(state_it->second->get_x() - pos.x()) + (state_it->second->get_y() - pos.y())*(state_it->second->get_y() - pos.y()) + (state_it->second->get_z() - pos.z())*(state_it->second->get_z() - pos.z()));
          break;
      }
    }

    //std::cout<<"a "<<n_tpc_hits<<" "<<mvtx_l_state_residual[0]<<" "<<mvtx_l_state_residual[1]<<" "<<mvtx_l_state_residual[2]<<std::endl;


    /*std::cout<<"hits: "<<n_mvtx_hits<<" hits state: "<<n_mvtx_hits_state<<std::endl;

    std::cout<<"layers: ";
    for (int j = 0; j < 3; ++j) {
      std::cout << (mvtx_l[j] ? 1 : 0) << " ";
    }

    std::cout<<" state layers: ";
    for (int j = 0; j < 3; ++j) {
      std::cout << (mvtx_l_state[j] ? 1 : 0) << " ";
    }
    std::cout<<std::endl;*/

    

    

    h_status[0]->Fill(0.5);

    //require number of INTT clusters >= 2
    if(n_intt_hits < 2) continue;
    h_status[0]->Fill(1.5);

    //require the INTT clusters are from same crossing
    bool good_intt_time = std::all_of(intt_time.begin() + 1, intt_time.end(), [&](int i){ return i == intt_time[0]; }); 
    intt_time.clear();
    if(good_intt_time == false){
      if(n_intt_hits ==2) h_INTT_time_delta[0]->Fill(TMath::Abs(intt_time[0]-intt_time[1]));
      continue;
    } 
    h_status[0]->Fill(2.5);

    //require the tracks does not share INTT clusters
    bool good_track_INTT = true;
    for (const auto& ckey : TrackAnalysisUtils::get_cluster_keys(track)){
      if (duplicate_cluster_vector_INTT.find(ckey) != duplicate_cluster_vector_INTT.end()) {
        good_track_INTT = false;
      }
    }
    if(good_track_INTT == false) continue;
    h_status[0]->Fill(3.5);

    //require the tracks does not share MVTX clusters
    bool good_track = true;
    for (const auto& ckey : TrackAnalysisUtils::get_cluster_keys(track)){
      if (duplicate_cluster_vector.find(ckey) != duplicate_cluster_vector.end()) {
        good_track = false;
      }
    }
    if(good_track == false) continue;
    h_status[0]->Fill(4.5);

    // z position cut
    if(TMath::Abs(track->get_z()) > 10 ) continue;
    h_status[0]->Fill(5.5);

    //eta cut
    if(TMath::Abs(track->get_eta()) > 1.1 ) continue;
    h_status[0]->Fill(6.5);

    // require 2 or 3 MVTX layers fired (layers not clusters)
    if(std::count_if(std::begin(mvtx_l), std::end(mvtx_l), [](bool i){ return i==true; }) < 2) continue;
    h_status[0]->Fill(7.5);

    //fill Ttree variables
    pt = track->get_pt();
    eta = track->get_eta();
    phi = track->get_phi();
    frac_p_z = track->get_p() / track->get_charge();
    dEdx = calc_dedx(track->get_tpc_seed());
    layers = -1;
    nTPC = n_tpc_hits;
    residual_l0 = mvtx_l_state_residual[0];
    residual_l1 = mvtx_l_state_residual[1];
    residual_l2 = mvtx_l_state_residual[2];

    std::cout<<"b "<<n_tpc_hits<<" "<<n_mvtx_hits<<" "<<n_mvtx_hits_state<<" "<<mvtx_l_state_residual[0]<<" "<<mvtx_l_state_residual[1]<<" "<<mvtx_l_state_residual[2]<<std::endl;

    // 3 MVTX layers fired
    if(mvtx_l[0] == true && mvtx_l[1] == true && mvtx_l[2] == true){
       h_silicon_match[0]->Fill(1./track->get_pt());
       h_silicon_match_etaphi[0]->Fill(track->get_eta(),track->get_phi());
       layers = 7;
    }
    // Outermost MVTX missing
    if(mvtx_l[0] == true && mvtx_l[1] == true && mvtx_l[2] == false){
      h_silicon_match[1]->Fill(1./track->get_pt());
      h_silicon_match_etaphi[1]->Fill(track->get_eta(),track->get_phi());
      layers = 3;
    } 
    // Middle MVTX missing
    if(mvtx_l[0] == true && mvtx_l[1] == false && mvtx_l[2] == true){
      h_silicon_match[2]->Fill(1./track->get_pt());
      h_silicon_match_etaphi[2]->Fill(track->get_eta(),track->get_phi());
      layers = 5;
    } 
    // Innermost missing
    if(mvtx_l[0] == false && mvtx_l[1] == true && mvtx_l[2] == true){
      h_silicon_match[3]->Fill(1./track->get_pt());
      h_silicon_match_etaphi[3]->Fill(track->get_eta(),track->get_phi());
      layers = 6;
    } 
    // any 2 MVTX layers fired
    if(std::count_if(std::begin(mvtx_l), std::end(mvtx_l), [](bool i){ return i==true; })>= 2){
      h_silicon_match[4]->Fill(1./track->get_pt());
      h_silicon_match_etaphi[4]->Fill(track->get_eta(),track->get_phi());
    } 


    // 3 MVTX layers fired
    if(mvtx_l_state[0] == true && mvtx_l_state[1] == true && mvtx_l_state[2] == true){
      states = 7;
   }
   // Outermost MVTX missing
   if(mvtx_l_state[0] == true && mvtx_l_state[1] == true && mvtx_l_state[2] == false){
    states = 3;
   } 
   // Middle MVTX missing
   if(mvtx_l_state[0] == true && mvtx_l_state[1] == false && mvtx_l_state[2] == true){
    states = 5;
   } 
   // Innermost missing
   if(mvtx_l_state[0] == false && mvtx_l_state[1] == true && mvtx_l_state[2] == true){
    states = 6;
   } 
    tree->Fill();
  }

  //clear event vectors for tracks
  cluster_vector.clear();
  duplicate_cluster_vector.clear();
  cluster_vector_INTT.clear();
  duplicate_cluster_vector_INTT.clear();


  //lets look at seeds
  std::vector<TrkrDefs::cluskey> cluster_vector_seed;
  std::vector<TrkrDefs::cluskey> cluster_vector_seed_INTT;
  //fetch MVTX clusters
  for (auto it = silicon_track_map->begin(); it != silicon_track_map->end(); ++it) {
    if(*it == nullptr) continue;
    for (auto it2 = (*it)->begin_cluster_keys(); it2 != (*it)->end_cluster_keys(); ++it2) {
      switch (TrkrDefs::getTrkrId(*it2)){
        case TrkrDefs::mvtxId:
          cluster_vector_seed.push_back(*it2);
        break;
        case TrkrDefs::inttId:
          cluster_vector_seed_INTT.push_back(*it2);
        break;
      }
    }
  }

  std::set<TrkrDefs::cluskey> duplicate_cluster_vector_seed = findDuplicates(cluster_vector_seed);
  std::set<TrkrDefs::cluskey> duplicate_cluster_vector_seed_INTT = findDuplicates(cluster_vector_seed_INTT);
  //std::cout<<"mvtx seed seeds: "<<cluster_vector_seed.size()<<" duplicate seed seeds "<<duplicate_cluster_vector_seed.size()<<std::endl;

  //std::cout<<"SiliconTrackSeedContainer"<<std::endl;
  for (auto it = silicon_track_map->begin(); it != silicon_track_map->end(); ++it) {
    if(*it == nullptr) continue;
    std::vector<float> intt_time;
    int n_mvtx_hits = 0;
    int n_intt_hits = 0;
    bool mvtx_l[3] = {false,false,false};
    //bool intt_l[3] = {false,false};
    //std::cout<<"new track ";
    for (auto it2 = (*it)->begin_cluster_keys(); it2 != (*it)->end_cluster_keys(); ++it2) {
      switch (TrkrDefs::getTrkrId(*it2)){
        case TrkrDefs::mvtxId:
          n_mvtx_hits++;
          mvtx_l[static_cast<int>(TrkrDefs::getLayer(*it2))]=true;
          break;
        case TrkrDefs::inttId:
          //intt_l[static_cast<int>(TrkrDefs::getLayer(*it2))]=true;
          
          intt_time.push_back(InttDefs::getTimeBucketId(*it2));
          n_intt_hits++;
          break;
      }
    }
    //std::cout<<std::endl;


    h_status[1]->Fill(0.5);

    //require >=2 INTT clusters
    if(n_intt_hits < 2) continue;
    h_status[1]->Fill(1.5);

    //all INTT clusters must eb from same crossing
    bool good_intt_time = std::all_of(intt_time.begin() + 1, intt_time.end(), [&](int i){ return i == intt_time[0]; }); 
    intt_time.clear();
    if(good_intt_time == false){
      if(n_intt_hits ==2) h_INTT_time_delta[1]->Fill(TMath::Abs(intt_time[0]-intt_time[1]));
      continue;
    } 
    h_status[1]->Fill(2.5);

    //they cannot share MVTX clusters
    bool good_track_INTT = true;
    for (auto it2 = (*it)->begin_cluster_keys(); it2 != (*it)->end_cluster_keys(); ++it2) {
      if (duplicate_cluster_vector_seed_INTT.find(*it2) != duplicate_cluster_vector_seed_INTT.end()) {
        good_track_INTT = false;
      }
    }
    if(good_track_INTT == false) continue;
    h_status[1]->Fill(3.5);

    //they cannot share MVTX clusters
    bool good_track = true;
    for (auto it2 = (*it)->begin_cluster_keys(); it2 != (*it)->end_cluster_keys(); ++it2) {
      if (duplicate_cluster_vector_seed.find(*it2) != duplicate_cluster_vector_seed.end()) {
        good_track = false;
      }
    }
    if(good_track == false) continue;
    h_status[1]->Fill(4.5);

    //z position cut
    if(TMath::Abs((*it)->get_Z0()) > 10 ) continue;
    h_status[1]->Fill(5.5);

    //eta cut
    if(TMath::Abs((*it)->get_eta()) > 1.1 ) continue;
    h_status[1]->Fill(6.5);

    // require 2 or 3 MVTX layers fired (layers not clusters)
    if(std::count_if(std::begin(mvtx_l), std::end(mvtx_l), [](bool i){ return i==true; }) < 2) continue;
    h_status[1]->Fill(7.5);

    // 3 MVTX layers fired
    if(mvtx_l[0] == true && mvtx_l[1] == true && mvtx_l[2] == true){
      h_seed_match[0]->Fill(1./(*it)->get_pt());
      h_seed_match_etaphi[0]->Fill((*it)->get_eta(),(*it)->get_phi());
    } 
    // Outermost MVTX missing
    if(mvtx_l[0] == true && mvtx_l[1] == true && mvtx_l[2] == false){
      h_seed_match[1]->Fill(1./(*it)->get_pt());
      h_seed_match_etaphi[1]->Fill((*it)->get_eta(),(*it)->get_phi());
    } 
    // Middle MVTX missing
    if(mvtx_l[0] == true && mvtx_l[1] == false && mvtx_l[2] == true){
      h_seed_match[2]->Fill(1./(*it)->get_pt());
      h_seed_match_etaphi[2]->Fill((*it)->get_eta(),(*it)->get_phi());
    } 
    // Innermost missing
    if(mvtx_l[0] == false && mvtx_l[1] == true && mvtx_l[2] == true){
      h_seed_match[3]->Fill(1./(*it)->get_pt());
      h_seed_match_etaphi[3]->Fill((*it)->get_eta(),(*it)->get_phi());
    } 
    // any 2 MVTX layers fired
    if(std::count_if(std::begin(mvtx_l), std::end(mvtx_l), [](bool i){ return i==true; })>= 2){
      h_seed_match[4]->Fill(1./(*it)->get_pt());
      h_seed_match_etaphi[4]->Fill((*it)->get_eta(),(*it)->get_phi());
    } 
  }

  //clear event seed variables
  cluster_vector_seed.clear();
  duplicate_cluster_vector_seed.clear();

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int MvtxMatchingEfficiency::EndRun(const int /*runnumber*/)
{ 
  //TFile *root_out = new TFile("MVTX_ME.root","RECREATE");

  std::cout << "MvtxMatchingEfficiency::End - Output to " << m_outputFileName << std::endl;
  
  if(PHTFileServer::get().cd(m_outputFileName)){
    for(int i = 0; i<5;i++){
      h_silicon_match[i]->Write();
      h_silicon_match_etaphi[i]->Write();
      h_seed_match[i]->Write();
      h_seed_match_etaphi[i]->Write();
    }
  
    h_status[0]->Write();
    h_status[1]->Write();

    h_INTT_time_delta[0]->Write();
    h_INTT_time_delta[1]->Write();

    tree->Write();
  }

  std::cout << "MvtxMatchingEfficiency::End(PHCompositeNode *topNode) This is the End..." << std::endl;

  return Fun4AllReturnCodes::EVENT_OK; 
}


float MvtxMatchingEfficiency::calc_dedx(TrackSeed *tpcseed){

  std::vector<TrkrDefs::cluskey> clusterKeys;
  clusterKeys.insert(clusterKeys.end(), tpcseed->begin_cluster_keys(),tpcseed->end_cluster_keys());

  std::vector<float> dedxlist;
  for (unsigned long cluster_key : clusterKeys){
    unsigned int layer_local = TrkrDefs::getLayer(cluster_key);
    if(TrkrDefs::getTrkrId(cluster_key) != TrkrDefs::TrkrId::tpcId){
      continue;
    }
    TrkrCluster* cluster = _cluster_map->findCluster(cluster_key);

    float adc = cluster->getAdc();
    PHG4TpcCylinderGeom* GeoLayer_local = _geom_container->GetLayerCellGeom(layer_local);
    float thick = GeoLayer_local->get_thickness();
    
    float r = GeoLayer_local->get_radius();
    float alpha = (r * r) / (2 * r * TMath::Abs(1.0 / tpcseed->get_qOverR()));
    float beta = atan(tpcseed->get_slope());
    float alphacorr = cos(alpha);
    if(alphacorr<0||alphacorr>4){
      alphacorr=4;
    }
    float betacorr = cos(beta);
    if(betacorr<0||betacorr>4){
      betacorr=4;
    }
    adc/=thick;
    adc*=alphacorr;
    adc*=betacorr;
    dedxlist.push_back(adc);
    sort(dedxlist.begin(), dedxlist.end());
  }
  int trunc_min = 0;
  int trunc_max = (int)dedxlist.size()*0.7;
  float sumdedx = 0;
  int ndedx = 0;
  for(int j = trunc_min; j<=trunc_max;j++){
    sumdedx+=dedxlist.at(j);
    ndedx++;
  }
  sumdedx/=ndedx;
  return sumdedx;
}


std::set<TrkrDefs::cluskey> MvtxMatchingEfficiency::findDuplicates(std::vector<TrkrDefs::cluskey> vec){
  std::set<TrkrDefs::cluskey> duplicates;
  std::sort(vec.begin(), vec.end());
  std::set<TrkrDefs::cluskey> distinct(vec.begin(), vec.end());
  std::set_difference(vec.begin(), vec.end(), distinct.begin(), distinct.end(),
  std::inserter(duplicates, duplicates.end()));
  return duplicates;
}