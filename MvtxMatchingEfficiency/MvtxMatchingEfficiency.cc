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

#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

#include <TH1.h>
#include <TH1D.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TFile.h>
#include <algorithm>
#include <set>

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


 
  std::string matName[2] = {"wTPC","justSilicon"};
  for(int i = 0; i<2;i++){ 
    TString name = "h_status_"+matName[i];
    h_status[i]= new TH1I(name,name,6,0,6);
    h_status[i]->GetXaxis()->SetBinLabel(1,"Total");
    h_status[i]->GetXaxis()->SetBinLabel(2,"INTT >= 2");
    h_status[i]->GetXaxis()->SetBinLabel(3,"INTT same crossing");
    h_status[i]->GetXaxis()->SetBinLabel(4,"MVTX unique clusters");
    h_status[i]->GetXaxis()->SetBinLabel(5,"z < 10 cm");
    h_status[i]->GetXaxis()->SetBinLabel(6,"eta < 1.1");
  }

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

  _cluster_map = findNode::getClass<TrkrClusterContainer>(topNode, "CORRECTED_TRKR_CLUSTER");
  if (!_cluster_map)
  {
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


  //std::cout<<"SvtxTrackMap"<<std::endl;
  //fetch MVTX clusters
  std::vector<TrkrDefs::cluskey> cluster_vector;
  for (auto it = trackmap->begin(); it != trackmap->end(); ++it) {
    for (const auto& ckey : TrackAnalysisUtils::get_cluster_keys((*it).second)){
      switch (TrkrDefs::getTrkrId(ckey)){
        case TrkrDefs::mvtxId:
          cluster_vector.push_back(ckey);
          break;
      }
    }
  }

std::set<TrkrDefs::cluskey> duplicate_cluster_vector = findDuplicates(cluster_vector);
std::cout<<"mvtx seeds: "<<cluster_vector.size()<<" duplicate seeds "<<duplicate_cluster_vector.size()<<std::endl;
  
  for (auto it = trackmap->begin(); it != trackmap->end(); ++it) {
    //number of cluster per subsystem
    int n_mvtx_hits = 0;
    int n_intt_hits = 0;
    int n_tpc_hits = 0;
    //fired mvtx layers 0 - innermost , 2 - outermost
    bool mvtx_l[3] = {false,false,false};

    std::vector<float> intt_time;
    
    for (const auto& ckey : TrackAnalysisUtils::get_cluster_keys((*it).second)){
      switch (TrkrDefs::getTrkrId(ckey)){
        case TrkrDefs::mvtxId:
          n_mvtx_hits++;
          mvtx_l[static_cast<int>(TrkrDefs::getLayer(ckey))]=true;
          cluster_vector.push_back(ckey);
          break;
        case TrkrDefs::inttId:
         intt_time.push_back(InttDefs::getTimeBucketId(ckey));
          n_intt_hits++;
          break;
        case TrkrDefs::tpcId:
          n_tpc_hits++;
          break;
      }
    }

    

    h_status[0]->Fill(0.5);

    if(n_intt_hits < 2) continue;
    h_status[0]->Fill(1.5);

    bool good_intt_time = std::all_of(intt_time.begin() + 1, intt_time.end(), [&](int i){ return i == intt_time[0]; }); 
    
    intt_time.clear();

    if(good_intt_time == false) continue;
    h_status[0]->Fill(2.5);

    bool good_track = true;
    for (const auto& ckey : TrackAnalysisUtils::get_cluster_keys((*it).second)){
      if (duplicate_cluster_vector.find(ckey) != duplicate_cluster_vector.end()) {
        good_track = false;
      }
    }
    
    if(good_track == false) continue;

    h_status[0]->Fill(3.5);

    if(TMath::Abs((*it).second->get_z()) > 10 ) continue;

    h_status[0]->Fill(4.5);

    if(TMath::Abs((*it).second->get_eta()) > 1.1 ) continue;

    h_status[0]->Fill(5.5);

    //std::cout<<(good_intt_time==true?"good":"bad")<<std::endl;

    



    //float dedx = calc_dedx((*it).second->get_tpc_seed());
    
    //std::cout << "mvtx " << n_mvtx_hits << " " << mvtx_l[0] << " " << mvtx_l[1] << " " << mvtx_l[2] << " intt " << n_intt_hits << " tpc " << n_tpc_hits << " pt "<<(*it).second->get_pt()<<" dedx "<<dedx<<std::endl;
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



/*for (auto it = trackmap->begin(); it != trackmap->end(); ++it) {
  bool good_track = true;
  for (const auto& ckey : TrackAnalysisUtils::get_cluster_keys((*it).second)){
    if (duplicate_cluster_vector.find(ckey) != duplicate_cluster_vector.end()) {
      good_track = false;
  }
  }
  if (good_track == true){
    n_good_tracks++;
  }
}*/

//std::cout<<"all tracks: "<<trackmap->size()<<" good tracks: "<<n_good_tracks<<std::endl;
//for(auto v : cluster_vector){
//  TrkrDefs::printBits(v);
//}
//std::cout<<"duplicate mvtx seeds "<<std::endl;
//for(auto v : duplicate_cluster_vector){
//  TrkrDefs::printBits(v);
//}

cluster_vector.clear();
duplicate_cluster_vector.clear();


std::vector<TrkrDefs::cluskey> cluster_vector_seed;
  //fetch MVTX clusters
  for (auto it = silicon_track_map->begin(); it != silicon_track_map->end(); ++it) {
    if(*it == nullptr) continue;
    for (auto it2 = (*it)->begin_cluster_keys(); it2 != (*it)->end_cluster_keys(); ++it2) {
      switch (TrkrDefs::getTrkrId(*it2)){
        case TrkrDefs::mvtxId:
          cluster_vector_seed.push_back(*it2);
        break;
      }
    }
  }

std::set<TrkrDefs::cluskey> duplicate_cluster_vector_seed = findDuplicates(cluster_vector_seed);
std::cout<<"mvtx seed seeds: "<<cluster_vector_seed.size()<<" duplicate seed seeds "<<duplicate_cluster_vector_seed.size()<<std::endl;


//std::cout<<"SiliconTrackSeedContainer"<<std::endl;
for (auto it = silicon_track_map->begin(); it != silicon_track_map->end(); ++it) {

  std::vector<float> intt_time;


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
        intt_time.push_back(InttDefs::getTimeBucketId(*it2));
        n_intt_hits++;
        break;
    }
  }


  h_status[1]->Fill(0.5);

    if(n_intt_hits < 2) continue;
    h_status[1]->Fill(1.5);

    bool good_intt_time = std::all_of(intt_time.begin() + 1, intt_time.end(), [&](int i){ return i == intt_time[0]; }); 
    
    intt_time.clear();

    if(good_intt_time == false) continue;
    h_status[1]->Fill(2.5);

    bool good_track = true;
    for (auto it2 = (*it)->begin_cluster_keys(); it2 != (*it)->end_cluster_keys(); ++it2) {
      if (duplicate_cluster_vector_seed.find(*it2) != duplicate_cluster_vector_seed.end()) {
        good_track = false;
      }
    }
    
    if(good_track == false) continue;

    h_status[1]->Fill(3.5);

    if(TMath::Abs((*it)->get_Z0()) > 10 ) continue;

    h_status[1]->Fill(4.5);

    if(TMath::Abs((*it)->get_eta()) > 1.1 ) continue;

    h_status[1]->Fill(5.5);


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

  cluster_vector_seed.clear();
  duplicate_cluster_vector_seed.clear();



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

  h_status[0]->Write();
  h_status[1]->Write();
  

  root_out->Close();
  
  return Fun4AllReturnCodes::EVENT_OK; 
}


float MvtxMatchingEfficiency::calc_dedx(TrackSeed *tpcseed){

  std::vector<TrkrDefs::cluskey> clusterKeys;
  clusterKeys.insert(clusterKeys.end(), tpcseed->begin_cluster_keys(),
         tpcseed->end_cluster_keys());

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