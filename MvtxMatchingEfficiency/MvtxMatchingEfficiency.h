// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef QA_TRACKING_MVTXMATHINGEFFICIENCY_H_
#define QA_TRACKING_MVTXMATHINGEFFICIENCY_H_

#include <fun4all/SubsysReco.h>

#include <array>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <trackbase/TrkrDefs.h>

class ActsGeometry;
class TrkrHitSetContainer;
class TrkrClusterContainer;
class TrkrClusterHitAssoc;
class PHG4TpcCylinderGeomContainer;

class TH1;
class TH1D;
class TH2D;
class TH1I;
class TH2;
class TCanvas;
class TrackSeed;
class TTree;

class PHCompositeNode;

class MvtxMatchingEfficiency : public SubsysReco
{
 public:
 MvtxMatchingEfficiency(const std::string &name = "MvtxMatchingEfficiency",const std::string outputfilename = "out.root");

  ~MvtxMatchingEfficiency() override = default;

  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;
  int EndRun(const int runnumber) override;

  float calc_dedx(TrackSeed *tpcseed);
  std::set<TrkrDefs::cluskey> findDuplicates(std::vector<TrkrDefs::cluskey> vec);

  private:

  std::string m_outputFileName;

  float pt, eta, phi, frac_p_z, dEdx,residual_l0,residual_l1,residual_l2;
  int layers, states,nTPC;

  int ievent = 0;
  TH1D *h_silicon_match[5] = {nullptr};
  TH2D *h_silicon_match_etaphi[5] = {nullptr};
  TH1D *h_seed_match[5] = {nullptr};
  TH2D *h_seed_match_etaphi[5] = {nullptr};
  TH1I *h_status[2] = {nullptr};
  TH1I *h_INTT_time_delta[2] = {nullptr};

  TrkrClusterContainer *_cluster_map = nullptr;
  PHG4TpcCylinderGeomContainer *_geom_container{nullptr};
  ActsGeometry *m_tGeometry = nullptr;

  TTree *tree = nullptr;
};

#endif  // QA_TRACKING_MVTXMATHINGEFFICIENCY_H_
