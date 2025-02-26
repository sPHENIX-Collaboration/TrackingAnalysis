#include <iostream>
#include <map>

void match() {
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1);
    TGaxis::SetMaxDigits(3);

    int run = 53744;

    TFile *file_kfp = new TFile(Form("all%d_kfp.root",run));
    TTree *tree_kfp = (TTree*) file_kfp->Get("DecayTree");
    TTree *tree_kfp_v2 = tree_kfp->CopyTree("track_1_MVTX_nHits>0 && track_1_INTT_nHits>0 && track_2_MVTX_nHits>0 && track_2_INTT_nHits>0 && track_1_pT>0.2 && track_2_pT>0.2 && K_S0_DIRA>0.88 && K_S0_decayLength>0.2 && track_1_track_2_DCA<5");
    TFile *file_tony = new TFile(Form("all%d_tony.root",run));
    TTree *tree_tony = (TTree*) file_tony->Get("ntp_reco_info");
    TTree *tree_tony_v2 = tree_tony->CopyTree("charge1==charge2 && crossing1 == crossing2 && sqrt(px1*px1+py1*py1) > 0.20 && sqrt(px2*px2+py2*py2) > 0.20 && cosThetaReco > 0.88 && (pow(projected_pathlength_x,2)+pow(projected_pathlength_y,2)+pow(projected_pathlength_z,2)) > 0.2 && abs(projected_pair_dca) < 0.5");

    float id1, id2, eventNumber, crossing1, crossing2;
    float invariant_mass, dca3dxy1, dca3dxy2, dca3dz1, dca3dz2;
    float projected_pathlength_x, projected_pathlength_y, projected_pathlength_z;
    tree_tony_v2->SetBranchAddress("id1",&id1);
    tree_tony_v2->SetBranchAddress("id2",&id2);
    tree_tony_v2->SetBranchAddress("eventNumber",&eventNumber);
    tree_tony_v2->SetBranchAddress("crossing1",&crossing1);
    tree_tony_v2->SetBranchAddress("crossing2",&crossing2);
    tree_tony_v2->SetBranchAddress("invariant_mass",&invariant_mass);
    tree_tony_v2->SetBranchAddress("projected_pathlength_x",&projected_pathlength_x);
    tree_tony_v2->SetBranchAddress("projected_pathlength_y",&projected_pathlength_y);
    tree_tony_v2->SetBranchAddress("projected_pathlength_z",&projected_pathlength_z);
    tree_tony_v2->SetBranchAddress("dca3dxy1",&dca3dxy1);
    tree_tony_v2->SetBranchAddress("dca3dxy2",&dca3dxy2);
    tree_tony_v2->SetBranchAddress("dca3dz1",&dca3dz1);
    tree_tony_v2->SetBranchAddress("dca3dz2",&dca3dz2);

    int track_1_bunch_crossing, track_2_bunch_crossing, eventNumberKFP, track_1_track_ID, track_2_track_ID;
    float K_S0_mass;
    float K_S0_decayLength;
    float track_1_IP_xy, track_2_IP_xy;
    float track_1_x, track_1_y, track_1_z;
    float track_2_x, track_2_y, track_2_z;
    float primary_vertex_x, primary_vertex_y, primary_vertex_z;
    tree_kfp_v2->SetBranchAddress("track_1_bunch_crossing",&track_1_bunch_crossing);
    tree_kfp_v2->SetBranchAddress("track_2_bunch_crossing",&track_2_bunch_crossing);
    tree_kfp_v2->SetBranchAddress("eventNumber",&eventNumberKFP);
    tree_kfp_v2->SetBranchAddress("track_1_track_ID",&track_1_track_ID);
    tree_kfp_v2->SetBranchAddress("track_2_track_ID",&track_2_track_ID);
    tree_kfp_v2->SetBranchAddress("K_S0_mass",&K_S0_mass);
    tree_kfp_v2->SetBranchAddress("K_S0_decayLength",&K_S0_decayLength);
    tree_kfp_v2->SetBranchAddress("track_1_IP_xy",&track_1_IP_xy);
    tree_kfp_v2->SetBranchAddress("track_2_IP_xy",&track_2_IP_xy);
    tree_kfp_v2->SetBranchAddress("track_1_x",&track_1_x);
    tree_kfp_v2->SetBranchAddress("track_1_y",&track_1_y);
    tree_kfp_v2->SetBranchAddress("track_1_z",&track_1_z);
    tree_kfp_v2->SetBranchAddress("track_2_x",&track_2_x);
    tree_kfp_v2->SetBranchAddress("track_2_y",&track_2_y);
    tree_kfp_v2->SetBranchAddress("track_2_z",&track_2_z);
    tree_kfp_v2->SetBranchAddress("primary_vertex_x",&primary_vertex_x);
    tree_kfp_v2->SetBranchAddress("primary_vertex_y",&primary_vertex_y);
    tree_kfp_v2->SetBranchAddress("primary_vertex_z",&primary_vertex_z);

    int nPass0(0), nPass1(0), nPass2(0), nPass3(0), nPass4(0), nPass5(0);
    std::map<std::tuple<int, int, int>, int> tonyMap;
    int nevent_tony = tree_tony_v2->GetEntries();
    for (int i=0; i<nevent_tony; i++)
    {
      tree_tony_v2->GetEntry(i);
      nPass0++;
      if ((int)crossing1 != (int)crossing2) continue;
      nPass1++;
      if (sqrt(projected_pathlength_x*projected_pathlength_x+projected_pathlength_y*projected_pathlength_y+projected_pathlength_z*projected_pathlength_z)<20) continue;
      nPass2++;
      tonyMap[std::make_tuple((int)eventNumber, (int)id1, (int)id2)] = i;
    }
    cout<<"all entries in KshortReconstruction = "<<nPass0<<endl;
    cout<<"crossing1==crossing2 = "<<nPass1<<endl;
    cout<<"decay length > 20 = "<<nPass2<<endl;

    TH1* h1_diff_mass = new TH1F("h1_diff_mass","KFParticle mass - KshortReconstruction mass;#DeltaM [GeV/#it{c}^{2}];Counts",100, -0.03,0.03);
    TH1* h1_diff_dcaz = new TH1F("h1_diff_dcaz","KFParticle dcaz - KshortReconstruction dcaz;DCA_{z}  [cm];Counts",100, -10,10);
    TH1* h1_diff_dcaxy = new TH1F("h1_diff_dcaxy","KFParticle dcaxy - KshortReconstruction dcaxy;DCA_{xy}  [cm];Counts",100, -10,10);
    TH1* h1_diff_dL = new TH1F("h1_diff_dL","KFParticle dL - KshortReconstruction dL;#DeltaL [cm];Counts",100, -10,10);

    int npass0(0), npass1(0), npass2(0), npass3(0), npass4(0), npass5(0);
    int nmatch=0;
    int nevent_kfp = tree_kfp_v2->GetEntries();
    for (int i=0; i<nevent_kfp; i++)
    {
      for (int ii=0; ii<10; ii++) if(i==Int_t(nevent_kfp*0.1*ii)) cout<<"completed "<<Int_t(10*ii)<<"%"<<endl;
      tree_kfp_v2->GetEntry(i);

      std::tuple<int, int, int> key;
      if (track_1_track_ID<track_2_track_ID) key = std::make_tuple(eventNumberKFP,track_1_track_ID,track_2_track_ID);
      else if (track_1_track_ID>track_2_track_ID) key = std::make_tuple(eventNumberKFP,track_2_track_ID,track_1_track_ID);
      auto it = tonyMap.find(key);

      npass0++;
      //if (!(track_1_IP_xy<0.05 && track_2_IP_xy<0.05)) continue;
      npass1++;
      if (K_S0_decayLength<20) continue;
      npass2++;

      if (it != tonyMap.end()) {
        tree_tony_v2->GetEntry(it->second);
        //std::cout << "Key found: " << it->second << std::endl;
        //cout<<"Ks mass: KFP "<<K_S0_mass<<" , KshortReconstruction "<<invariant_mass<<endl;
        h1_diff_mass->Fill(K_S0_mass-invariant_mass);
	h1_diff_dL->Fill(K_S0_decayLength - sqrt(projected_pathlength_x*projected_pathlength_x+projected_pathlength_y*projected_pathlength_y+projected_pathlength_z*projected_pathlength_z));
        if (track_1_track_ID==id1 && track_2_track_ID==id2)
        {
          h1_diff_dcaz->Fill(fabs(track_1_z-primary_vertex_z)-dca3dz1);
          h1_diff_dcaz->Fill(fabs(track_2_z-primary_vertex_z)-dca3dz2);
          h1_diff_dcaxy->Fill(track_1_IP_xy-dca3dxy1);
          h1_diff_dcaxy->Fill(track_2_IP_xy-dca3dxy2);
        }
        else if (track_1_track_ID==id2 && track_2_track_ID==id1)
        {
          h1_diff_dcaz->Fill(fabs(track_1_z-primary_vertex_z)-dca3dz2);
          h1_diff_dcaz->Fill(fabs(track_2_z-primary_vertex_z)-dca3dz1);
          h1_diff_dcaxy->Fill(track_1_IP_xy-dca3dxy2);
          h1_diff_dcaxy->Fill(track_2_IP_xy-dca3dxy1);
        }

        nmatch++;
	npass3++;
      }
    }
    cout<<"number of matched "<<nmatch<<endl;
    cout<<"Within [0.03,0.03] = "<<h1_diff_mass->Integral()<<endl;
    cout<<"all entries in KFP = "<<npass0<<endl;
    cout<<"IP_xy > 0.05 = "<<npass1<<endl;
    cout<<"decayLength>20 = "<<npass2<<endl;
    cout<<"matched = "<<npass3<<endl;

    TCanvas *can = new TCanvas("can","",800,600);
    can->Divide(2,2);
    can->cd(1);
    h1_diff_mass->Draw("e");
    can->cd(2);
    h1_diff_dcaz->Draw("e");
    can->cd(3);
    h1_diff_dcaxy->Draw("e");
    can->cd(4);
    h1_diff_dL->Draw("e");
    can->SaveAs("figure/diff.pdf");


}
