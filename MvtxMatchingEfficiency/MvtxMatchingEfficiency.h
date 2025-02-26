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

class PHCompositeNode;

class MvtxMatchingEfficiency : public SubsysReco
{
 public:
 MvtxMatchingEfficiency(const std::string &name = "MvtxMatchingEfficiency");

  ~MvtxMatchingEfficiency() override = default;

  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;
  int EndRun(const int runnumber) override;

  float calc_dedx(TrackSeed *tpcseed);
  std::set<TrkrDefs::cluskey> findDuplicates(std::vector<TrkrDefs::cluskey> vec);

  private:

  int ievent = 0;
  TH1I *h_seed_match  = nullptr;
  TH1D *h_silicon_match[5] = {nullptr};
  TH2D *h_silicon_match_etaphi[5] = {nullptr};
  TH2D *h_seed_match_etaphi[5] = {nullptr};
  TH1I *h_status[2] = {nullptr};

  TrkrClusterContainer *_cluster_map = nullptr;
  PHG4TpcCylinderGeomContainer *_geom_container{nullptr};
};

#endif  // QA_TRACKING_MVTXMATHINGEFFICIENCY_H_
