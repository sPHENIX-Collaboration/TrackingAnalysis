#include "MvtxMatchingEfficiency.h"

#include <g4detectors/PHG4CylinderGeomContainer.h>

#include <trackbase/ActsGeometry.h>
#include <trackbase/MvtxDefs.h>
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


#include <fun4all/Fun4AllHistoManager.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/SubsysReco.h>

#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

#include <TH1.h>
#include <TH1D.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TFile.h>
#include <algorithm>

//____________________________________________________________________________..
MvtxMatchingEfficiency::MvtxMatchingEfficiency(const std::string &name)
  : SubsysReco(name)
{
}

//____________________________________________________________________________..
int MvtxMatchingEfficiency::InitRun(PHCompositeNode */*topNode*/)
{
  // print configuration
  std::cout << "MvtxMatchingEfficiency::InitRun "<< std::endl;

  h_silicon_match[0] = new TH1D("h_intt_2_mvtx_111","h_intt_2_mvtx_111",40,0,5);
  h_silicon_match[1] = new TH1D("h_intt_2_mvtx_110","h_intt_2_mvtx_110",40,0,5);
  h_silicon_match[2] = new TH1D("h_intt_2_mvtx_101","h_intt_2_mvtx_101",40,0,5);
  h_silicon_match[3] = new TH1D("h_intt_2_mvtx_011","h_intt_2_mvtx_011",40,0,5);
  h_silicon_match[4] = new TH1D("h_intt_2_mvtx_2","h_intt_2_mvtx_2",40,0,5);

  h_silicon_match_etaphi[0] = new TH2D("h_intt_2_mvtx_111_etaphi","h_intt_2_mvtx_111_etaphi",44,-1.1,1.1,64,-3.2,3.2);
  h_silicon_match_etaphi[1] = new TH2D("h_intt_2_mvtx_110_etaphi","h_intt_2_mvtx_110_etaphi",44,-1.1,1.1,64,-3.2,3.2);
  h_silicon_match_etaphi[2] = new TH2D("h_intt_2_mvtx_101_etaphi","h_intt_2_mvtx_101_etaphi",44,-1.1,1.1,64,-3.2,3.2);
  h_silicon_match_etaphi[3] = new TH2D("h_intt_2_mvtx_011_etaphi","h_intt_2_mvtx_011_etaphi",44,-1.1,1.1,64,-3.2,3.2);
  h_silicon_match_etaphi[4] = new TH2D("h_intt_2_mvtx_2_etaphi","h_intt_2_mvtx_2_etaphi",44,-1.1,1.1,64,-3.2,3.2);

  h_seed_match_etaphi[0] = new TH2D("h_intt_2_mvtx_111_etaphi_seed","h_intt_2_mvtx_111_etaphi_seed",44,-1.1,1.1,64,-3.2,3.2);
  h_seed_match_etaphi[1] = new TH2D("h_intt_2_mvtx_110_etaphi_seed","h_intt_2_mvtx_110_etaphi_seed",44,-1.1,1.1,64,-3.2,3.2);
  h_seed_match_etaphi[2] = new TH2D("h_intt_2_mvtx_101_etaphi_seed","h_intt_2_mvtx_101_etaphi_seed",44,-1.1,1.1,64,-3.2,3.2);
  h_seed_match_etaphi[3] = new TH2D("h_intt_2_mvtx_011_etaphi_seed","h_intt_2_mvtx_011_etaphi_seed",44,-1.1,1.1,64,-3.2,3.2);
  h_seed_match_etaphi[4] = new TH2D("h_intt_2_mvtx_2_etaphi_seed","h_intt_2_mvtx_2_etaphi_seed",44,-1.1,1.1,64,-3.2,3.2);

  h_seed_match = new TH1I("h_intt_2_mvtx_X","h_intt_2_mvtx_X",5,0,5);

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int MvtxMatchingEfficiency::process_event(PHCompositeNode *topNode)
{
  ievent++;
  //std::cout<<"processing event "<<ievent<<std::endl;
  
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

  //std::cout<<"SvtxTrackMap"<<std::endl;

  for (auto it = trackmap->begin(); it != trackmap->end(); ++it) {
    //number of cluster per subsystem
    int n_mvtx_hits = 0;
    int n_intt_hits = 0;
    int n_tpc_hits = 0;
    //fired mvtx layers 0 - innermost , 2 - outermost
    bool mvtx_l[3] = {false,false,false};
    for (const auto& ckey : TrackAnalysisUtils::get_cluster_keys((*it).second)){
      switch (TrkrDefs::getTrkrId(ckey)){
        case TrkrDefs::mvtxId:
          n_mvtx_hits++;
          mvtx_l[static_cast<int>(TrkrDefs::getLayer(ckey))]=true;
          break;
        case TrkrDefs::inttId:
          n_intt_hits++;
          break;
        case TrkrDefs::tpcId:
          n_tpc_hits++;
          break;
      }
    }

    //std::cout << "mvtx " << n_mvtx_hits << " " << mvtx_l[0] << " " << mvtx_l[1] << " " << mvtx_l[2] << " intt " << n_intt_hits << " tpc " << n_tpc_hits << std::endl;
    //std::cout<<std::count_if(std::begin(mvtx_l), std::end(mvtx_l), [](bool i){ return i==true; })<<std::endl;
    //if(n_intt_hits >=2 && std::count_if(std::begin(mvtx_l), std::end(mvtx_l), [](bool i){ return i==true; })>= 2) std::cout<<"True"<<std::endl;

    // 3 MVTX layers fired
    if(n_intt_hits >=2 && mvtx_l[0] == true && mvtx_l[1] == true && mvtx_l[2] == true){
       h_silicon_match[0]->Fill((*it).second->get_pt());
       h_silicon_match_etaphi[0]->Fill((*it).second->get_eta(),(*it).second->get_phi());
    }
    // Outermost MVTX missing
    if(n_intt_hits >=2 && mvtx_l[0] == true && mvtx_l[1] == true && mvtx_l[2] == false){
      h_silicon_match[1]->Fill((*it).second->get_pt());
      h_silicon_match_etaphi[1]->Fill((*it).second->get_eta(),(*it).second->get_phi());
    } 
    // Middle MVTX missing
    if(n_intt_hits >=2 && mvtx_l[0] == true && mvtx_l[1] == false && mvtx_l[2] == true){
      h_silicon_match[2]->Fill((*it).second->get_pt());
      h_silicon_match_etaphi[2]->Fill((*it).second->get_eta(),(*it).second->get_phi());
    } 
    // Innermost missing
    if(n_intt_hits >=2 && mvtx_l[0] == false && mvtx_l[1] == true && mvtx_l[2] == true){
      h_silicon_match[3]->Fill((*it).second->get_pt());
      h_silicon_match_etaphi[3]->Fill((*it).second->get_eta(),(*it).second->get_phi());
    } 
    // any 2 MVTX layers fired
    if(n_intt_hits >=2 && std::count_if(std::begin(mvtx_l), std::end(mvtx_l), [](bool i){ return i==true; })>= 2){
      h_silicon_match[4]->Fill((*it).second->get_pt());
      h_silicon_match_etaphi[4]->Fill((*it).second->get_eta(),(*it).second->get_phi());
    } 
}

//std::cout<<"SiliconTrackSeedContainer"<<std::endl;
for (auto it = silicon_track_map->begin(); it != silicon_track_map->end(); ++it) {
  if(*it == nullptr) continue;
  int n_mvtx_hits = 0;
  int n_intt_hits = 0;
  bool mvtx_l[3] = {false,false,false};
  for (auto it2 = (*it)->begin_cluster_keys(); it2 != (*it)->end_cluster_keys(); ++it2) {
    switch (TrkrDefs::getTrkrId(*it2)){
      case TrkrDefs::mvtxId:
        n_mvtx_hits++;
        mvtx_l[static_cast<int>(TrkrDefs::getLayer(*it2))]=true;
        break;
      case TrkrDefs::inttId:
        n_intt_hits++;
        break;
    }
  }
  // 3 MVTX layers fired
  if(n_intt_hits >=2 && mvtx_l[0] == true && mvtx_l[1] == true && mvtx_l[2] == true){
    h_seed_match->Fill(0.5);
    h_seed_match_etaphi[0]->Fill((*it)->get_eta(),(*it)->get_phi());
  } 
  // Outermost MVTX missing
  if(n_intt_hits >=2 && mvtx_l[0] == true && mvtx_l[1] == true && mvtx_l[2] == false){
    h_seed_match->Fill(1.5);
    h_seed_match_etaphi[1]->Fill((*it)->get_eta(),(*it)->get_phi());
  } 
  // Middle MVTX missing
  if(n_intt_hits >=2 && mvtx_l[0] == true && mvtx_l[1] == false && mvtx_l[2] == true){
    h_seed_match->Fill(2.5);
    h_seed_match_etaphi[2]->Fill((*it)->get_eta(),(*it)->get_phi());
  } 
  // Innermost missing
  if(n_intt_hits >=2 && mvtx_l[0] == false && mvtx_l[1] == true && mvtx_l[2] == true){
    h_seed_match->Fill(3.5);
    h_seed_match_etaphi[3]->Fill((*it)->get_eta(),(*it)->get_phi());
  } 
  // any 2 MVTX layers fired
  if(n_intt_hits >=2 && std::count_if(std::begin(mvtx_l), std::end(mvtx_l), [](bool i){ return i==true; })>= 2){
    h_seed_match->Fill(4.5);
    h_seed_match_etaphi[4]->Fill((*it)->get_eta(),(*it)->get_phi());
  } 
  //std::cout<<"mvtx "<<n_mvtx_hits<<" intt "<<n_intt_hits<<std::endl;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int MvtxMatchingEfficiency::EndRun(const int /*runnumber*/)
{ 
  TFile *root_out = new TFile("MVTX_ME.root","RECREATE");

  h_silicon_match[0]->Write();
  h_silicon_match[1]->Write();
  h_silicon_match[2]->Write();
  h_silicon_match[3]->Write();
  h_silicon_match[4]->Write();
  h_seed_match->Write();
  for(int i = 0; i<5;i++){
    h_silicon_match_etaphi[i]->Write();
    h_seed_match_etaphi[i]->Write();
  }
  

  root_out->Close();
  
  return Fun4AllReturnCodes::EVENT_OK; 
}


