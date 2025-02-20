// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef QA_TRACKING_MVTXMATHINGEFFICIENCY_H_
#define QA_TRACKING_MVTXMATHINGEFFICIENCY_H_

#include <fun4all/SubsysReco.h>

#include <array>
#include <map>
#include <set>
#include <string>

class ActsGeometry;
class TrkrHitSetContainer;
class TrkrClusterContainer;
class TrkrClusterHitAssoc;

class TH1;
class TH1D;
class TH2D;
class TH1I;
class TH2;
class TCanvas;

class PHCompositeNode;

class MvtxMatchingEfficiency : public SubsysReco
{
 public:
 MvtxMatchingEfficiency(const std::string &name = "MvtxMatchingEfficiency");

  ~MvtxMatchingEfficiency() override = default;

  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;
  int EndRun(const int runnumber) override;

  private:

  int ievent = 0;
  TH1I *h_seed_match  = nullptr;
  TH1D *h_silicon_match[5] = {nullptr};
  TH2D *h_silicon_match_etaphi[5] = {nullptr};
  TH2D *h_seed_match_etaphi[5] = {nullptr};
};

#endif  // QA_TRACKING_MVTXMATHINGEFFICIENCY_H_
