#include "PHModuleEdgeCorresctions.h"


#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllServer.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHDataNode.h>
#include <phool/PHNode.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHObject.h>
#include <phool/PHTimer.h>
#include <phool/getClass.h>
#include <phool/phool.h>

#include <tpc/TpcDistortionCorrectionContainer.h>
#include <tpc/TpcGlobalPositionWrapper.h>

#include <trackbase/TpcDefs.h>
#include <trackbase/TrkrCluster.h>
#include <trackbase/TrkrClusterContainer.h>
#include <trackbase_historic/SvtxTrack.h>
#include <trackbase_historic/SvtxTrackMap.h>

#include <g4detectors/PHG4CylinderGeomContainer.h>
#include <g4detectors/PHG4TpcCylinderGeom.h>
#include <g4detectors/PHG4TpcCylinderGeomContainer.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include <TCanvas.h>
#include <cmath>
#include <string>

#include <iostream>
#include <sstream>

namespace
{

    // square
    template <class T>
    inline constexpr T square(const T& x)
    {
        return x * x;
    }

    // radius
    template <class T>
    inline T r(const T& x, const T& y)
    {
        return std::sqrt(square(x) + square(y));
    }

    //sector edge from in the range -pi to pi
    inline constexpr double phi_edge(int isec)
    {
        return M_PI - (2*M_PI/TpcDefs::NSectors)/2 - isec*(2*M_PI/TpcDefs::NSectors);
    }
    //sector phi edge in the range from 0 to 2pi
    inline constexpr double phi_edge_pos(int isec)
    {
        return ( isec < 6 ) ? (M_PI - (2*M_PI/TpcDefs::NSectors)/2 - isec*(2*M_PI/TpcDefs::NSectors)) : (M_PI - (2*M_PI/TpcDefs::NSectors)/2 - isec*(2*M_PI/TpcDefs::NSectors) + 2*M_PI);
    }
    //! get cluster keys from a given track
    std::vector<TrkrDefs::cluskey> get_cluster_keys(SvtxTrack* track)
    {
        std::vector<TrkrDefs::cluskey> out;
        for (const auto& seed : {track->get_silicon_seed(), track->get_tpc_seed()})
        {
            if (seed)
            {
                std::copy(seed->begin_cluster_keys(), seed->end_cluster_keys(), std::back_inserter(out));
            }
        }
        return out;
    }


}  // namespace

//___________________________________________________________________________________
PHModuleEdgeCorresctions::PHModuleEdgeCorresctions(const std::string& name)
        : SubsysReco(name)
{
    std::cout << "PHModuleEdgeCorresctions::PHModuleEdgeCorresctions" <<std::endl;
}


//___________________________________________________________________________________
int PHModuleEdgeCorresctions::Init(PHCompositeNode* /*topNode*/)
{
    // configuration printout
    std::cout << "PHModuleEdgeCorresctions::Init - m_minPt: " << m_minPt << " GeV/c" << std::endl;
    std::cout << "PHModuleEdgeCorresctions::Init - m_maxQuality: " << m_maxQuality << std::endl;
    std::cout << "PHModuleEdgeCorresctions::Init - m_minTpcClusters: " << m_minTpcClusters << std::endl;
    std::cout << "PHModuleEdgeCorresctions::Init - m_maxResidual: " << m_maxResidual << std::endl;

    // reset counters
    m_total_tracks = 0;
    m_accepted_tracks = 0;

    m_total_clusters = 0;
    m_accepted_clusters = 0;

    outfile = new TFile(m_outputfile.c_str(), "RECREATE");
    m_tree = new TTree("residualtree", "A tree with TPC track, cluster, and state basic info");
    m_tree->Branch("quality", &m_quality, "m_quality/F");
    m_tree->Branch("ntpc", &m_ntpc, "m_ntpc/I");
    m_tree->Branch("clussector", &m_clsector);
    m_tree->Branch("clusside", &m_clside);
    m_tree->Branch("clusgx", &m_clusgx);
    m_tree->Branch("clusgy", &m_clusgy);
    m_tree->Branch("clusgz", &m_clusgz);
    m_tree->Branch("clusphi", &m_clusphi);
    m_tree->Branch("stategx", &m_stategx);
    m_tree->Branch("stategy", &m_stategy);
    m_tree->Branch("xresidual", &m_xresidual);

    m_tree->Branch("pt", &m_pt, "m_pt/F");
    m_tree->Branch("eta", &m_eta, "m_eta/F");
    m_tree->Branch("phi", &m_phi, "m_phi/F");
    m_tree->Branch("cluslayer", &m_cluslayer);

    snprintf(side_name[0], sizeof(side_name[0]), "posz");
    snprintf(side_name[1], sizeof(side_name[1]), "negz");


    for(int j=0; j<NSides; j++)
    {

        snprintf(name_tr_all[j], sizeof(name_tr_all[j]), "all_tracks_side_%d",j);
        all_tracks[j] =new TH2F(name_tr_all[j], name_tr_all[j],1000,-81,81,1000,-81,81);

        for(int i=0; i<NSectors; i++)
        {
            snprintf(name_tr[j][i], sizeof(name_tr[j][i]), "tracks_side_%d_sector_%d",j,i);
            tracks[j][i] =new TH2F(name_tr[j][i], name_tr[j][i],1000,-81,81,1000,-81,81);


            snprintf(name_res[j][i], sizeof(name_res[j][i]), "rphi_residual_side_%d_sector_%d",j,i);
            snprintf(title_res[j][i], sizeof(title_res[j][i]), "r#phi residual, Sector %d, ", i);
            strcat(title_res[j][i], side_name[j]);
            rphiresidual[j][i] = new TH2F(name_res[j][i],title_res[j][i],58,0,57,200,-1.05,1.05);

            snprintf(name_mean[j][i], sizeof(name_mean[j][i]), "rphi_residual_mean_side_%d_sector_%d",j,i);
            snprintf(title_mean[j][i], sizeof(title_mean[j][i]), "Mean r#phi residual, Sector %d, ",i);
            strcat(title_mean[j][i], side_name[j]);

        }
    }
    if(m_DoResidualFit){
        fit_r3 = new TF1("fit_r3","pol3",  tpc_frame_r3_inner,  tpc_frame_r3_outer);
        fit_r2 = new TF1("fit_r2","pol3",  tpc_frame_r2_inner,  tpc_frame_r2_outer);
        fit_r1 = new TF1("fit_r1","pol3",  tpc_frame_r1_inner,  tpc_frame_r1_outer);
    }





    std::cout << "PHModuleEdgeCorresctions::Init - EVENT_OK " << std::endl;

    return Fun4AllReturnCodes::EVENT_OK;
}

//___________________________________________________________________________________
int PHModuleEdgeCorresctions::InitRun(PHCompositeNode* topNode)
{
    std::cout << "PHModuleEdgeCorresctions::InitRun " << std::endl;
    if (getNodes(topNode) != Fun4AllReturnCodes::EVENT_OK)
    {
        std::cout << "PHModuleEdgeCorresctions::InitRun - Abort Event:: getNodes - Event not ok " << std::endl;
        return Fun4AllReturnCodes::ABORTEVENT;
    }

    if (createNodes(topNode) != Fun4AllReturnCodes::EVENT_OK)
    {
        std::cout << "PHModuleEdgeCorresctions::InitRun - Abort Event:: createNodes - Event not ok " << std::endl;
        return Fun4AllReturnCodes::ABORTEVENT;
    }
    std::cout << "PHModuleEdgeCorresctions::InitRun - EVENT_OK " << std::endl;

    auto tpccellgeo = findNode::getClass<PHG4TpcCylinderGeomContainer>(topNode, "CYLINDERCELLGEOM_SVTX");
    m_clusterMover.initialize_geometry(tpccellgeo);
    m_clusterMover.set_verbosity(0);
    Fun4AllServer* se = Fun4AllServer::instance();
    m_runnumber = se->RunNumber();

    return Fun4AllReturnCodes::EVENT_OK;
}

//___________________________________________________________________________________
int PHModuleEdgeCorresctions::process_event(PHCompositeNode* topNode)
{
    std::cout << "PHModuleEdgeCorresctions::process_event - Event " <<m_event<< std::endl;
    const auto returnVal = processTracks(topNode);
    ++m_event;

    return returnVal;
}

//___________________________________________________________________________________
int PHModuleEdgeCorresctions::End(PHCompositeNode* /*topNode*/)
{
    std::cout << "PHModuleEdgeCorresctions::End - writing residuals to " << m_outputfile << std::endl;
    outfile->cd();
    m_tree->Write();

    for (int side =0; side<2; side++)
    {
        all_tracks[side]->Write();
        for (int sector = 0; sector<NSectors; sector++){
            tracks[side][sector]->Write();
            rphiresidual[side][sector]->Write();

            char slice_name[200];
            strcpy(slice_name,name_res[side][sector]);
            strcat(slice_name,"_1");
            rphiresidual[side][sector]->FitSlicesY();
            TH1F *rphiresidual_1=(TH1F*)gDirectory->Get(slice_name);
            rphiresidual_mean[side][sector]=new TH1F(*rphiresidual_1);
            if (m_DoResidualFit){
                rphiresidual_mean[side][sector]->Fit("fit_r1", "R+");
                rphiresidual_mean[side][sector]->Fit("fit_r2", "R+");
                rphiresidual_mean[side][sector]->Fit("fit_r3", "R+");
            }
            rphiresidual_mean[side][sector]->Write();
        }
    }

    if (m_CreateCorrectionMap ==true ){
      std::cout << "PHModuleEdgeCorresctions::End - writing correction map to  " << m_outputfile_corr << std::endl;
      if (m_outputfile_corr_ex!="")
      {

         TFile *file_existing_corr = TFile::Open(m_outputfile_corr_ex.c_str());
         if(file_existing_corr)
         {
              std::cout<<"Read from file:  "<<m_outputfile_corr_ex<<std::endl;
              PrevhIntDistortionP_posz = (TH2F*)(file_existing_corr->Get("hIntDistortionP_posz"));   
              PrevhIntDistortionP_negz = (TH2F*)(file_existing_corr->Get("hIntDistortionP_negz"));  
         }else{
             std::cout << "There is no previous iteration of the correction map" << std::endl;
         }
      }
            outfile_corr = new TFile(m_outputfile_corr.c_str(), "RECREATE");
            outfile_corr->cd();
            hIntDistortionR_posz = new TH2F("hIntDistortionR_posz", "R Distortion at (phi,r);phi;r", nph, pih, pfh, nrh, rih, rfh);
            hIntDistortionP_posz = new TH2F("hIntDistortionP_posz", "P Distortion at (phi,r);phi;r", nph, pih, pfh, nrh, rih, rfh);
            hIntDistortionZ_posz = new TH2F("hIntDistortionZ_posz", "Z Distortion at (phi,r);phi;r", nph, pih, pfh, nrh, rih, rfh);

            hIntDistortionR_negz = new TH2F("hIntDistortionR_negz", "R Distortion at (phi,r);phi;r", nph, pih, pfh, nrh, rih, rfh);
            hIntDistortionP_negz = new TH2F("hIntDistortionP_negz", "P Distortion at (phi,r);phi;r", nph, pih, pfh, nrh, rih, rfh);
            hIntDistortionZ_negz = new TH2F("hIntDistortionZ_negz", "Z Distortion at (phi,r);phi;r", nph, pih, pfh, nrh, rih, rfh);      

            FillCorrectionMap(rphiresidual_mean);

            hIntDistortionR_posz->Write();
            hIntDistortionP_posz->Write();
            hIntDistortionZ_posz->Write();
            hIntDistortionR_negz->Write();
            hIntDistortionP_negz->Write();
            hIntDistortionZ_negz->Write();
       
     
     }
    //    outfile->Close();

    // print counters
    std::cout
            << "PHModuleEdgeCorresctions::End -"
            << " track statistics total: " << m_total_tracks
            << " accepted: " << m_accepted_tracks
            << " fraction: " << 100. * m_accepted_tracks / m_total_tracks << "%"
            << std::endl;

    std::cout
            << "PHModuleEdgeCorresctions::End -"
            << " cluster statistics total: " << m_total_clusters
            << " accepted: " << m_accepted_clusters << " fraction: "
            << 100. * m_accepted_clusters / m_total_clusters << "%"
            << std::endl;

    return Fun4AllReturnCodes::EVENT_OK;
}

//___________________________________________________________________________________
int PHModuleEdgeCorresctions::processTracks(PHCompositeNode* /*topNode*/)
{
      if (Verbosity())
      {
         std::cout << "PHModuleEdgeCorresctions::processTracks - proto track size " << m_trackMap->size() << std::endl;
      }

    for (const auto& [trackKey, track] : *m_trackMap)
    {
        if (!track)
        {
          continue;
        }
        if (Verbosity() > 1)
        {
            std::cout << "PHModuleEdgeCorresctions::processTracks - Processing track key " << trackKey << std::endl;
        }

        ++m_total_tracks;
        if (checkTrack(track)){
            ++m_accepted_tracks;
            std::cout<<" Track Accepted "<<std::endl;
            m_pt = track->get_pt();
            m_eta = track->get_eta();
            m_phi = track->get_phi();
            m_quality = track->get_quality();
            processTrack(track);
        }
    }

    return Fun4AllReturnCodes::EVENT_OK;
}

//___________________________________________________________________________________
bool PHModuleEdgeCorresctions::checkTrack(SvtxTrack* track)
{
    if (Verbosity() > 2)
    {
        std::cout << "PHModuleEdgeCorresctions::checkTrack - pt: " << track->get_pt() << std::endl;
    }

    if (track->get_pt() < m_minPt)
    {
        std::cout<<" Track pt is lower then "<<m_minPt<<std::endl;
        return false;
    }

    if (Verbosity() > 2)
    {
        std::cout << "PHModuleEdgeCorresctions::checkTrack - quality: " << track->get_quality() << std::endl;
    }


    if (track->get_quality() > m_maxQuality)
    {
        std::cout<<" Track quality is higher then "<<m_maxQuality<<std::endl;
        return false;
    }

    // make sure cluster is from TPC
    m_ntpc=0;
    for (const auto& ckey : get_cluster_keys(track))
    {
        const auto detId = TrkrDefs::getTrkrId(ckey);
        if (detId == TrkrDefs::tpcId)
        {
             m_ntpc++;
        }else{
           return false;
        }
    }

   if (m_ntpc<m_minTpcClusters) return false;

    //  m_nhits = track->get_quality(count_clusters<TrkrDefs::tpcId>(cluster_keys));

    return true;
}




//_____________________________________________________________________________________________
void PHModuleEdgeCorresctions::processTrack(SvtxTrack* track)
{

     
  if (Verbosity() > 1)
  {
    std::cout << "PHModuleEdgeCorresctions::processTrack -"
              << " track momentum: " << track->get_p()
              << " position: " << Acts::Vector3(track->get_x(), track->get_y(), track->get_z())
              << std::endl;
  }


    // store crossing. It is used in calculating cluster's global position
    const auto crossing = track->get_crossing();
    assert(crossing != SHRT_MAX);

    clearClusterStateVectors();

    // Acts::Vector3 global;
    // get the fully corrected cluster global positions
    std::vector<std::pair<TrkrDefs::cluskey, Acts::Vector3>> global_raw;

    for (const auto& cluskey : get_cluster_keys(track))
    {
        // increment counter
        ++m_total_clusters;
        const auto cluster = clustermap->findCluster(cluskey);
        Acts::Vector3 global = geometry->getGlobalPosition(cluskey, cluster); 

      if (TrkrDefs::getTrkrId(cluskey) == TrkrDefs::tpcId)
      {
        global = TpcGlobalPositionWrapper::getGlobalPositionDistortionCorrected(cluskey, cluster, geometry, crossing, m_dcc_module_edge, m_dcc_static, m_dcc_average, m_dcc_fluctuation);
      }
        global_raw.emplace_back(std::make_pair(cluskey, global));


        // move the corrected cluster positions back to the original readout surface
        auto global_moved = m_clusterMover.processTrack(global_raw);

        Acts::Vector3 clusglob_moved(0, 0, 0);
        for (const auto& pair : global_moved)
        {
            auto thiskey = pair.first;
            clusglob_moved = pair.second;
            if (thiskey == cluskey)
            {
                break;
            }
        }

        SvtxTrackState* state = nullptr;
        // the track states from the Acts fit are fitted to fully corrected clusters, and are on the surface
        for (auto state_iter = track->begin_states();
             state_iter != track->end_states();
             ++state_iter)
        {
            SvtxTrackState* tstate = state_iter->second;
            auto stateckey = tstate->get_cluskey();
            if (stateckey == cluskey)
            {
                state = tstate;
                break;
            }
        }

  
        if (state)
        {


        m_clusgx.push_back(clusglob_moved.x());
        m_clusgy.push_back(clusglob_moved.y());
        m_clusgz.push_back(clusglob_moved.z());

        m_stategx.push_back(state->get_x());
        m_stategy.push_back(state->get_y());
        m_stategz.push_back(state->get_z());

        const double clusR = r(clusglob_moved.x(), clusglob_moved.y()); 
        const double clusPhi = std::atan2(clusglob_moved.y(),clusglob_moved.x());
        const double clusZ = clusglob_moved.z();

        const double stateR =  r(state->get_x(),state->get_y());
        const double statePhi = std::atan2(state->get_y(),state->get_x());

        m_clusphi.push_back(clusPhi);

        const double xresidual = clusR*clusPhi - stateR*statePhi;
        if (abs(xresidual)<1.){ 

        int side, sector=-9;
        side = (clusZ >= 0) ? 0 : 1;

        for (int s=0; s<NSectors;s++)
	{
	    if ((s==11)&&((clusPhi >= -M_PI && clusPhi < phi_edge(11) )||( clusPhi > phi_edge(0) && clusPhi < M_PI))) { 
                 sector = s;
            } else if((clusPhi > phi_edge(s+1) && clusPhi < phi_edge(s)))  {
                 sector = s;
            }
	}		
//        const int side = (int) TpcDefs::getSide(cluskey);
//        const int sector = (int) TpcDefs::getSectorId(cluskey);
        unsigned int layer = TrkrDefs::getLayer(cluskey);
        m_clsector.push_back(sector);
        m_clside.push_back(side);
        m_cluslayer.push_back(layer);


        m_xresidual.push_back(xresidual);
       
        if ( side>-1 &&sector>-1){
            all_tracks[side]->Fill(clusglob_moved.x(),clusglob_moved.y());
            tracks[side][sector]->Fill(clusglob_moved.x(), clusglob_moved.y());
            rphiresidual[side][sector]->Fill(layer, xresidual);
        }
        // increment number of accepted clusters
        ++m_accepted_clusters;
        }
        }//end of if state
    } //end loop clusters
    std::cout<<"FILL TREE "<<std::endl;
    m_tree->Fill();
}


//_______________________________________________________________________________________________________
void PHModuleEdgeCorresctions::FillCorrectionMap(TH1 *phiresmean[2][12])
{
    std::cout<<"PHModuleEdgeCorresctions::FillCorrectionMap - Filling and Writing Correction Map"<<std::endl;


    float partR=0, partP=0;
    float distortR=0, distortP_p=0, distortP_n=0, distortZ=0;

    float l_radius_low[48], l_radius_hi[48];
    for (int l=0; l<48; l++)
    {
        l_radius_low[l] = tpc_frame_inner[l/16] + ((double) (l + 7 - tpc_layer_inner[l/16])) * (double)((tpc_frame_outer[l/16]-tpc_frame_inner[l/16])/16.);
        l_radius_hi[l] = tpc_frame_inner[l/16] + ((double) (l + 1 + 7 - tpc_layer_inner[l/16])) * (double)((tpc_frame_outer[l/16]-tpc_frame_inner[l/16])/16.);
    }


    for( int ir = 0; ir < nrh; ir++)
    {
        partR = (ir + 0.5) * deltar + rih;

        distortP_p=0;
        distortP_n=0;

        for (int ip = 0; ip < nph; ip++)
        {
            partP = (ip + 0.5) * deltap + pih;
            for(int sector =0 ; sector<NSectors; sector++)
            {
            if ((partP > phi_edge_pos(sector + 1) && partP<phi_edge_pos(sector)) || ((sector==5)&&((partP > phi_edge_pos(6))||(partP<phi_edge_pos(5) ))))
            {
            for (int l=0; l<3*16; l++){
                if(partR>=l_radius_low[l] && partR < l_radius_hi[l]) {
                    int bin_r_p=0, bin_r_n=0;
                    bin_r_p =phiresmean[0][sector]->FindBin(l+7);
                    bin_r_n =phiresmean[1][sector]->FindBin(l+7);
                    distortP_p = (double)((double)(phiresmean[0][sector]->GetBinContent(bin_r_p))/partR);
                    distortP_n = (double)((double)(phiresmean[1][sector]->GetBinContent(bin_r_n))/partR);
                    if (m_outputfile_corr_ex!="")
                    {
                        int bin_map_r_p=0, bin_map_p_p=0, bin_map_r_n=0, bin_map_p_n;
                        bin_map_p_p =PrevhIntDistortionP_posz->GetXaxis()->FindBin(partP); 
                        bin_map_r_p =PrevhIntDistortionP_posz->GetYaxis()->FindBin(partR); 
                        bin_map_p_n =PrevhIntDistortionP_negz->GetXaxis()->FindBin(partP); 
                        bin_map_r_n =PrevhIntDistortionP_negz->GetYaxis()->FindBin(partR); 
                        distortP_p+=PrevhIntDistortionP_posz->GetBinContent(bin_map_p_p,bin_map_r_p);
		        distortP_n+=PrevhIntDistortionP_negz->GetBinContent(bin_map_p_n,bin_map_r_n);
                    } //if - Previous corr map
                } // if - check which layer we are in
            } //for  - loop by layers
            }//if - check which sector we are ini
            }//for - loop by sectors
            hIntDistortionR_posz->Fill(partP, partR, distortR);
            hIntDistortionP_posz->Fill(partP, partR, distortP_p);
            hIntDistortionZ_posz->Fill(partP, partR, distortZ);
            hIntDistortionR_negz->Fill(partP, partR, distortR);
            hIntDistortionP_negz->Fill(partP, partR, distortP_n);
            hIntDistortionZ_negz->Fill(partP, partR, distortZ);
        } // loop by phi
    } //loop by R

}

//_______________________________________________________________________________
int PHModuleEdgeCorresctions::createNodes(PHCompositeNode* /*topNode*/)
{
    return Fun4AllReturnCodes::EVENT_OK;
}

//_______________________________________________________________________________
int PHModuleEdgeCorresctions::getNodes(PHCompositeNode* topNode)
{
    clustermap = findNode::getClass<TrkrClusterContainer>(topNode, "TRKR_CLUSTER");
    if (!clustermap)
    {
        std::cout << PHWHERE << "No TRKR_CLUSTER node on node tree. Exiting." << std::endl;
        return Fun4AllReturnCodes::ABORTEVENT;
    }

    geometry = findNode::getClass<ActsGeometry>(topNode, "ActsGeometry");
    if (!geometry)
    {
        std::cout << "ActsTrackingGeometry not on node tree. Exiting." << std::endl;
        return Fun4AllReturnCodes::ABORTEVENT;
    }

    m_trackMap = findNode::getClass<SvtxTrackMap>(topNode, "SvtxTrackMap");
    if (!m_trackMap)
    {
        std::cout << PHWHERE << "SvtxTrackMap not on node tree. Exiting." << std::endl;
        return Fun4AllReturnCodes::ABORTEVENT;
    }

    // tpc distortion corrections
    m_dcc_module_edge = findNode::getClass<TpcDistortionCorrectionContainer>(topNode, "TpcDistortionCorrectionContainerModuleEdge");
    m_dcc_static = findNode::getClass<TpcDistortionCorrectionContainer>(topNode, "TpcDistortionCorrectionContainerStatic");
    m_dcc_average = findNode::getClass<TpcDistortionCorrectionContainer>(topNode, "TpcDistortionCorrectionContainerAverage");
    m_dcc_fluctuation = findNode::getClass<TpcDistortionCorrectionContainer>(topNode, "TpcDistortionCorrectionContainerFluctuation");

    return Fun4AllReturnCodes::EVENT_OK;
}


//____________________________________________________________________________..
void PHModuleEdgeCorresctions::clearClusterStateVectors()
{
    m_clusgr.clear();
    m_clusgx.clear();
    m_clusgy.clear();
    m_clusgz.clear();
    m_clusphi.clear();
    m_stategx.clear();
    m_stategy.clear();
    m_stategz.clear();
    m_xresidual.clear();
    m_clsector.clear();
    m_clside.clear();
    m_cluslayer.clear();
}


