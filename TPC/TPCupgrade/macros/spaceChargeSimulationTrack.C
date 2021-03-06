void CheckCommutator(const char * inputCharge="SpaceCharge.root"){
  //
  // 1.) create an space charge correction map - using as input 2x2d space charge distribution
  // file 
  /*
    const char * inputCharge="SpaceCharge.root"
  */
  AliTPCSpaceCharge3D *spaceCharge = new AliTPCSpaceCharge3D;
  spaceCharge->SetSCDataFileName(inputCharge);
  spaceCharge->SetOmegaTauT1T2(0.325,1,1); // Ne CO2
  //spaceCharge->SetOmegaTauT1T2(0.41,1,1.05); // Ar CO2
  spaceCharge->InitSpaceCharge3DDistortion();
  spaceCharge->CreateHistoSCinZR(0.,50,50)->Draw("surf1");
  spaceCharge->CreateHistoDRPhiinZR(0,100,100)->Draw("colz");
  spaceCharge->AddVisualCorrection(spaceCharge,1);
  //
  // 2.) Instantiate magnetic field
  //
  ocdb="local://$ALICE_ROOT/OCDB/";
  AliCDBManager::Instance()->SetDefaultStorage(ocdb);
  AliCDBManager::Instance()->SetRun(0);   
  TGeoGlobalMagField::Instance()->SetField(new AliMagF("Maps","Maps", 1., 1., AliMagF::k5kG));   
  //
  // 3.)   OCDB correction  have to be initialized from somewhere
  //
  TFile * f  = TFile::Open("Correction.root");
  TObjArray  * array = AliCDBEntry->GetObject();
  AliTPCComposedCorrection * corrDefault = (AliTPCComposedCorrection *)array->At(0);
  corrDefault->Init();
  //
  // 4.) Create comutators
  //
  TObjArray * array_SC_Default  = new TObjArray(2);
  TObjArray * array_Default_SC  = new TObjArray(2);
  //
  array_SC_Default->AddAt(spaceCharge,0);
  array_SC_Default->AddAt(corrDefault,1);
  //
  array_Default_SC->AddAt(corrDefault,0);
  array_Default_SC->AddAt(spaceCharge,1);
   AliTPCComposedCorrection *corr_SC_Default = new AliTPCComposedCorrection(array_SC_Default,AliTPCComposedCorrection::kQueue);
  AliTPCComposedCorrection *corr_Default_SC = new AliTPCComposedCorrection(array_Default_SC,AliTPCComposedCorrection::kQueue);
  corr_SC_Default->AddVisualCorrection(corr_SC_Default,1);
  corr_Default_SC->AddVisualCorrection(corr_Default_SC,2);
  //
  // 5.) Use TF1, TF2 functionality to visualize comutators
  //
  //  
  TCanvas * canvasNonComute = new TCanvas("SpacechargeDefault"," SpacechargeDefault",1200,700);
  canvasNonComute->Divide(2,1); 
  canvasNonComute->cd(1);

  canvasNonComute->cd(1);gPad->SetRightMargin(0.2);
  TF2 * fdiffXY0 = new TF2("fdiffXY0", "(AliTPCCorrection::GetCorrXYZ(x,y,10,0,1)-AliTPCCorrection::GetCorrXYZ(x,y,10,0,2))*(sqrt(x*x+y*y)>85&&sqrt(x*x+y*y)<245)",-250,250,-250,250);
  fdiffXY0->SetNpx(200);
  fdiffXY0->SetNpy(200);
  fdiffXY0->GetXaxis()->SetTitle("x (cm)");
  fdiffXY0->GetYaxis()->SetTitle("y (cm)");
  fdiffXY0->GetZaxis()->SetTitle("#delta R (cm)");
  fdiffXY0->Draw("colz");
  TPaveText *paveText = new TPaveText(-250,200,0.0,250,"");
  paveText->AddText("[SpaceCharge,Default]");
  paveText->Draw();

  
  canvasNonComute->cd(2);gPad->SetRightMargin(0.2);
  TF2 * fdiffXY1 = new TF2("fdiffXY0", "(AliTPCCorrection::GetCorrXYZ(x,y,10,1,1)-AliTPCCorrection::GetCorrXYZ(x,y,10,1,2))*(sqrt(x*x+y*y)>85&&sqrt(x*x+y*y)<245)",-250,250,-250,250);
  fdiffXY1->SetNpx(200);
  fdiffXY1->SetNpy(200);
  fdiffXY1->GetXaxis()->SetTitle("x (cm)");
  fdiffXY1->GetYaxis()->SetTitle("y (cm)");
  fdiffXY1->GetZaxis()->SetTitle("#delta R#Phi (cm)");
  fdiffXY1->Draw("colz");
  //
  //
  //
}




void DrawFuncionIntegralSpaceCharge(){
  //
  // Make nice plot and compare the r and rphi distortion using "Stefan standard implementation" integration  
  // and using integration along trajectories
  //
  AliTPCSpaceCharge3D *spaceCharge = new AliTPCSpaceCharge3D;
  spaceCharge->SetSCDataFileName("SpaceCharge.root");
  spaceCharge->SetOmegaTauT1T2(0.325,1,1); // Ne CO2
  //spaceCharge->SetOmegaTauT1T2(0.41,1,1.05); // Ar CO2
  spaceCharge->InitSpaceCharge3DDistortion();
  spaceCharge->CreateHistoSCinZR(0.,50,50)->Draw("surf1");
  spaceCharge->CreateHistoDRPhiinZR(0,100,100)->Draw("colz"); 
  ocdb="local://$ALICE_ROOT/OCDB/";
  AliCDBManager::Instance()->SetDefaultStorage(ocdb);
  AliCDBManager::Instance()->SetRun(0);   
  TGeoGlobalMagField::Instance()->SetField(new AliMagF("Maps","Maps", 1., 1., AliMagF::k5kG));   
  spaceCharge->AddVisualCorrection(spaceCharge,1); 

  //
  //
  TCanvas *canvasIntegrate = new TCanvas("canvasIntegrate","canvasIntegrate",600,600);
  canvasIntegrate->Divide(1,2);
  //
  TF1 *fdistRStefan = new TF1("fdistRStefan","AliTPCCorrection::GetCorrXYZ(x,0,10,0,1)",85,250);
  TF1 *fdistRDriftS5 = new TF1("fdistRDriftS3","AliTPCCorrection::GetCorrXYZIntegrateZ(x,0,10,0,1,5)",85,250);
  TF1 *fdistRDriftS2 = new TF1("fdistRDriftS2","AliTPCCorrection::GetCorrXYZIntegrateZ(x,0,10,0,1,2)",85,250);
  TF1 *fdistRDriftS1 = new TF1("fdistRDriftS1","AliTPCCorrection::GetCorrXYZIntegrateZ(x,0,10,0,1,1)",85,250);
  TF1 *fdistRPhiStefan = new TF1("fdistRPhiStefan","AliTPCCorrection::GetCorrXYZ(x,0,10,1,1)",85,250);
  TF1 *fdistRPhiDriftS5 = new TF1("fdistRPhiDriftS3","AliTPCCorrection::GetCorrXYZIntegrateZ(x,0,10,1,1,5)",85,250);
  TF1 *fdistRPhiDriftS2 = new TF1("fdistRPhiDriftS2","AliTPCCorrection::GetCorrXYZIntegrateZ(x,0,10,1,1,2)",85,250);
  TF1 *fdistRPhiDriftS1 = new TF1("fdistRPhiDriftS1","AliTPCCorrection::GetCorrXYZIntegrateZ(x,0,10,1,1,1)",85,250);
  canvasIntegrate->cd(1);
  TLegend * legendR=new TLegend(0.4,0.6,0.9,0.9,"Space charge #Delta_{R}");
  fdistRStefan->GetXaxis()->SetTitle("R (cm)");
  fdistRStefan->GetYaxis()->SetTitle("#Delta_{R} (cm)");
  fdistRStefan->SetLineColor(2);
  //
  fdistRDriftS5->SetLineColor(1); fdistRDriftS5->SetLineStyle(2); 
  fdistRDriftS2->SetLineColor(4); fdistRDriftS2->SetLineStyle(1); 
  fdistRDriftS1->SetLineColor(1); fdistRDriftS1->SetLineStyle(3);
  fdistRStefan->Draw();
  fdistRDriftS5->Draw("same");
  fdistRDriftS2->Draw("same");
  fdistRDriftS1->Draw("same");
  legendR->AddEntry(fdistRStefan,"Stefan integration");
  legendR->AddEntry(fdistRDriftS5,"Drift lines (step=5cm);");
  legendR->AddEntry(fdistRDriftS2,"Drift lines (step=2cm);");
  legendR->AddEntry(fdistRDriftS1,"Drift lines (step=1cm);");
  legendR->Draw();

  canvasIntegrate->cd(2);  
  fdistRPhiStefan->GetXaxis()->SetTitle("RPhi (cm)");
  fdistRPhiStefan->GetYaxis()->SetTitle("#Delta_{RPhi} (cm)");
  fdistRPhiStefan->SetLineColor(2);
  //
  fdistRPhiDriftS5->SetLineColor(4); fdistRPhiDriftS5->SetLineStyle(2); 
  fdistRPhiDriftS2->SetLineColor(4); 
  fdistRPhiDriftS1->SetLineColor(4); fdistRPhiDriftS1->SetLineStyle(2);
  fdistRPhiStefan->Draw();
  fdistRPhiDriftS5->Draw("same");
  fdistRPhiDriftS2->Draw("same");
  fdistRPhiDriftS1->Draw("same");
  
  TLegend * legendRPhi=new TLegend(0.4,0.6,0.9,0.9,"Space charge #Delta_{RPhi}");
  fdistRPhiStefan->GetXaxis()->SetTitle("RPhi (cm)");
  fdistRPhiStefan->GetYaxis()->SetTitle("#Delta_{RPhi} (cm)");
  fdistRPhiStefan->SetLineColor(2);
 legendRPhi->AddEntry(fdistRPhiStefan,"Stefan integration");
  legendRPhi->AddEntry(fdistRPhiDriftS5,"Drift lines (step=5cm);");
  legendRPhi->AddEntry(fdistRPhiDriftS2,"Drift lines (step=2cm);");
  legendRPhi->AddEntry(fdistRPhiDriftS1,"Drift lines (step=1cm);");
  legendRPhi->Draw();
  canvasIntegrate->SaveAs("canvasIntegrate.png");
}

void DrawFuncions(){
  //
  // Make a default plot for the  
  //
  AliTPCSpaceCharge3D *spaceCharge = new AliTPCSpaceCharge3D;
  spaceCharge->SetSCDataFileName("SpaceCharge.root");
  spaceCharge->SetOmegaTauT1T2(0.325,1,1); // Ne CO2
  //spaceCharge->SetOmegaTauT1T2(0.41,1,1.05); // Ar CO2
  spaceCharge->InitSpaceCharge3DDistortion();
  spaceCharge->CreateHistoSCinZR(0.,50,50)->Draw("surf1");
  spaceCharge->CreateHistoDRPhiinZR(0,100,100)->Draw("colz"); 
  ocdb="local://$ALICE_ROOT/OCDB/";
  AliCDBManager::Instance()->SetDefaultStorage(ocdb);
  AliCDBManager::Instance()->SetRun(0);   
  TGeoGlobalMagField::Instance()->SetField(new AliMagF("Maps","Maps", 1., 1., AliMagF::k5kG));   
  spaceCharge->AddVisualCorrection(spaceCharge,1); 

  TCanvas *canvasFun = new TCanvas("canvasFun","canvasFun",600,500);
  gStyle->SetOptTitle(1);
  TF1 * fdiffR = new TF1("fdiffR", "(AliTPCCorrection::GetCorrXYZ(x,0,10,0,1)-AliTPCCorrection::GetCorrXYZ(x+1,0,10,0,1))",80,245);
  fdiffR->SetNpx(1000);
  fdiffR->GetXaxis()->SetTitle("R (cm)");
  fdiffR->GetYaxis()->SetTitle("#Delta_{R}(R)-#Delta_{R}(R+1) (cm)");
  fdiffR->Draw();
  canvasFun->SaveAs("radialShrinking.png");

  TF1 * fdiffSigmaR = new TF1("fdiffSigmaR", "(AliTPCCorrection::GetCorrXYZ(x,0,10,0,1)-AliTPCCorrection::GetCorrXYZ(x,0,10-7,0,1))",80,245);
  fdiffSigmaR->SetNpx(1000);
  fdiffSigmaR->GetXaxis()->SetTitle("R (cm)");
  fdiffSigmaR->GetYaxis()->SetTitle("#Delta_{R}(R,Z)-#Delta_{R}(R,Z+#sigma_{z}) (cm)");
  fdiffSigmaR->Draw();
  canvasFun->SaveAs("radialSigmaR.png");
  //
  TF1 * fdistortion = new TF1("fdiffSigmaR", "AliTPCCorrection::GetCorrXYZ(x,0,10,0,1)+x",80,245);
  fdistortion->SetNpx(1000);
  fdistortion->GetXaxis()->SetTitle("R (cm)");
  fdistortion->GetYaxis()->SetTitle("R' (cm)");
  fdistortion->Draw();  
  canvasFun->SaveAs("radialDistortion.png");  
}


void spaceChargeSimulationTrack(Double_t refX){
  //
  // 1. Initialzation form space charge maps
  //
  AliTPCSpaceCharge3D *spaceCharge = new AliTPCSpaceCharge3D;
  spaceCharge->SetSCDataFileName("SpaceCharge.root");
  spaceCharge->SetOmegaTauT1T2(0.325,1,1); // Ne CO2
  //spaceCharge->SetOmegaTauT1T2(0.41,1,1.05); // Ar CO2
  spaceCharge->InitSpaceCharge3DDistortion();
  spaceCharge->CreateHistoSCinZR(0.,50,50)->Draw("surf1");
  spaceCharge->CreateHistoDRPhiinZR(0,100,100)->Draw("colz"); 
  ocdb="local://$ALICE_ROOT/OCDB/";
  AliCDBManager::Instance()->SetDefaultStorage(ocdb);
  AliCDBManager::Instance()->SetRun(0);   
  TGeoGlobalMagField::Instance()->SetField(new AliMagF("Maps","Maps", 1., 1., AliMagF::k5kG));   
  spaceCharge->AddVisualCorrection(spaceCharge,1); 
  TF1 * fdiffR = new TF1("fdiffR", "(AliTPCCorrection::GetCorrXYZ(x,0,10,0,1)-AliTPCCorrection::GetCorrXYZ(x+1,0,10,0,1))",80,245);
  //
  // 2. MC generation of the tracks
  //    Distort track and fit distroted track within AliTPCorrection::FitDistortedTrack(*t, refX, dir,  pcstream)
  //
  Double_t etaCuts=1;
  Double_t mass = TDatabasePDG::Instance()->GetParticle("pi+")->Mass();
  TF1 fpt("fpt",Form("x*(1+(sqrt(x*x+%f^2)-%f)/([0]*[1]))^(-[0])",mass,mass),0.4,10);
  fpt.SetParameters(7.24,0.120);
  fpt.SetNpx(10000);
  Int_t nTracks=10000; 
  TTreeSRedirector  * pcstream = new TTreeSRedirector("trackDist.root","recreate");
  
  for(Int_t nt=0; nt<nTracks; nt++){
    Double_t phi = gRandom->Uniform(0.0, 2*TMath::Pi());
    Double_t eta = gRandom->Uniform(-etaCuts, etaCuts);
    Double_t pt = 1/(gRandom->Rndm()*5+0.00001); // momentum for f1
    //   printf("phi %lf  eta %lf pt %lf\n",phi,eta,pt);
    Short_t sign=1;
    if(gRandom->Rndm() < 0.5){
      sign =1;
    }else{
      sign=-1;
    }
    
    Double_t theta = 2*TMath::ATan(TMath::Exp(-eta))-TMath::Pi()/2.;
    Double_t pxyz[3];
    pxyz[0]=pt*TMath::Cos(phi);
    pxyz[1]=pt*TMath::Sin(phi);
    pxyz[2]=pt*TMath::Tan(theta);
    Double_t vertex[3]={0,0,0};
    Double_t cv[21]={0};
    AliExternalTrackParam *t= new AliExternalTrackParam(vertex, pxyz, cv, sign);   
    //    Double_t refX=1.;
    Int_t dir=-1;
    AliExternalTrackParam *td =  spaceCharge->FitDistortedTrack(*t, refX, dir,  pcstream);
  }  
  delete pcstream;
  //
  // Visulalize distortion of of tracks
  //
  TFile * f = TFile::Open("trackDist.root");
  TTree * tree = (TTree*)f->Get("fitDistortSpaceCharge3D");
  tree->SetMarkerStyle(25);
  tree->SetMarkerSize(0.4);
  
  //
  // DCA distortion example
  //
  tree->Draw("track1.GetY()-track0.GetY():track0.GetTgl():abs(track0.fP[4])","track0.fP[4]>0","colz",10000);
  tree->Draw("track1.GetTgl()-track0.GetTgl():track0.GetSigned1Pt():track0.GetTgl()","abs(track0.GetTgl()-0.5)<0.4","colz",10000);

  /*
    problems:
    
   */
}

/*
 */

void DrawDistortions(){
  //
  //
  //
  TFile * f = TFile::Open("trackDist.root");
  TTree * tree = (TTree*)f->Get("fitDistortSpaceCharge3D");
  tree->SetMarkerStyle(25);
  tree->SetMarkerSize(0.4);  

  //
  // DCA as function of theta and 1/pt 
  //
  TCanvas *c1 = new TCanvas();
  tree->Draw("track1.GetY()-track0.GetY():track0.GetSigned1Pt():track0.GetTgl()","track0.GetTgl()>0","colz",10000);
  // Get the histogram and set axis titles
  TH2F *htemp = (TH2F*)gPad->GetPrimitive("htemp");
  htemp->GetXaxis()->SetTitle("1/p_{T}");
  htemp->GetYaxis()->SetTitle("#Delta DCA");
  htemp->GetZaxis()->SetTitle("tan(#lambda)");
  htemp->SetTitle("DCA difference");
  c1->Update();
  c1->SaveAs("DCAdifference.eps")

  //
  tree->Draw("track1.GetSigned1Pt()-track0.GetSigned1Pt():track0.GetSigned1Pt():track0.GetTgl()","track0.GetTgl()>0","colz",10000);
  htemp = (TH2F*)gPad->GetPrimitive("htemp");
  htemp->GetXaxis()->SetTitle("1/p_{T}");
  htemp->GetYaxis()->SetTitle("#Delta 1/pT (1/GeV/c)");
  htemp->GetZaxis()->SetTitle("tan(#lambda)");
  htemp->SetTitle("1/pT difference");
  c1->Update();
  c1->SaveAs("1Ptdifference.eps")
  //
  // 
  TCanvas *canvasEvent = new TCanvas("canvasEvent","canvasEvent",600,500);
  tree->SetMarkerSize(0.25);
  tree->SetMarkerColor(1);
  tree->Draw("point0.fY:point0.fX","point0.fX!=0","",40,0);
  tree->SetMarkerColor(2);
  tree->Draw("point1.fY:point1.fX","point1.fX!=0","same",40,0);
  htemp = (TH2F*)gPad->GetPrimitive("htemp");
  htemp->GetXaxis()->SetTitle("x (cm)");
  htemp->GetYaxis()->SetTitle("y (cm)");
  htemp->SetTitle("Event");
  c1->Update();
  c1->SaveAs("Event.eps")

}
