#ifndef MACRO_FUN4ALLG4SPHENIX_C
#define MACRO_FUN4ALLG4SPHENIX_C

#include <GlobalVariables.C>

#include <G4Setup_sPHENIX.C> // use default

#include <DisplayOn.C>
#include <G4_Mbd.C>
#include <G4_CaloTrigger.C>
#include <G4_Centrality.C>
#include <G4_DSTReader.C>
#include <G4_Global.C>
#include <G4_HIJetReco.C>
#include <G4_Input.C>
#include <G4_Jets.C>
#include <G4_KFParticle.C>
#include <G4_ParticleFlow.C>
#include <G4_Production.C>
#include <G4_TopoClusterReco.C>

#include <Trkr_RecoInit.C>
#include <Trkr_Clustering.C>
#include <Trkr_LaserClustering.C>
#include <Trkr_Reco.C>
#include <Trkr_Eval.C>
#include <Trkr_QA.C>

#include <Trkr_Diagnostics.C>
#include <G4_User.C>
#include <QA.C>

#include <ffamodules/FlagHandler.h>
#include <ffamodules/HeadReco.h>
#include <ffamodules/SyncReco.h>
#include <ffamodules/CDBInterface.h>

#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllServer.h>

#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

#include <Rtypes.h> // resolves R__LOAD_LIBRARY for clang-tidy
#include <TROOT.h>

// Heavy-flavor simulation includes
#include <phpythia8/PHPy8ParticleTrigger.h>
#include <decayfinder/DecayFinder.h>
#include <hftrackefficiency/HFTrackEfficiency.h>

#include "HF_Selections.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)

// For HepMC Hijing
// try inputFile = /sphenix/sim/sim01/sphnxpro/sHijing_HepMC/sHijing_0-12fm.dat

int Fun4All_G4_sPHENIX(const int nEvents = 10,                                                                                                                         //
                       const string &inputFile = "https://www.phenix.bnl.gov/WWW/publish/phnxbld/sPHENIX/files/sPHENIX_G4Hits_sHijing_9-11fm_00000_00010.root",        //
                       const string &outputFile = "Kshort2pipi_G4sPHENIX.root",                                                                                               //
                       const string &embed_input_file = "https://www.phenix.bnl.gov/WWW/publish/phnxbld/sPHENIX/files/sPHENIX_G4Hits_sHijing_9-11fm_00000_00010.root", //
                       const int skip = 0,                                                                                                                             //
                       const string &outdir = "./",                                                                                                                     //
                       const int runNumber = 1,                                                                                                                        //
                       std::string processID = "0"                                                                                                                     //
)
{
    size_t pos = outputFile.find('_');
    std::string prefix;
    if (pos != std::string::npos)
    {
        prefix = outputFile.substr(0, pos);
    }
    else
    {
        prefix = outputFile;
    }

    std::cout << "Prefix: " << prefix << std::endl;

    Fun4AllServer *se = Fun4AllServer::instance();
    se->Verbosity(0);

    //Opt to print all random seed used for debugging reproducibility. Comment out to reduce stdout prints.
    PHRandomSeed::Verbosity(1);
    CDBInterface::instance()->Verbosity(1);

    // just if we set some flags somewhere in this macro
    recoConsts *rc = recoConsts::instance();
    // By default every random number generator uses
    // PHRandomSeed() which reads /dev/urandom to get its seed
    // if the RANDOMSEED flag is set its value is taken as seed
    // You can either set this to a random value using PHRandomSeed()
    // which will make all seeds identical (not sure what the point of
    // this would be:
    //  rc->set_IntFlag("RANDOMSEED",PHRandomSeed());
    // or set it to a fixed value so you can debug your code
    rc->set_IntFlag("RANDOMSEED", std::stoi(processID));

    //===============
    // Input options
    //===============
    // verbosity setting (applies to all input managers)
    Input::VERBOSITY = 0;
    // First enable the input generators
    // Either:
    // read previously generated g4-hits files, in this case it opens a DST and skips
    // the simulations step completely. The G4Setup macro is only loaded to get information
    // about the number of layers used for the cell reco code
    //  Input::READHITS = true;
    INPUTREADHITS::filename[0] = inputFile;
    // if you use a filelist
    // INPUTREADHITS::listfile[0] = inputFile;
    // Or:
    // Use particle generator
    // And
    // Further choose to embed newly simulated events to a previous simulation. Not compatible with `readhits = true`
    // In case embedding into a production output, please double check your G4Setup_sPHENIX.C and G4_*.C consistent with those in the production macro folder
    // E.g. /sphenix/sim//sim01/production/2016-07-21/single_particle/spacal2d/
    //  Input::EMBED = true;
    INPUTEMBED::filename[0] = embed_input_file;
    // if you use a filelist
    // INPUTEMBED::listfile[0] = embed_input_file;

    Input::SIMPLE = false;
    // Input::SIMPLE_NUMBER = 2; // if you need 2 of them
    // Input::SIMPLE_VERBOSITY = 1;

    // Enable this is emulating the nominal pp/pA/AA collision vertex distribution
    Input::BEAM_CONFIGURATION = Input::pp_COLLISION; // Input::AA_COLLISION (default), Input::pA_COLLISION, Input::pp_COLLISION

    //  Input::PYTHIA6 = true;

    Input::PYTHIA8 = true;

    //  Input::GUN = true;
    //  Input::GUN_NUMBER = 3; // if you need 3 of them
    // Input::GUN_VERBOSITY = 1;

    // Input::COSMIC = true;

    // D0 generator
    // Input::DZERO = false;
    // Input::DZERO_VERBOSITY = 0;
    // Lambda_c generator //Not ready yet
    // Input::LAMBDAC = false;
    // Input::LAMBDAC_VERBOSITY = 0;
    //  Upsilon generator
    // Input::UPSILON = true;
    // Input::UPSILON_NUMBER = 3; // if you need 3 of them
    // Input::UPSILON_VERBOSITY = 0;

    //  Input::HEPMC = true;
    INPUTHEPMC::filename = inputFile;
    //-----------------
    // Hijing options (symmetrize hijing, add flow, add fermi motion)
    //-----------------
    //  INPUTHEPMC::HIJINGFLIP = true;
    //  INPUTHEPMC::FLOW = true;
    //  INPUTHEPMC::FLOW_VERBOSITY = 3;
    //  INPUTHEPMC::FERMIMOTION = true;

    // Event pile up simulation with collision rate in Hz MB collisions.
    // Input::PILEUPRATE = 50e3; // 50 kHz for AuAu
    // Input::PILEUPRATE = 3e6; // 3MHz for pp

    // Enable this is emulating the nominal pp/pA/AA collision vertex distribution
    // for HepMC records (hijing, pythia8)
    //  Input::BEAM_CONFIGURATION = Input::AA_COLLISION; // for 2023 sims we want the AA geometry for no pileup sims
    //  Input::BEAM_CONFIGURATION = Input::pp_COLLISION; // for 2024 sims we want the pp geometry for no pileup sims
    //  Input::BEAM_CONFIGURATION = Input::pA_COLLISION; // for pAu sims we want the pA geometry for no pileup sims

    // Heavy-flavor simulation setup
    int particleID = 421;
    if (prefix == "Kshort2pipi")
    {
        PYTHIA8::config_file = "steeringCards/pythia8_K2pipi_Detroit.cfg";
        EVTGENDECAYER::DecayFile = "decFiles/K2pipi.DEC";
        particleID = 310;
    }
    else if (prefix == "D2Kpi")
    {
        PYTHIA8::config_file = "steeringCards/pythia8_D2Kpi_Detroit.cfg";
        EVTGENDECAYER::DecayFile = "decFiles/D2Kpi.DEC";
    }
    else if (prefix == "D2Kpipi") // D+
    {
      PYTHIA8::config_file = "steeringCards/pythia8_D2Kpipi_Detroit.cfg";
      EVTGENDECAYER::DecayFile = "decFiles/D2Kpipi.DEC";
      particleID = 411;
    }
    else if (prefix == "D2KKpi") // D_s
    {
      PYTHIA8::config_file = "steeringCards/pythia8_D2KKpi_Detroit.cfg";
      EVTGENDECAYER::DecayFile = "decFiles/D2KKpi.DEC";
      particleID = 431;
    }
    else if (prefix == "Lc2pKpi") // Lambda_c
    {
      PYTHIA8::config_file = "steeringCards/pythia8_Lc2pKpi_Detroit.cfg";
      EVTGENDECAYER::DecayFile = "decFiles/Lc2pKpi.DEC";
      particleID = 4122;
    }
    else if (prefix == "MinBias")
    {
        PYTHIA8::config_file = "steeringCards/pythia8_Detroit.cfg";
    }

    //-----------------
    // Initialize the selected Input/Event generation
    //-----------------
    // This creates the input generator(s)
    InputInit();

    //--------------
    // Set generator specific options
    //--------------
    // can only be set after InputInit() is called

    // Simple Input generator:
    // if you run more than one of these Input::SIMPLE_NUMBER > 1
    // add the settings for other with [1], next with [2]...
    if (Input::SIMPLE)
    {
        INPUTGENERATOR::SimpleEventGenerator[0]->add_particles("pi-", 5);
        if (Input::HEPMC || Input::EMBED)
        {
            INPUTGENERATOR::SimpleEventGenerator[0]->set_reuse_existing_vertex(true);
            INPUTGENERATOR::SimpleEventGenerator[0]->set_existing_vertex_offset_vector(0.0, 0.0, 0.0);
        }
        else
        {
            INPUTGENERATOR::SimpleEventGenerator[0]->set_vertex_distribution_function(PHG4SimpleEventGenerator::Gaus, PHG4SimpleEventGenerator::Gaus, PHG4SimpleEventGenerator::Gaus);
            INPUTGENERATOR::SimpleEventGenerator[0]->set_vertex_distribution_mean(0., 0., 0.);
            INPUTGENERATOR::SimpleEventGenerator[0]->set_vertex_distribution_width(0.01, 0.01, 5.);
        }
        INPUTGENERATOR::SimpleEventGenerator[0]->set_eta_range(-1, 1);
        INPUTGENERATOR::SimpleEventGenerator[0]->set_phi_range(-M_PI, M_PI);
        INPUTGENERATOR::SimpleEventGenerator[0]->set_pt_range(0.1, 20.);
    }

    // Upsilons
    // if you run more than one of these Input::UPSILON_NUMBER > 1
    // add the settings for other with [1], next with [2]...
    if (Input::UPSILON)
    {
        INPUTGENERATOR::VectorMesonGenerator[0]->add_decay_particles("e", 0);
        INPUTGENERATOR::VectorMesonGenerator[0]->set_rapidity_range(-1, 1);
        INPUTGENERATOR::VectorMesonGenerator[0]->set_pt_range(0., 10.);
        // Y species - select only one, last one wins
        INPUTGENERATOR::VectorMesonGenerator[0]->set_upsilon_1s();
        if (Input::HEPMC || Input::EMBED)
        {
            INPUTGENERATOR::VectorMesonGenerator[0]->set_reuse_existing_vertex(true);
            INPUTGENERATOR::VectorMesonGenerator[0]->set_existing_vertex_offset_vector(0.0, 0.0, 0.0);
        }
    }

    // particle gun
    // if you run more than one of these Input::GUN_NUMBER > 1
    // add the settings for other with [1], next with [2]...
    if (Input::GUN)
    {
        INPUTGENERATOR::Gun[0]->AddParticle("pi-", 0, 1, 0);
        INPUTGENERATOR::Gun[0]->set_vtx(0, 0, 0);
    }

    // pythia6
    if (Input::PYTHIA6)
    {
        //! Nominal collision geometry is selected by Input::BEAM_CONFIGURATION
        Input::ApplysPHENIXBeamParameter(INPUTGENERATOR::Pythia6);
    }

    // pythia8
    if (Input::PYTHIA8)
    {
        //! Nominal collision geometry is selected by Input::BEAM_CONFIGURATION
        Input::ApplysPHENIXBeamParameter(INPUTGENERATOR::Pythia8);
    }

    //--------------
    // Set Input Manager specific options
    //--------------
    // can only be set after InputInit() is called

    if (Input::HEPMC)
    {
        //! Nominal collision geometry is selected by Input::BEAM_CONFIGURATION
        Input::ApplysPHENIXBeamParameter(INPUTMANAGER::HepMCInputManager);

        // optional overriding beam parameters
        // INPUTMANAGER::HepMCInputManager->set_vertex_distribution_width(100e-4, 100e-4, 8, 0);  //optional collision smear in space, time
        //    INPUTMANAGER::HepMCInputManager->set_vertex_distribution_mean(0,0,0,0);//optional collision central position shift in space, time
        // //optional choice of vertex distribution function in space, time
        // INPUTMANAGER::HepMCInputManager->set_vertex_distribution_function(PHHepMCGenHelper::Gaus, PHHepMCGenHelper::Gaus, PHHepMCGenHelper::Gaus, PHHepMCGenHelper::Gaus);
        //! embedding ID for the event
        //! positive ID is the embedded event of interest, e.g. jetty event from pythia
        //! negative IDs are backgrounds, .e.g out of time pile up collisions
        //! Usually, ID = 0 means the primary Au+Au collision background
        // INPUTMANAGER::HepMCInputManager->set_embedding_id(Input::EmbedID);
        if (Input::PILEUPRATE > 0)
        {
            // Copy vertex settings from foreground hepmc input
            INPUTMANAGER::HepMCPileupInputManager->CopyHelperSettings(INPUTMANAGER::HepMCInputManager);
            // and then modify the ones you want to be different
            // INPUTMANAGER::HepMCPileupInputManager->set_vertex_distribution_width(100e-4,100e-4,8,0);
        }
    }
    if (Input::PILEUPRATE > 0)
    {
        //! Nominal collision geometry is selected by Input::BEAM_CONFIGURATION
        Input::ApplysPHENIXBeamParameter(INPUTMANAGER::HepMCPileupInputManager);
    }
    // register all input generators with Fun4All

    // Heavy-flavor simulation setup //! need to be set before InputRegister() otherwise weird things happen (but why?)
    PHPy8ParticleTrigger *p8_hf_signal_trigger = new PHPy8ParticleTrigger();
    p8_hf_signal_trigger->SetPtLow(0.);
    p8_hf_signal_trigger->SetEtaHighLow(1.3, -1.3);     // sample a rapidity range higher than the sPHENIX tracking pseudorapidity
    p8_hf_signal_trigger->SetStableParticleOnly(false); // process unstable particles that include quarks
    p8_hf_signal_trigger->PrintConfig();
    if (prefix != "MinBias")
    {
        p8_hf_signal_trigger->AddParticles(particleID);
        p8_hf_signal_trigger->AddParticles(-1 * particleID);
        INPUTGENERATOR::Pythia8->register_trigger(p8_hf_signal_trigger);
        INPUTGENERATOR::Pythia8->set_trigger_OR();

        // ! Nominal collision geometry is selected by Input::BEAM_CONFIGURATION
        // Input::ApplysPHENIXBeamParameter(INPUTGENERATOR::Pythia8);
    }

    InputRegister();

    if (!Input::READHITS)
    {
        rc->set_IntFlag("RUNNUMBER", 1); //! This need to be set for G4_TrkrSimulation.C TPC()?

        SyncReco *sync = new SyncReco();
        se->registerSubsystem(sync);

        HeadReco *head = new HeadReco();
        se->registerSubsystem(head);
    }

    FlagHandler *flag = new FlagHandler();
    se->registerSubsystem(flag);

    // set up production relatedstuff
    //   Enable::PRODUCTION = true;

    //======================
    // Write the DST
    //======================

    Enable::DSTOUT = true;
    Enable::DSTOUT_COMPRESS = false;
    DstOut::OutputDir = outdir;
    DstOut::OutputFile = outputFile;

    // Option to convert DST to human command readable TTree for quick poke around the outputs
    //   Enable::DSTREADER = true;

    // turn the display on (default off)
    // Enable::DISPLAY = true;

    //======================
    // What to run
    //======================
    // QA, main switch
    Enable::QA = true;

    // Heavy-flavor simulation setup
    if (prefix != "MinBias")
    {
        DecayFinder *myFinder = new DecayFinder("myFinder");
        myFinder->Verbosity(1);
        if (prefix == "Kshort2pipi")
        {
            myFinder->setDecayDescriptor("K_S0 -> pi^- pi^+");
        }
        else if (prefix == "D2Kpi")
        {
            myFinder->setDecayDescriptor("[D0 -> K^- pi^+]cc");
        }
        else if (prefix == "D2Kpipi") // D+
        {
          myFinder->setDecayDescriptor("[D+ -> K^- pi^+ pi^+]cc");
        }
        else if (prefix == "D2KKpi") // D_s
        {
          myFinder->setDecayDescriptor("[D_s+ -> {phi -> K^+ K^-} pi^+]cc");
        }
        else if (prefix == "Lc2pKpi") // L_c
        {
          myFinder->setDecayDescriptor("[Lambda_c+ -> proton^+ K^- pi^+]cc");
        }
        myFinder->saveDST(1);
        myFinder->allowPi0(1);
        myFinder->allowPhotons(1);
        myFinder->triggerOnDecay(1);
        myFinder->setPTmin(0.0);          // Note: sPHENIX min pT is 0.2 GeV for tracking
        myFinder->setEtaRange(-1.3, 1.3); // Note: sPHENIX acceptance is |eta| <= 1.1
        myFinder->useDecaySpecificEtaRange(false);
        se->registerSubsystem(myFinder);
    }

    // Heavy-flavor simulation setup
    Enable::MVTX_APPLYMISALIGNMENT = true;
    ACTSGEOM::mvtx_applymisalignment = Enable::MVTX_APPLYMISALIGNMENT;

    // Global options (enabled for all enables subsystems - if implemented)
    //  Enable::ABSORBER = true;
    //  Enable::OVERLAPCHECK = true;
    //  Enable::VERBOSITY = 1;

    // Enable::MBD = true;
    // Enable::MBD_SUPPORT = true; // save hist in MBD/BBC support structure
    // Enable::MBDRECO = Enable::MBD && true;
    Enable::MBDFAKE = true; // Smeared vtx and t0, use if you don't want real MBD/BBC in simulation

    Enable::PIPE = true;
    Enable::PIPE_ABSORBER = true;

    // central tracking
    Enable::MVTX = true;
    Enable::MVTX_CELL = Enable::MVTX && true;
    Enable::MVTX_CLUSTER = Enable::MVTX_CELL && true;
    Enable::MVTX_QA = Enable::MVTX_CLUSTER && Enable::QA && true;
    Enable::MVTX_VERBOSITY = 0;

    Enable::INTT = true;
    // Enable::INTT_ABSORBER = true; // enables layerwise support structure readout
    // Enable::INTT_SUPPORT = true; // enable global support structure readout
    Enable::INTT_CELL = Enable::INTT && true;
    Enable::INTT_CLUSTER = Enable::INTT_CELL && true;
    Enable::INTT_QA = Enable::INTT_CLUSTER && Enable::QA && true;
    Enable::INTT_VERBOSITY = 0;

    Enable::TPC = true;
    Enable::TPC_ABSORBER = false;
    Enable::TPC_CELL = Enable::TPC && true;
    Enable::TPC_CLUSTER = Enable::TPC_CELL && true;
    Enable::TPC_QA = Enable::TPC_CLUSTER && Enable::QA && true;

    Enable::MICROMEGAS = true;
    Enable::MICROMEGAS_CELL = Enable::MICROMEGAS && true;
    Enable::MICROMEGAS_CLUSTER = Enable::MICROMEGAS_CELL && true;
    Enable::MICROMEGAS_QA = Enable::MICROMEGAS_CLUSTER && Enable::QA && true;

    Enable::TRACKING_TRACK = (Enable::MICROMEGAS_CLUSTER && Enable::TPC_CLUSTER && Enable::INTT_CLUSTER && Enable::MVTX_CLUSTER) && true;
    Enable::GLOBAL_RECO = (Enable::MBDFAKE || Enable::MBDRECO || Enable::TRACKING_TRACK) && true;
    Enable::TRACKING_EVAL = Enable::TRACKING_TRACK && Enable::GLOBAL_RECO && true;
    Enable::TRACKING_QA = Enable::TRACKING_TRACK && Enable::QA && true;
    Enable::TRACKING_VERBOSITY = 0;

    // only do track matching if TRACKINGTRACK is also used
    Enable::TRACK_MATCHING = Enable::TRACKING_TRACK && false;
    Enable::TRACK_MATCHING_TREE = Enable::TRACK_MATCHING && false;
    Enable::TRACK_MATCHING_TREE_CLUSTERS = Enable::TRACK_MATCHING_TREE && false;

    // Additional tracking tools
    // Enable::TRACKING_DIAGNOSTICS = Enable::TRACKING_TRACK && true;
    // G4TRACKING::filter_conversion_electrons = true;
    //  G4TRACKING::use_alignment = true;

    // enable pp mode and set extended readout time
    // TRACKING::pp_mode = true;
    // TRACKING::pp_extended_readout_time = 20000;

    // set flags to simulate and correct TPC distortions, specify distortion and correction files
    // G4TPC::ENABLE_STATIC_DISTORTIONS = true;
    // G4TPC::static_distortion_filename = std::string("/sphenix/user/rcorliss/distortion_maps/2023.02/Summary_hist_mdc2_UseFieldMaps_AA_event_0_bX180961051_0.distortion_map.hist.root");
    // G4TPC::ENABLE_STATIC_CORRECTIONS = true;
    // G4TPC::static_correction_filename = std::string("/sphenix/user/rcorliss/distortion_maps/2023.02/Summary_hist_mdc2_UseFieldMaps_AA_smoothed_average.correction_map.hist.root");
    // G4TPC::ENABLE_AVERAGE_CORRECTIONS = false;

    //  cemc electronics + thin layer of W-epoxy to get albedo from cemc
    //  into the tracking, cannot run together with CEMC
    //  Enable::CEMCALBEDO = true;

    Enable::CEMC = false;
    Enable::CEMC_ABSORBER = false;
    Enable::CEMC_CELL = Enable::CEMC && false;
    Enable::CEMC_TOWER = Enable::CEMC_CELL && false;
    Enable::CEMC_CLUSTER = Enable::CEMC_TOWER && false;
    Enable::CEMC_EVAL = Enable::CEMC_G4Hit && Enable::CEMC_CLUSTER && false;
    Enable::CEMC_QA = Enable::CEMC_CLUSTER && Enable::QA && false;

    Enable::HCALIN = false;
    Enable::HCALIN_ABSORBER = false;
    Enable::HCALIN_CELL = Enable::HCALIN && false;
    Enable::HCALIN_TOWER = Enable::HCALIN_CELL && false;
    Enable::HCALIN_CLUSTER = Enable::HCALIN_TOWER && false;
    Enable::HCALIN_EVAL = Enable::HCALIN_G4Hit && Enable::HCALIN_CLUSTER && false;
    Enable::HCALIN_QA = Enable::HCALIN_CLUSTER && Enable::QA && false;

    Enable::MAGNET = true;
    Enable::MAGNET_ABSORBER = true;

    Enable::HCALOUT = false;
    Enable::HCALOUT_ABSORBER = false;
    Enable::HCALOUT_CELL = Enable::HCALOUT && false;
    Enable::HCALOUT_TOWER = Enable::HCALOUT_CELL && false;
    Enable::HCALOUT_CLUSTER = Enable::HCALOUT_TOWER && false;
    Enable::HCALOUT_EVAL = Enable::HCALOUT_G4Hit && Enable::HCALOUT_CLUSTER && false;
    Enable::HCALOUT_QA = Enable::HCALOUT_CLUSTER && Enable::QA && false;

    Enable::EPD = true;
    Enable::EPD_TILE = Enable::EPD && true;

    Enable::BEAMLINE = true;
    //  Enable::BEAMLINE_ABSORBER = true;  // makes the beam line magnets sensitive volumes
    //  Enable::BEAMLINE_BLACKHOLE = true; // turns the beamline magnets into black holes
    Enable::ZDC = true;
    //  Enable::ZDC_ABSORBER = true;
    //  Enable::ZDC_SUPPORT = true;
    Enable::ZDC_TOWER = Enable::ZDC && true;
    Enable::ZDC_EVAL = Enable::ZDC_TOWER && false;

    //! forward flux return plug door. Out of acceptance and off by default.
    // Enable::PLUGDOOR = true;
    Enable::PLUGDOOR_ABSORBER = true;

    // Enable::GLOBAL_FASTSIM = true;

    // Enable::KFPARTICLE = true;
    // Enable::KFPARTICLE_VERBOSITY = 1;
    // Enable::KFPARTICLE_TRUTH_MATCH = true;
    // Enable::KFPARTICLE_SAVE_NTUPLE = true;

    Enable::CALOTRIGGER = Enable::CEMC_TOWER && Enable::HCALIN_TOWER && Enable::HCALOUT_TOWER && false;

    Enable::JETS = (Enable::GLOBAL_RECO || Enable::GLOBAL_FASTSIM) && false;
    Enable::JETS_EVAL = Enable::JETS && false;
    Enable::JETS_QA = Enable::JETS && Enable::QA && true;

    // HI Jet Reco for p+Au / Au+Au collisions (default is false for
    // single particle / p+p-only simulations, or for p+Au / Au+Au
    // simulations which don't particularly care about jets)
    Enable::HIJETS = Enable::JETS && Enable::CEMC_TOWER && Enable::HCALIN_TOWER && Enable::HCALOUT_TOWER && false;

    // 3-D topoCluster reconstruction, potentially in all calorimeter layers
    Enable::TOPOCLUSTER = Enable::CEMC_TOWER && Enable::HCALIN_TOWER && Enable::HCALOUT_TOWER && false;
    // particle flow jet reconstruction - needs topoClusters!
    Enable::PARTICLEFLOW = Enable::TOPOCLUSTER && true;
    // centrality reconstruction
    Enable::CENTRALITY = false;

    // new settings using Enable namespace in GlobalVariables.C
    Enable::BLACKHOLE = true;
    // Enable::BLACKHOLE_SAVEHITS = false; // turn off saving of bh hits
    // Enable::BLACKHOLE_FORWARD_SAVEHITS = false; // disable forward/backward hits
    // BlackHoleGeometry::visible = true;

    // run user provided code (from local G4_User.C)
    // Enable::USER = true;

    //===============
    // conditions DB flags
    //===============
    Enable::CDB = true;
    // global tag
    rc->set_StringFlag("CDB_GLOBALTAG", CDB::global_tag);
    // 64 bit timestamp
    rc->set_uint64Flag("TIMESTAMP", CDB::timestamp);

    //---------------
    // World Settings
    //---------------
    //  G4WORLD::PhysicsList = "FTFP_BERT"; //FTFP_BERT_HP best for calo
    //  G4WORLD::WorldMaterial = "G4_AIR"; // set to G4_GALACTIC for material scans

    //---------------
    // Magnet Settings
    //---------------

    //  G4MAGNET::magfield =  std::string(getenv("CALIBRATIONROOT"))+ std::string("/Field/Map/sphenix3dbigmapxyz.root");  // default map from the calibration database
    //  G4MAGNET::magfield = "1.5"; // alternatively to specify a constant magnetic field, give a float number, which will be translated to solenoidal field in T, if string use as fieldmap name (including path)
    //  G4MAGNET::magfield_rescale = 1.;  // make consistent with expected Babar field strength of 1.4T

    //---------------
    // Pythia Decayer
    //---------------
    // list of decay types in
    // $OFFLINE_MAIN/include/g4decayer/EDecayType.hh
    // default is All:
    // G4P6DECAYER::decayType = EDecayType::kAll;

    // Initialize the selected subsystems
    G4Init();

    //---------------------
    // GEANT4 Detector description
    //---------------------
    if (!Input::READHITS)
    {
        G4Setup();
    }

    //------------------
    // Detector Division
    //------------------

    if ((Enable::MBD && Enable::MBDRECO) || Enable::MBDFAKE)
        Mbd_Reco();

    if (Enable::MVTX_CELL)
        Mvtx_Cells();
    if (Enable::INTT_CELL)
        Intt_Cells();
    if (Enable::TPC_CELL)
        TPC_Cells();
    if (Enable::MICROMEGAS_CELL)
        Micromegas_Cells();

    if (Enable::CEMC_CELL)
        CEMC_Cells();

    if (Enable::HCALIN_CELL)
        HCALInner_Cells();

    if (Enable::HCALOUT_CELL)
        HCALOuter_Cells();

    //-----------------------------
    // CEMC towering and clustering
    //-----------------------------

    if (Enable::CEMC_TOWER)
        CEMC_Towers();
    if (Enable::CEMC_CLUSTER)
        CEMC_Clusters();

    //--------------
    // EPD tile reconstruction
    //--------------

    if (Enable::EPD_TILE)
        EPD_Tiles();

    //-----------------------------
    // HCAL towering and clustering
    //-----------------------------

    if (Enable::HCALIN_TOWER)
        HCALInner_Towers();
    if (Enable::HCALIN_CLUSTER)
        HCALInner_Clusters();

    if (Enable::HCALOUT_TOWER)
        HCALOuter_Towers();
    if (Enable::HCALOUT_CLUSTER)
        HCALOuter_Clusters();

    // if enabled, do topoClustering early, upstream of any possible jet reconstruction
    if (Enable::TOPOCLUSTER)
        TopoClusterReco();

    //--------------
    // SVTX tracking
    //--------------
    if (Enable::TRACKING_TRACK)
    {
        TrackingInit();
    }
    if (Enable::MVTX_CLUSTER)
        Mvtx_Clustering();
    if (Enable::INTT_CLUSTER)
        Intt_Clustering();
    if (Enable::TPC_CLUSTER)
    {
        if (G4TPC::ENABLE_DIRECT_LASER_HITS || G4TPC::ENABLE_CENTRAL_MEMBRANE_HITS)
        {
            TPC_LaserClustering();
        }
        else
        {
            TPC_Clustering();
        }
    }
    if (Enable::MICROMEGAS_CLUSTER)
        Micromegas_Clustering();

    if (Enable::TRACKING_TRACK)
    {
        Tracking_Reco();
    }

    if (Enable::TRACKING_DIAGNOSTICS)
    {
        const std::string kshortFile = "./kshort_" + outputFile;
        const std::string residualsFile = "./residuals_" + outputFile;

        G4KshortReconstruction(kshortFile);
        seedResiduals(residualsFile);
    }

    // Heavy-flavor simulation setup
    auto vtxfinder = new PHSimpleVertexFinder;
    vtxfinder->Verbosity(0);
    vtxfinder->setDcaCut(0.5);
    vtxfinder->setTrackPtCut(-99999.);
    vtxfinder->setBeamLineCut(1);
    vtxfinder->setTrackQualityCut(1000000000);
    vtxfinder->setNmvtxRequired(3);
    vtxfinder->setOutlierPairCut(0.1);
    se->registerSubsystem(vtxfinder);

    //-----------------
    // Global Vertexing
    //-----------------

    if (Enable::GLOBAL_RECO && Enable::GLOBAL_FASTSIM)
    {
        std::cout << "You can only enable Enable::GLOBAL_RECO or Enable::GLOBAL_FASTSIM, not both" << std::endl;
        gSystem->Exit(1);
    }
    if (Enable::GLOBAL_RECO)
    {
        Global_Reco();
    }
    else if (Enable::GLOBAL_FASTSIM)
    {
        Global_FastSim();
    }

    //-----------------
    // Centrality Determination
    //-----------------

    if (Enable::CENTRALITY)
    {
        Centrality();
    }

    //-----------------
    // Calo Trigger Simulation
    //-----------------

    if (Enable::CALOTRIGGER)
    {
        CaloTrigger_Sim();
    }

    //---------
    // Jet reco
    //---------

    if (Enable::JETS)
        Jet_Reco();
    if (Enable::HIJETS)
        HIJetReco();

    if (Enable::PARTICLEFLOW)
        ParticleFlow();

    // Heavy-flavor simulation setup
    build_truthreco_tables();

    //----------------------
    // Simulation evaluation
    //----------------------
    string outputroot = outdir + outputFile;
    string remove_this = ".root";
    pos = outputroot.find(remove_this);
    if (pos != string::npos)
    {
        outputroot.erase(pos, remove_this.length());
    }

    output_dir = outdir; // Top dir of where the output nTuples will be written
    trailer = "_" + processID + ".root";
    if (prefix == "Kshort2pipi")
    {
        create_hf_directories(pipi_reconstruction_name, pipi_output_dir, pipi_output_reco_file);

        HFTrackEfficiency *myTrackEff = new HFTrackEfficiency("myTrackEff");
        myTrackEff->Verbosity(1);
        // myTrackEff->setTruthRecoMatchingPercentage(100.);
        myTrackEff->setDFNodeName("myFinder");
        myTrackEff->triggerOnDecay(1);
        myTrackEff->writeSelectedTrackMap(true);
        myTrackEff->writeOutputFile(true);
        std::string dir_HFEff = "HFEff";
        system(("mkdir -p " + pipi_output_dir + "/" + dir_HFEff).c_str());
        std::string outputHFEffFile = pipi_output_dir + "/" + dir_HFEff + "/outputHFTrackEff_" + prefix + "_" + processID + ".root";
        myTrackEff->setOutputFileName(outputHFEffFile);
        se->registerSubsystem(myTrackEff);

        init_kfp_dependencies();
        reconstruct_pipi_mass();
    }
    else if (prefix == "D2Kpi")
    {
        create_hf_directories(Kpi_reconstruction_name, Kpi_output_dir, Kpi_output_reco_file);

        HFTrackEfficiency *myTrackEff = new HFTrackEfficiency("myTrackEff");
        myTrackEff->Verbosity(1);
        // myTrackEff->setTruthRecoMatchingPercentage(100.);
        myTrackEff->setDFNodeName("myFinder");
        myTrackEff->triggerOnDecay(1);
        myTrackEff->writeSelectedTrackMap(true);
        myTrackEff->writeOutputFile(true);
        std::string dir_HFEff = "HFEff";
        system(("mkdir -p " + Kpi_output_dir + "/" + dir_HFEff).c_str());
        std::string outputHFEffFile = Kpi_output_dir + "/" + dir_HFEff + "/outputHFTrackEff_" + prefix + "_" + processID + ".root";
        myTrackEff->setOutputFileName(outputHFEffFile);
        se->registerSubsystem(myTrackEff);

        init_kfp_dependencies();
        reconstruct_Kpi_mass();
    }
    else if (prefix == "D2Kpipi") // D+
    {
      create_hf_directories(Kpipi_reconstruction_name, Kpipi_output_dir, Kpipi_output_reco_file);

      HFTrackEfficiency *myTrackEff = new HFTrackEfficiency("myTrackEff");
      myTrackEff->Verbosity(1);
      //myTrackEff->setTruthRecoMatchingPercentage(100.);
      myTrackEff->setDFNodeName("myFinder");
      myTrackEff->triggerOnDecay(1);
      myTrackEff->writeSelectedTrackMap(true);
      myTrackEff->writeOutputFile(true);
      std::string dir_HFEff = "HFEff";
      system(("mkdir -p " + Kpipi_output_dir + "/" + dir_HFEff).c_str());
      std::string outputHFEffFile = Kpipi_output_dir + "/" + dir_HFEff + "outputHFTrackEff_" + prefix + "_" + processID + ".root";
      myTrackEff->setOutputFileName(outputHFEffFile);
      se->registerSubsystem(myTrackEff);

      init_kfp_dependencies();
      reconstruct_Kpipi_mass();
    }
    else if (prefix == "D2KKpi") // D_s
    {
      create_hf_directories(KKpi_reconstruction_name, KKpi_output_dir, KKpi_output_reco_file);

      HFTrackEfficiency *myTrackEff = new HFTrackEfficiency("myTrackEff");
      myTrackEff->Verbosity(1);
      //myTrackEff->setTruthRecoMatchingPercentage(100.);
      myTrackEff->setDFNodeName("myFinder");
      myTrackEff->triggerOnDecay(1);
      myTrackEff->writeSelectedTrackMap(true);
      myTrackEff->writeOutputFile(true);
      std::string dir_HFEff = "HFEff";
      system(("mkdir -p " + KKpi_output_dir + "/" + dir_HFEff).c_str());
      std::string outputHFEffFile = KKpi_output_dir + "/" + dir_HFEff + "outputHFTrackEff_" + prefix + "_" + processID + ".root";
      myTrackEff->setOutputFileName(outputHFEffFile);
      se->registerSubsystem(myTrackEff);

      init_kfp_dependencies();
      reconstruct_KKpi_mass();
    }
    else if (prefix == "Lc2pKpi") // Lambda_c
    {
      create_hf_directories(pKpi_reconstruction_name, pKpi_output_dir, pKpi_output_reco_file);

      HFTrackEfficiency *myTrackEff = new HFTrackEfficiency("myTrackEff");
      myTrackEff->Verbosity(1);
      //myTrackEff->setTruthRecoMatchingPercentage(100.);
      myTrackEff->setDFNodeName("myFinder");
      myTrackEff->triggerOnDecay(1);
      myTrackEff->writeSelectedTrackMap(true);
      myTrackEff->writeOutputFile(true);
      std::string dir_HFEff = "HFEff";
      system(("mkdir -p " + pKpi_output_dir + "/" + dir_HFEff).c_str());
      std::string outputHFEffFile = pKpi_output_dir + "/" + dir_HFEff + "outputHFTrackEff_" + prefix + "_" + processID + ".root";
      myTrackEff->setOutputFileName(outputHFEffFile);
      se->registerSubsystem(myTrackEff);

      init_kfp_dependencies();
      reconstruct_pKpi_mass();
    }
    else if (prefix == "MinBias")
    {
        create_hf_directories(mb_reconstruction_name, mb_output_dir, mb_output_reco_file);
        std::cout << "mb_reconstruction_name: " << mb_reconstruction_name << std::endl;
        std::cout << "mb_output_dir: " << mb_output_dir << std::endl;
        std::cout << "mb_output_reco_file: " << mb_output_reco_file << std::endl;
        init_kfp_dependencies();
        reconstruct_mb_mass();
    }

    if (Enable::TRACKING_EVAL)
    {
        std::string dir_eval = "TrackingEval";
        if (prefix == "Kshort2pipi")
        {
            system(("mkdir -p " + pipi_output_dir + "/" + dir_eval).c_str());
            Tracking_Eval(Form("%s/%s/outputTrackingEval_%s_%s.root", pipi_output_dir.c_str(), dir_eval.c_str(), prefix.c_str(), processID.c_str()));
        }
        else if (prefix == "D2Kpi")
        {
            system(("mkdir -p " + Kpi_output_dir + "/" + dir_eval).c_str());
            Tracking_Eval(Form("%s/%s/outputTrackingEval_%s_%s.root", Kpi_output_dir.c_str(), dir_eval.c_str(), prefix.c_str(), processID.c_str()));
        }
        else if (prefix == "D2Kpipi")
        {
            system(("mkdir -p " + Kpipi_output_dir + "/" + dir_eval).c_str());
            Tracking_Eval(Form("%s/%s/outputTrackingEval_%s_%s.root", Kpipi_output_dir.c_str(), dir_eval.c_str(), prefix.c_str(), processID.c_str()));
        }
        else if (prefix == "D2KKpi")
        {
            system(("mkdir -p " + KKpi_output_dir + "/" + dir_eval).c_str());
            Tracking_Eval(Form("%s/%s/outputTrackingEval_%s_%s.root", KKpi_output_dir.c_str(), dir_eval.c_str(), prefix.c_str(), processID.c_str()));
        }
        else if (prefix == "Lc2pKpi")
        {
            system(("mkdir -p " + pKpi_output_dir + "/" + dir_eval).c_str());
            Tracking_Eval(Form("%s/%s/outputTrackingEval_%s_%s.root", pKpi_output_dir.c_str(), dir_eval.c_str(), prefix.c_str(), processID.c_str()));
        }
        else if (prefix == "MinBias")
        {
            system(("mkdir -p " + mb_output_dir + "/" + dir_eval).c_str());
            Tracking_Eval(Form("%s/%s/outputTrackingEval_%s_%s.root", mb_output_dir.c_str(), dir_eval.c_str(), prefix.c_str(), processID.c_str()));
        }
    }

    if (Enable::CEMC_EVAL)
        CEMC_Eval(outputroot + "_g4cemc_eval.root");

    if (Enable::HCALIN_EVAL)
        HCALInner_Eval(outputroot + "_g4hcalin_eval.root");

    if (Enable::HCALOUT_EVAL)
        HCALOuter_Eval(outputroot + "_g4hcalout_eval.root");

    if (Enable::JETS_EVAL)
        Jet_Eval(outputroot + "_g4jet_eval.root");

    if (Enable::DSTREADER)
        G4DSTreader(outputroot + "_DSTReader.root");

    if (Enable::USER)
        UserAnalysisInit();

    // Writes electrons from conversions to a new track map on the node tree
    // the ntuple file is for diagnostics, it is produced only if the flag is set in G4_Tracking.C
    if (G4TRACKING::filter_conversion_electrons)
        Filter_Conversion_Electrons(outputroot + "_secvert_ntuple.root");

    //======================
    // Run KFParticle on evt
    //======================
    if (Enable::KFPARTICLE && Input::UPSILON)
        KFParticle_Upsilon_Reco();
    if (Enable::KFPARTICLE && Input::DZERO)
        KFParticle_D0_Reco();

    //----------------------
    // Standard QAs
    //----------------------

    if (Enable::CEMC_QA)
        CEMC_QA();
    if (Enable::HCALIN_QA)
        HCALInner_QA();
    if (Enable::HCALOUT_QA)
        HCALOuter_QA();

    if (Enable::JETS_QA)
        Jet_QA();

    if (Enable::MVTX_QA)
        Mvtx_QA();
    if (Enable::INTT_QA)
        Intt_QA();
    if (Enable::TPC_QA)
        TPC_QA();
    if (Enable::MICROMEGAS_QA)
        Micromegas_QA();
    if (Enable::TRACKING_QA)
        Tracking_QA();

    if (Enable::TRACKING_QA && Enable::CEMC_QA && Enable::HCALIN_QA && Enable::HCALOUT_QA)
        QA_G4CaloTracking();

    if (Enable::TRACK_MATCHING)
        Track_Matching(outputroot + "_g4trackmatching.root");

    //--------------
    // Set up Input Managers
    //--------------

    InputManagers();

    if (Enable::PRODUCTION)
    {
        Production_CreateOutputDir();
    }

    if (Enable::DSTOUT)
    {
        // std::string FullOutFile = DstOut::OutputDir + "/" + DstOut::OutputFile;
        std::string FullOutFile;
        if (prefix == "Kshort2pipi")
        {
            FullOutFile = pipi_output_dir + "/DST/" + prefix + "_DST_" + processID + ".root";
            system(("mkdir -p " + pipi_output_dir + "/DST").c_str());
            std::cout << "Final DST output file: " << FullOutFile << std::endl;
        }
        else if (prefix == "D2Kpi")
        {
            FullOutFile = Kpi_output_dir + "/DST/" + prefix + "_DST_" + processID + ".root";
            system(("mkdir -p " + Kpi_output_dir + "/DST").c_str());
            std::cout << "Final DST output file: " << FullOutFile << std::endl;
        }
        else if (prefix == "D2Kpipi")
        {
            FullOutFile = Kpipi_output_dir + "/DST/" + prefix + "_DST_" + processID + ".root";
            system(("mkdir -p " + Kpipi_output_dir + "/DST").c_str());
            std::cout << "Final DST output file: " << FullOutFile << std::endl;
        }
        else if (prefix == "D2KKpi")
        {
            FullOutFile = KKpi_output_dir + "/DST/" + prefix + "_DST_" + processID + ".root";
            system(("mkdir -p " + KKpi_output_dir + "/DST").c_str());
            std::cout << "Final DST output file: " << FullOutFile << std::endl;
        }
        else if (prefix == "Lc2pKpi")
        {
            FullOutFile = pKpi_output_dir + "/DST/" + prefix + "_DST_" + processID + ".root";
            system(("mkdir -p " + pKpi_output_dir + "/DST").c_str());
            std::cout << "Final DST output file: " << FullOutFile << std::endl;
        }
        else if (prefix == "MinBias")
        {
            FullOutFile = mb_output_dir + "/DST/" + prefix + "_DST_" + processID + ".root";
            system(("mkdir -p " + mb_output_dir + "/DST").c_str());
            std::cout << "Final DST output file: " << FullOutFile << std::endl;
        }
        Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", FullOutFile);
        out->AddNode("G4GEO_TPC");
        // out->StripNode("G4HIT_PIPE");
        // out->StripNode("G4HIT_SVTXSUPPORT");
        // out->StripNode("PHG4INEVENT");
        // out->StripNode("Sync");
        // out->StripNode("myFinder_DecayMap");
        // out->StripNode("G4HIT_PIPE");
        // out->StripNode("G4HIT_MVTX");
        // out->StripNode("G4HIT_INTT");
        // out->StripNode("G4HIT_TPC");
        // out->StripNode("G4HIT_MICROMEGAS");
        // out->StripNode("TRKR_HITSET");
        // out->StripNode("TRKR_HITTRUTHASSOC");
        // out->StripNode("TRKR_CLUSTER");
        // out->StripNode("TRKR_CLUSTERHITASSOC");
        // out->StripNode("TRKR_CLUSTERCROSSINGASSOC");
        // out->StripNode("TRAINING_HITSET");
        // out->StripNode("TRKR_TRUTHTRACKCONTAINER");
        // out->StripNode("TRKR_TRUTHCLUSTERCONTAINER");
        // out->StripNode("alignmentTransformationContainer");
        // out->StripNode("alignmentTransformationContainerTransient");
        // out->StripNode("SiliconTrackSeedContainer");
        // out->StripNode("TpcTrackSeedContainer");
        // out->StripNode("SvtxTrackSeedContainer");
        // out->StripNode("ActsTrajectories");
        // out->StripNode("SvtxTrackMap");
        // out->StripNode("SvtxAlignmentStateMap");
        // out->SaveRunNode(0);

        if (Enable::DSTOUT_COMPRESS)
        {
            ShowerCompress();
            DstCompress(out);
        }
        se->registerOutputManager(out);
        std::cout << "Saving DST output to: " << FullOutFile << std::endl;
    }
    //-----------------
    // Event processing
    //-----------------
    if (Enable::DISPLAY)
    {
        DisplayOn();

        gROOT->ProcessLine("Fun4AllServer *se = Fun4AllServer::instance();");
        gROOT->ProcessLine("PHG4Reco *g4 = (PHG4Reco *) se->getSubsysReco(\"PHG4RECO\");");

        std::cout << "-------------------------------------------------" << std::endl;
        std::cout << "You are in event display mode. Run one event with" << std::endl;
        std::cout << "se->run(1)" << std::endl;
        std::cout << "Run Geant4 command with following examples" << std::endl;
        gROOT->ProcessLine("displaycmd()");

        return 0;
    }

    if (nEvents < 0)
    {
        return 0;
    }

    se->skip(skip);
    se->run(nEvents);
    //  se->PrintTimer();

    //-----
    // QA output
    //-----
    if (Enable::QA)
    {
        std::string output_dir_QA = "./qaOut"; // default
        if (prefix == "Kshort2pipi")
        {
            output_dir_QA = pipi_output_dir + "/qaOut";
        }
        else if (prefix == "D2Kpi")
        {
            output_dir_QA = Kpi_output_dir + "/qaOut";
        }
        else if (prefix == "D2Kpipi")
        {
            output_dir_QA = Kpipi_output_dir + "/qaOut";
        }
        else if (prefix == "D2KKpi")
        {
            output_dir_QA = KKpi_output_dir + "/qaOut";
        }
        else if (prefix == "Lc2pKpi")
        {
            output_dir_QA = pKpi_output_dir + "/qaOut";
        }
        else if (prefix == "MinBias")
        {
            output_dir_QA = mb_output_dir + "/qaOut";
        }
        std::string makeDirectory = "mkdir -p " + output_dir_QA;
        system(makeDirectory.c_str());
        QA_Output(output_dir_QA + "/qaHists" + trailer);
    }

    //-----
    // Exit
    //-----

    CDBInterface::instance()->Print(); // print used DB files
    se->End();

    if (prefix == "Kshort2pipi")
    {
        end_kfparticle(pipi_output_reco_file, pipi_output_dir);
    }
    else if (prefix == "D2Kpi")
    {
        end_kfparticle(Kpi_output_reco_file, Kpi_output_dir);
    }
    else if (prefix == "D2Kpipi") // D+
    {
      end_kfparticle(Kpipi_output_reco_file, Kpipi_output_dir);
    }
    else if (prefix == "D2KKpi") // D_s
    {
      end_kfparticle(KKpi_output_reco_file, KKpi_output_dir);
    }
    else if (prefix == "Lc2pKpi") // Lambda_c
    {
      end_kfparticle(pKpi_output_reco_file, pKpi_output_dir);
    }
    else if (prefix == "MinBias")
    {
        std::cout << "mb_output_reco_file: " << mb_output_reco_file << std::endl;
        std::cout << "mb_output_dir: " << mb_output_dir << std::endl;
        end_kfparticle(mb_output_reco_file, mb_output_dir);
    }

    std::cout << "All done" << std::endl;
    delete se;
    if (Enable::PRODUCTION)
    {
        Production_MoveOutput();
    }

    gSystem->Exit(0);
    return 0;
}
#endif
