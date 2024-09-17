#ifndef TRACKRECO_PHModuleEdgeCorresctions_H
#define TRACKRECO_PHModuleEdgeCorresctions_H

#include <fun4all/SubsysReco.h>
#include <tpc/TpcClusterMover.h>
#include <tpc/TpcClusterZCrossingCorrection.h>
#include <tpc/TpcDistortionCorrection.h>
#include <trackbase/ActsGeometry.h>
#include <trackbase/ClusterErrorPara.h>
#include <trackbase/TrkrDefs.h>
#include <trackbase_historic/ActsTransformations.h>


#include <memory>
#include <optional>

class PHCompositeNode;
class SvtxTrack;
class SvtxTrackMap;
class TrkrCluster;
class TrkrClusterContainer;

class TFile;
class TH1;
class TH2;
class TTree;
class TCanvas;
/**
 * This class takes preliminary fits from PHActsTrkFitter to the
 * silicon + MM clusters and calculates the residuals in the TPC
 * from that track fit. The TPC state has to be explicitly determined
 * here since the Acts::DirectNavigator does not visit the TPC states
 */
class PHModuleEdgeCorresctions : public SubsysReco
{
public:
    PHModuleEdgeCorresctions(const std::string &name = "PHModuleEdgeCorresctions");
    ~PHModuleEdgeCorresctions() override = default;

    int Init(PHCompositeNode *topNode) override;
    int InitRun(PHCompositeNode *topNode) override;
    int process_event(PHCompositeNode *topNode) override;
    int End(PHCompositeNode *topNode) override;

 
    void runnumber(const int run) { m_runnumber = run; }
    void segment(const int seg) { m_segment = seg; }

    /// track min pT
    void setMinPt(double value)
    {
        m_minPt = value;
    }
    /// track max quality
    void setMaxQuality(double value)
    {
        m_maxQuality = value;
    }
    /// cluster max rphi residual
    void setMaxRPhiResidual(double value)
    {
        m_maxResidual = value;
    }
    /// cluster min number required TPC clusters in track
    void setRequiredTpcClusters(int value)
    {
        m_minTpcClusters = value;
    }

    /// cluster min number required TPC clusters in track
    void setDoResidualFit(bool value)
    {
        m_DoResidualFit = value;
    }

    /// output file name for storing the space charge reconstruction matrices
    void setOutputfile(const std::string &outputfile)
    {
        m_outputfile = outputfile;
    }

    void CreateCorrectionMap(bool value)
    {
        m_CreateCorrectionMap = value;
    }

    void setCorrectionFile(const std::string &outputfile_corr)
    {
        m_outputfile_corr = outputfile_corr;
    }

    void setExistingCorrectionFile(const std::string &outputfile_corr_ex)
    {
        m_outputfile_corr_ex = outputfile_corr_ex;
    }

private:
    using BoundTrackParam =
            const Acts::BoundTrackParameters;


    int getNodes(PHCompositeNode *topNode);
    int createNodes(PHCompositeNode *topNode);

    Acts::Vector3 getGlobalPosition(TrkrDefs::cluskey, TrkrCluster *, short int crossing) const;

    int processTracks(PHCompositeNode *topNode);

    bool checkTrack(SvtxTrack *track);
    void processTrack(SvtxTrack *track);
    void FillCorrectionMap(TH1 * rphiresmean[2][12]);
    void clearClusterStateVectors();

    SvtxTrackMap *m_trackMap = nullptr;
    ActsGeometry *geometry = nullptr;
    TrkrClusterContainer *clustermap = nullptr;
    TpcClusterMover m_clusterMover;
    // crossing z correction
    TpcClusterZCrossingCorrection m_clusterCrossingCorrection;

    // distortion corrections
    TpcDistortionCorrectionContainer *m_dcc_module_edge = nullptr;
    TpcDistortionCorrectionContainer *m_dcc_static = nullptr;
    TpcDistortionCorrectionContainer *m_dcc_average = nullptr;
    TpcDistortionCorrectionContainer *m_dcc_fluctuation = nullptr;

    /// tpc distortion correction utility class
    TpcDistortionCorrection m_distortionCorrection;
    int m_ntpc;//std::numeric_limits<int>::quiet_NaN();
    float m_pt;//std::numeric_limits<float>::quiet_NaN();
    float m_eta;//std::numeric_limits<float>::quiet_NaN();
    float m_phi;//std::numeric_limits<float>::quiet_NaN();
    float m_quality;//std::numeric_limits<float>::quiet_NaN();


    static constexpr int m_minClusCount = 10;
 
    float tpc_frame_outer[3] = {40.249, 57.476, 75.911}; 
    float tpc_frame_inner[3] = {31.105,41.153, 58.367} ; 
    int tpc_layer_outer[3] = {22, 38, 54}; 
    int tpc_layer_inner[3] = {7, 23, 39} ; 



   ///Correction Map dimentions
    int nph = 42*4, nrh = 28*8;
    float pih = -0.157080, pfh = 6.440265;
    float rih = 17.769230, rfh = 80.230766; 
    float deltar = (rfh - rih)/nrh;
    float deltap = (pfh - pih)/nph;  


    std::vector<float> m_clusgx;
    std::vector<float> m_clusgy;
    std::vector<float> m_clusgz;
    std::vector<float> m_clusphi;
    std::vector<float> m_stategx;
    std::vector<float> m_stategy;
    std::vector<float> m_stategz;
    std::vector<float> m_xresidual;
    std::vector<float> m_clusgr;
    std::vector<int> m_clsector;
    std::vector<int> m_clside;
    std::vector<int> m_cluslayer;

    static constexpr int NSides = 2;
    static constexpr int NSectors = 12;

    char side_name[NSides][200];

    TH2 *all_tracks[NSides]; 
    char name_tr_all[NSides][200];

    TH2 *tracks[NSides][NSectors]; 
    char name_tr[NSides][NSectors][200];


    TH2 *rphiresidual[NSides][NSectors]; 
    char name_res[NSides][NSectors][200];
    char title_res[NSides][NSectors][200];

    TH1 *rphiresidual_mean[NSides][NSectors];
    char name_mean[NSides][NSectors][200];
    char title_mean[NSides][NSectors][200];

  TH2 *hIntDistortionR_posz;
  TH2 *hIntDistortionP_posz;
  TH2 *hIntDistortionZ_posz;

  TH2 *hIntDistortionR_negz;
  TH2 *hIntDistortionP_negz;
  TH2 *hIntDistortionZ_negz;


  TH2 *PrevhIntDistortionP_posz=nullptr;
  TH2 *PrevhIntDistortionP_negz=nullptr; 


  float tpc_frame_r3_outer = 54.;//75.911; 
  float tpc_frame_r3_inner = 39.;//58.367; 
  float tpc_frame_r2_outer = 38.;//57.476; 
  float tpc_frame_r2_inner = 23.;//41.153; 
  float tpc_frame_r1_outer = 22.;//40.249; 
  float tpc_frame_r1_inner = 7.;//21.783; 

  TF1 *fit_r3;
  TF1 *fit_r2;
  TF1 *fit_r1;




    // TODO: check if needed
    int m_event = 0;

    int m_segment = std::numeric_limits<int>::quiet_NaN();
    int m_runnumber = std::numeric_limits<int>::quiet_NaN(); 

    double m_minPt = 0.5;
    double m_maxQuality = 100.0;
    double m_maxResidual = 1.0;
    int m_minTpcClusters = 30;
    bool m_DoResidualFit = true;
    bool m_CreateCorrectionMap = false; 
    bool m_SavePic = false; 
 
   /// output file
    std::string m_outputfile = "TpcModuleEdgeCorrections.root";
    std::string m_outputfile_corr = "ModuleEdgeCorrectionMap.root";
    std::string m_outputfile_corr_ex = "";
    TTree *m_tree = nullptr;
    TFile *outfile = nullptr;
    TFile *outfile_corr = nullptr;
    char outfile_savepic[256];
    ///@name counters
    //@{
    int m_total_tracks = 0;
    int m_accepted_tracks = 0;

    int m_total_clusters = 0;
    int m_accepted_clusters = 0;
    //@}
};

#endif

