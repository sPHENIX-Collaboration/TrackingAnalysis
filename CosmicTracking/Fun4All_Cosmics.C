/*
 * This macro shows running the full event combining + tracking for 
 * cosmics running.. There are some analysis modules run at the end 
 * which package  hits, clusters, and clusters on tracks into trees 
 * for analysis.
 */

#include <G4_ActsGeom.C>
#include <G4_Magnet.C>
#include <GlobalVariables.C>
#include <QA.C>
#include <Trkr_Clustering.C>
#include <Trkr_Reco_Cosmics.C>
#include <Trkr_RecoInit.C>
#include <Trkr_TpcReadoutInit.C>

#include <ffamodules/CDBInterface.h>
#include <fun4allraw/Fun4AllStreamingInputManager.h>
#include <fun4allraw/InputManagerType.h>
#include <fun4allraw/SingleGl1PoolInput.h>
#include <fun4allraw/SingleInttPoolInput.h>
#include <fun4allraw/SingleMicromegasPoolInput.h>
#include <fun4allraw/SingleMvtxPoolInput.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllServer.h>

#include <phool/recoConsts.h>
#include <ffamodules/HeadReco.h>
#include <ffamodules/FlagHandler.h>
#include <ffamodules/SyncReco.h>

#include <fun4allraw/SingleTpcTimeFrameInput.h>

#include <cdbobjects/CDBTTree.h>

#include <trackingqa/InttClusterQA.h>

#include <trackingqa/MicromegasClusterQA.h>

#include <trackingqa/MvtxClusterQA.h>

#include <trackingqa/TpcClusterQA.h>
#include <trackingqa/TpcSeedsQA.h>

#include <trackingdiagnostics/TrackResiduals.h>
#include <trackingdiagnostics/TrkrNtuplizer.h>

#include <fun4all/Fun4AllUtils.h>

#include <stdio.h>

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


bool isGood(const string &infile);

void Fun4All_Cosmics(
    const int nEvents = 0,
    const int runnumber = 30177,
    const std::string &dir = ".",
    const std::string &outfilename = "cosmics",
    const std::string &input_gl1file = "gl1daq.list",
    const string &input_inttfile00 = "intt0.list",
    const string &input_inttfile01 = "intt1.list",
    const string &input_inttfile02 = "intt2.list",
    const string &input_inttfile03 = "intt3.list",
    const string &input_inttfile04 = "intt4.list",
    const string &input_inttfile05 = "intt5.list",
    const string &input_inttfile06 = "intt6.list",
    const string &input_inttfile07 = "intt7.list",
    const string &input_mvtxfile00 = "mvtx0.list",
    const string &input_mvtxfile01 = "mvtx1.list",
    const string &input_mvtxfile02 = "mvtx2.list",
    const string &input_mvtxfile03 = "mvtx3.list",
    const string &input_mvtxfile04 = "mvtx4.list",
    const string &input_mvtxfile05 = "mvtx5.list",
    const string &input_tpcfile00 = "tpc00.list",
    const string &input_tpcfile01 = "tpc01.list",
    const string &input_tpcfile02 = "tpc02.list",
    const string &input_tpcfile03 = "tpc03.list",
    const string &input_tpcfile04 = "tpc04.list",
    const string &input_tpcfile05 = "tpc05.list",
    const string &input_tpcfile06 = "tpc06.list",
    const string &input_tpcfile07 = "tpc07.list",
    const string &input_tpcfile08 = "tpc08.list",
    const string &input_tpcfile09 = "tpc09.list",
    const string &input_tpcfile10 = "tpc10.list",
    const string &input_tpcfile11 = "tpc11.list",
    const string &input_tpcfile12 = "tpc12.list",
    const string &input_tpcfile13 = "tpc13.list",
    const string &input_tpcfile14 = "tpc14.list",
    const string &input_tpcfile15 = "tpc15.list",
    const string &input_tpcfile16 = "tpc16.list",
    const string &input_tpcfile17 = "tpc17.list",
    const string &input_tpcfile18 = "tpc18.list",
    const string &input_tpcfile19 = "tpc19.list",
    const string &input_tpcfile20 = "tpc20.list",
    const string &input_tpcfile21 = "tpc21.list",
    const string &input_tpcfile22 = "tpc22.list",
    const string &input_tpcfile23 = "tpc23.list",
    const string &input_tpotfile = "tpot.list"
)
{
 vector<string> gl1_infile;
  gl1_infile.push_back(input_gl1file);

// MVTX
  vector<string> mvtx_infile;
  mvtx_infile.push_back(input_mvtxfile00);
  mvtx_infile.push_back(input_mvtxfile01);
  mvtx_infile.push_back(input_mvtxfile02);
  mvtx_infile.push_back(input_mvtxfile03);
  mvtx_infile.push_back(input_mvtxfile04);
  mvtx_infile.push_back(input_mvtxfile05);

// INTT
  vector<string> intt_infile;
  intt_infile.push_back(input_inttfile00);
  intt_infile.push_back(input_inttfile01);
  intt_infile.push_back(input_inttfile02);
  intt_infile.push_back(input_inttfile03);
  intt_infile.push_back(input_inttfile04);
  intt_infile.push_back(input_inttfile05);
  intt_infile.push_back(input_inttfile06);
  intt_infile.push_back(input_inttfile07);

  vector<string> tpc_infile;
  tpc_infile.push_back(input_tpcfile00);
  tpc_infile.push_back(input_tpcfile01);
  tpc_infile.push_back(input_tpcfile02);
  tpc_infile.push_back(input_tpcfile03);
  tpc_infile.push_back(input_tpcfile04);
  tpc_infile.push_back(input_tpcfile05);
  tpc_infile.push_back(input_tpcfile06);
  tpc_infile.push_back(input_tpcfile07);
  tpc_infile.push_back(input_tpcfile08);
  tpc_infile.push_back(input_tpcfile09);
  tpc_infile.push_back(input_tpcfile10);
  tpc_infile.push_back(input_tpcfile11);
  tpc_infile.push_back(input_tpcfile12);
  tpc_infile.push_back(input_tpcfile13);
  tpc_infile.push_back(input_tpcfile14);
  tpc_infile.push_back(input_tpcfile15);
  tpc_infile.push_back(input_tpcfile16);
  tpc_infile.push_back(input_tpcfile17);
  tpc_infile.push_back(input_tpcfile18);
  tpc_infile.push_back(input_tpcfile19);
  tpc_infile.push_back(input_tpcfile20);
  tpc_infile.push_back(input_tpcfile21);
  tpc_infile.push_back(input_tpcfile22);
  tpc_infile.push_back(input_tpcfile23);

// TPOT
  vector<string> tpot_infile;
  tpot_infile.push_back(input_tpotfile);

 auto se = Fun4AllServer::instance();
  se->Verbosity(2);
  auto rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", runnumber);

  Enable::CDB = true;
  rc->set_StringFlag("CDB_GLOBALTAG", "ProdA_2024");
  
  rc->set_uint64Flag("TIMESTAMP", runnumber);
  TRACKING::tpc_zero_supp = true;
  TRACKING::tpc_baseline_corr = true;


  TpcReadoutInit( runnumber );
  std::cout<< " run: " << runnumber
	   << " samples: " << TRACKING::reco_tpc_maxtime_sample
	   << " pre: " << TRACKING::reco_tpc_time_presample
	   << " vdrift: " << G4TPC::tpc_drift_velocity_reco
	   << std::endl;

 
  std::string geofile = CDBInterface::instance()->getUrl("Tracking_Geometry");

  Fun4AllRunNodeInputManager *ingeo = new Fun4AllRunNodeInputManager("GeoIn");
  ingeo->AddFile(geofile);
  se->registerInputManager(ingeo);

  CDBInterface *cdb = CDBInterface::instance();

  // can use for zero field
  //double fieldstrength = 0.01;
  //G4MAGNET::magfield_tracking = "0.01";
  double fieldstrength = std::numeric_limits<double>::quiet_NaN();
  bool ConstField = isConstantField(G4MAGNET::magfield_tracking,fieldstrength);

  if(ConstField && fieldstrength < 0.1)
  {
    G4MAGNET::magfield = "0.01";
    G4MAGNET::magfield_rescale = 1;
  }
  
  TrackingInit();

  int i= 0;
  int NumInputs = 0;
  Fun4AllStreamingInputManager *in = new Fun4AllStreamingInputManager("Comb");

  for (auto iter : gl1_infile)
  {
    if (isGood(iter))
    {
      SingleGl1PoolInput *gl1_sngl = new SingleGl1PoolInput("GL1_" + to_string(i));
      //    gl1_sngl->Verbosity(3);
      gl1_sngl->AddListFile(iter);
      in->registerStreamingInput(gl1_sngl, InputManagerType::GL1);
      i++;
    }
  }
  NumInputs += i;

  i = 0;
  for (auto iter : intt_infile)
  {
    if (isGood(iter))
    {
    SingleInttPoolInput *intt_sngl = new SingleInttPoolInput("INTT_" + to_string(i));
    //    intt_sngl->Verbosity(3);
    intt_sngl->SetNegativeBco(1);
    intt_sngl->SetBcoRange(2);
    intt_sngl->AddListFile(iter);
    in->registerStreamingInput(intt_sngl, InputManagerType::INTT);
    i++;
    }
  }
  NumInputs += i;

  i = 0;
  for (auto iter : mvtx_infile)
  {
    if (isGood(iter))
    {
    SingleMvtxPoolInput *mvtx_sngl = new SingleMvtxPoolInput("MVTX_" + to_string(i));
    //    mvtx_sngl->Verbosity(3);
    mvtx_sngl->AddListFile(iter);
    in->registerStreamingInput(mvtx_sngl, InputManagerType::MVTX);
    i++;
    }
  }
  NumInputs += i;

  i = 0;


  
  for (auto iter : tpc_infile)
  {
    if (isGood(iter))
    {

      SingleTpcTimeFrameInput *tpc_sngl = new SingleTpcTimeFrameInput("TPC_" + to_string(i));
//    tpc_sngl->Verbosity(2);
      //   tpc_sngl->DryRun();
      tpc_sngl->setHitContainerName("TPCRAWHIT");
      tpc_sngl->AddListFile(iter);
      in->registerStreamingInput(tpc_sngl, InputManagerType::TPC);
      i++;
    }
  }
  NumInputs += i;
    i = 0;
  for (auto iter : tpot_infile)
  {
    if (isGood(iter))
    {
    SingleMicromegasPoolInput *mm_sngl = new SingleMicromegasPoolInput("MICROMEGAS_" + to_string(i));
    //   sngl->Verbosity(3);
    mm_sngl->SetBcoRange(10);
    mm_sngl->SetNegativeBco(2);
    mm_sngl->SetBcoPoolSize(50);
    mm_sngl->AddListFile(iter);
    in->registerStreamingInput(mm_sngl, InputManagerType::MICROMEGAS);
    i++;
    }
  }
  NumInputs += i;

  // if there is no input manager this macro will still run - so just quit here
  if (NumInputs == 0)
  {
    std::cout << "no file lists no input manager registered, quitting" << std::endl;
    gSystem->Exit(1);
  }
  se->registerInputManager(in);

  SyncReco *sync = new SyncReco();
  se->registerSubsystem(sync);

  HeadReco *head = new HeadReco();
  se->registerSubsystem(head);

  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);

  
  Mvtx_HitUnpacking();
  Intt_HitUnpacking();
  Tpc_HitUnpacking();
  Micromegas_HitUnpacking();
  
  Mvtx_Clustering();
  Intt_Clustering();
  
  Tpc_LaserEventIdentifying();

  auto tpcclusterizer = new TpcClusterizer;
  tpcclusterizer->Verbosity(0);
  tpcclusterizer->set_do_hit_association(G4TPC::DO_HIT_ASSOCIATION);
  tpcclusterizer->set_rawdata_reco();
  se->registerSubsystem(tpcclusterizer);

  Micromegas_Clustering();

  Tracking_Reco_TrackSeed();

  TrackSeedTrackMapConverter *converter = new TrackSeedTrackMapConverter();
  // Default set to full SvtxTrackSeeds. Can be set to
  // SiliconTrackSeedContainer or TpcTrackSeedContainer
  converter->setTrackSeedName("SvtxTrackSeedContainer");
  converter->Verbosity(0);
  converter->cosmics();
  converter->setFieldMap(G4MAGNET::magfield_tracking);
  se->registerSubsystem(converter);

  TString residoutfile = outfilename + "_resid.root";
  std::string residstring(residoutfile.Data());

  auto resid = new TrackResiduals("TrackResiduals");
  resid->Verbosity(0);
  resid->outfileName(residstring);
  resid->alignment(false);
  resid->clusterTree();
  //resid->hitTree();
  resid->noEventTree();
  resid->convertSeeds(true);


  if(ConstField && fieldstrength < 0.1)
  {
    resid->zeroField();
  }
  se->registerSubsystem(resid);

  // Fun4AllOutputManager *out = new Fun4AllDstOutputManager("out", "/sphenix/tg/tg01/hf/jdosbo/tracking_development/onlineoffline/hitsets.root");
  // se->registerOutputManager(out);


  se->run(nEvents);
  se->End();
  se->PrintTimer();

  if (Enable::QA)
  {
    se->registerSubsystem(new InttClusterQA);
    se->registerSubsystem(new TpcClusterQA);
    se->registerSubsystem(new MicromegasClusterQA);
    se->registerSubsystem(new TpcSeedsQA);
   
  }
  
  TString qaname = outfilename + runnumber +"_qa.root";
  std::string qaOutputFileName(qaname.Data());
  QAHistManagerDef::saveQARootFile(qaOutputFileName);
  delete se;
  std::cout << "Finished" << std::endl;
  gSystem->Exit(0);
}


bool isGood(const string &infile)
{
  ifstream intest;
  intest.open(infile);
  bool goodfile = false;
  if (intest.is_open())
  {
    if (intest.peek() != std::ifstream::traits_type::eof()) // is it non zero?
    {
      goodfile = true;
    }
    intest.close();
  }
  return goodfile;
}
