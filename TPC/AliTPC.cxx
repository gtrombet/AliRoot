/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/*
$Log$
Revision 1.19.2.4  2000/06/26 07:39:42  kowal2
Changes to obey the coding rules

Revision 1.19.2.3  2000/06/25 08:38:41  kowal2
Splitted from AliTPCtracking

Revision 1.19.2.2  2000/06/16 12:59:28  kowal2
Changed parameter settings

Revision 1.19.2.1  2000/06/09 07:15:07  kowal2

Defaults loaded automatically (hard-wired)
Optional parameters can be set via macro called in the constructor

Revision 1.19  2000/04/18 19:00:59  fca
Small bug fixes to TPC files

Revision 1.18  2000/04/17 09:37:33  kowal2
removed obsolete AliTPCDigitsDisplay.C

Revision 1.17.2.2  2000/04/10 08:15:12  kowal2

New, experimental data structure from M. Ivanov
New tracking algorithm
Different pad geometry for different sectors
Digitization rewritten

Revision 1.17.2.1  2000/04/10 07:56:53  kowal2
Not used anymore - removed

Revision 1.17  2000/01/19 17:17:30  fca
Introducing a list of lists of hits -- more hits allowed for detector now

Revision 1.16  1999/11/05 09:29:23  fca
Accept only signals > 0

Revision 1.15  1999/10/08 06:26:53  fca
Removed ClustersIndex - not used anymore

Revision 1.14  1999/09/29 09:24:33  fca
Introduction of the Copyright and cvs Log

*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Time Projection Chamber                                                  //
//  This class contains the basic functions for the Time Projection Chamber  //
//  detector. Functions specific to one particular geometry are              //
//  contained in the derived classes                                         //
//                                                                           //
//Begin_Html
/*
<img src="picts/AliTPCClass.gif">
*/
//End_Html
//                                                                           //
//                                                                          //
///////////////////////////////////////////////////////////////////////////////

//

#include <TMath.h>
#include <TRandom.h>
#include <TVector.h>
#include <TMatrix.h>
#include <TGeometry.h>
#include <TNode.h>
#include <TTUBS.h>
#include <TObjectTable.h>
#include "TParticle.h"
#include "AliTPC.h"
#include <TFile.h>       
#include "AliRun.h"
#include <iostream.h>
#include <fstream.h>
#include "AliMC.h"


#include "AliTPCParamSR.h"
#include "AliTPCPRF2D.h"
#include "AliTPCRF1D.h"
#include "AliDigits.h"
#include "AliSimDigits.h"

#include "AliTPCDigitsArray.h"
#include "AliCluster.h"
#include "AliClusters.h"
#include "AliTPCClustersRow.h"
#include "AliTPCClustersArray.h"

#include "AliTPCcluster.h"
#include "AliTPCclusterer.h"
#include "AliTPCtracker.h"

#include <TInterpreter.h>

ClassImp(AliTPC) 

//_____________________________________________________________________________
AliTPC::AliTPC()
{
  //
  // Default constructor
  //
  fIshunt   = 0;
  fHits     = 0;
  fDigits   = 0;
  fNsectors = 0;
  //MI changes
  fDigitsArray = 0;
  fClustersArray = 0;
  fTPCParam=0;
}
 
//_____________________________________________________________________________
AliTPC::AliTPC(const char *name, const char *title)
      : AliDetector(name,title)
{
  //
  // Standard constructor
  //

  //
  // Initialise arrays of hits and digits 
  fHits     = new TClonesArray("AliTPChit",  176);
  gAlice->AddHitList(fHits);
  //MI change  
  fDigitsArray = 0;
  fClustersArray= 0;
  //
  // Initialise counters
  fNsectors = 0;

  //
  fIshunt     =  0;
  //
  // Initialise color attributes
  SetMarkerColor(kYellow);

  //
  //  Set TPC parameters
  //

  if (!strcmp(title,"Default")) {  
     fTPCParam = new AliTPCParamSR;
  } else {
    cerr<<"AliTPC warning: in Config.C you must set non-default parameters\n";
    fTPCParam=0;
  }

}

//_____________________________________________________________________________
AliTPC::~AliTPC()
{
  //
  // TPC destructor
  //

  fIshunt   = 0;
  delete fHits;
  delete fDigits;
  delete fTPCParam;
}

//_____________________________________________________________________________
void AliTPC::AddHit(Int_t track, Int_t *vol, Float_t *hits)
{
  //
  // Add a hit to the list
  //
  TClonesArray &lhits = *fHits;
  new(lhits[fNhits++]) AliTPChit(fIshunt,track,vol,hits);
}
 
//_____________________________________________________________________________
void AliTPC::BuildGeometry()
{

  //
  // Build TPC ROOT TNode geometry for the event display
  //
  TNode *nNode, *nTop;
  TTUBS *tubs;
  Int_t i;
  const int kColorTPC=19;
  char name[5], title[25];
  const Double_t kDegrad=TMath::Pi()/180;
  const Double_t kRaddeg=180./TMath::Pi();


  Float_t innerOpenAngle = fTPCParam->GetInnerAngle();
  Float_t outerOpenAngle = fTPCParam->GetOuterAngle();

  Float_t innerAngleShift = fTPCParam->GetInnerAngleShift();
  Float_t outerAngleShift = fTPCParam->GetOuterAngleShift();

  Int_t nLo = fTPCParam->GetNInnerSector()/2;
  Int_t nHi = fTPCParam->GetNOuterSector()/2;  

  const Double_t kloAng = (Double_t)TMath::Nint(innerOpenAngle*kRaddeg);
  const Double_t khiAng = (Double_t)TMath::Nint(outerOpenAngle*kRaddeg);
  const Double_t kloAngSh = (Double_t)TMath::Nint(innerAngleShift*kRaddeg);
  const Double_t khiAngSh = (Double_t)TMath::Nint(outerAngleShift*kRaddeg);  


  const Double_t kloCorr = 1/TMath::Cos(0.5*kloAng*kDegrad);
  const Double_t khiCorr = 1/TMath::Cos(0.5*khiAng*kDegrad);

  Double_t rl,ru;
  

  //
  // Get ALICE top node
  //

  nTop=gAlice->GetGeometry()->GetNode("alice");

  //  inner sectors

  rl = fTPCParam->GetInnerRadiusLow();
  ru = fTPCParam->GetInnerRadiusUp();
 

  for(i=0;i<nLo;i++) {
    sprintf(name,"LS%2.2d",i);
    name[4]='\0';
    sprintf(title,"TPC low sector %3d",i);
    title[24]='\0';
    
    tubs = new TTUBS(name,title,"void",rl*kloCorr,ru*kloCorr,250.,
                     kloAng*(i-0.5)+kloAngSh,kloAng*(i+0.5)+kloAngSh);
    tubs->SetNumberOfDivisions(1);
    nTop->cd();
    nNode = new TNode(name,title,name,0,0,0,"");
    nNode->SetLineColor(kColorTPC);
    fNodes->Add(nNode);
  }

  // Outer sectors

  rl = fTPCParam->GetOuterRadiusLow();
  ru = fTPCParam->GetOuterRadiusUp();

  for(i=0;i<nHi;i++) {
    sprintf(name,"US%2.2d",i);
    name[4]='\0';
    sprintf(title,"TPC upper sector %d",i);
    title[24]='\0';
    tubs = new TTUBS(name,title,"void",rl*khiCorr,ru*khiCorr,250,
                     khiAng*(i-0.5)+khiAngSh,khiAng*(i+0.5)+khiAngSh);
    tubs->SetNumberOfDivisions(1);
    nTop->cd();
    nNode = new TNode(name,title,name,0,0,0,"");
    nNode->SetLineColor(kColorTPC);
    fNodes->Add(nNode);
  }

}    

//_____________________________________________________________________________
Int_t AliTPC::DistancetoPrimitive(Int_t , Int_t )
{
  //
  // Calculate distance from TPC to mouse on the display
  // Dummy procedure
  //
  return 9999;
}

void AliTPC::Clusters2Tracks(TFile *of) {
  //-----------------------------------------------------------------
  // This is a track finder.
  //-----------------------------------------------------------------
  AliTPCtracker::Clusters2Tracks(fTPCParam,of);
}

//_____________________________________________________________________________
void AliTPC::CreateMaterials()
{
  //-----------------------------------------------
  // Create Materials for for TPC
  //-----------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  Int_t iSXFLD=gAlice->Field()->Integ();
  Float_t sXMGMX=gAlice->Field()->Max();

  Float_t amat[5]; // atomic numbers
  Float_t zmat[5]; // z
  Float_t wmat[5]; // proportions

  Float_t density;

  //  ********************* Gases *******************

  //--------------------------------------------------------------
  // pure gases
  //--------------------------------------------------------------

  // Ne


  Float_t aNe = 20.18;
  Float_t zNe = 10.;
  
  density = 0.0009;

  AliMaterial(20,"Ne",aNe,zNe,density,999.,999.);

  // Ar

  Float_t aAr = 39.948;
  Float_t zAr = 18.;

  density = 0.001782;
 
  AliMaterial(21,"Ar",aAr,zAr,density,999.,999.);

  Float_t aPure[2];
  
  aPure[0] = aNe;
  aPure[1] = aAr;
  

  //--------------------------------------------------------------
  // gases - compounds
  //--------------------------------------------------------------

  Float_t amol[3];

  //  CO2

  amat[0]=12.011;
  amat[1]=15.9994;

  zmat[0]=6.;
  zmat[1]=8.;

  wmat[0]=1.;
  wmat[1]=2.;

  density=0.001977;

  amol[0] = amat[0]*wmat[0]+amat[1]*wmat[1];

  AliMixture(10,"CO2",amat,zmat,density,-2,wmat);

  // CF4

  amat[0]=12.011;
  amat[1]=18.998;

  zmat[0]=6.;
  zmat[1]=9.;
 
  wmat[0]=1.;
  wmat[1]=4.;
 
  density=0.003034;

  amol[1] = amat[0]*wmat[0]+amat[1]*wmat[1];

  AliMixture(11,"CF4",amat,zmat,density,-2,wmat); 

  // CH4

  amat[0]=12.011;
  amat[1]=1.;

  zmat[0]=6.;
  zmat[1]=1.;

  wmat[0]=1.;
  wmat[1]=4.;

  density=0.000717;

  amol[2] = amat[0]*wmat[0]+amat[1]*wmat[1];

  AliMixture(12,"CH4",amat,zmat,density,-2,wmat);

  //----------------------------------------------------------------
  // gases - mixtures, ID >= 20 pure gases, <= 10 ID < 20 -compounds
  //----------------------------------------------------------------
 
  char namate[21];
 
  density = 0.;
  Float_t am=0;
  Int_t nc;

  Float_t a,z,rho,absl,x0,buf[1];
  Int_t nbuf;

  for(nc = 0;nc<fNoComp;nc++)
    {
    
      // retrive material constants
      
      gMC->Gfmate((*fIdmate)[fMixtComp[nc]],namate,a,z,rho,x0,absl,buf,nbuf);

      amat[nc] = a;
      zmat[nc] = z;

      Int_t nnc = (fMixtComp[nc]>=20) ? fMixtComp[nc]%20 : fMixtComp[nc]%10;
 
      am += fMixtProp[nc]*((fMixtComp[nc]>=20) ? aPure[nnc] : amol[nnc]); 
      density += fMixtProp[nc]*rho;  // density of the mixture
      
    }

  // mixture proportions by weight!

  for(nc = 0;nc<fNoComp;nc++)
    {

      Int_t nnc = (fMixtComp[nc]>=20) ? fMixtComp[nc]%20 : fMixtComp[nc]%10;

      wmat[nc] = fMixtProp[nc]*((fMixtComp[nc]>=20) ? aPure[nnc] : amol[nnc])/am;

    }  
  
  AliMixture(31,"Drift gas 1",amat,zmat,density,fNoComp,wmat);
  AliMixture(32,"Drift gas 2",amat,zmat,density,fNoComp,wmat);
  AliMixture(33,"Drift gas 3",amat,zmat,density,fNoComp,wmat); 

  AliMedium(2, "Drift gas 1", 31, 0, iSXFLD, sXMGMX, 10., 999.,.1, .001, .001);
  AliMedium(3, "Drift gas 2", 32, 0, iSXFLD, sXMGMX, 10., 999.,.1, .001, .001);
  AliMedium(4, "Drift gas 3", 33, 1, iSXFLD, sXMGMX, 10., 999.,.1, .001, .001);

  // Air 

  AliMaterial(24, "Air", 14.61, 7.3, .001205, 30420., 67500.);

  AliMedium(24, "Air", 24, 0, iSXFLD, sXMGMX, 10., .1, .1, .1, .1);

  //----------------------------------------------------------------------
  //               solid materials
  //----------------------------------------------------------------------

  // Al

  AliMaterial(30, "Al", 26.98, 13., 2.7, 8.9, 37.2);

  AliMedium(0, "Al",30, 0, iSXFLD, sXMGMX, 10., .1, .1, .1,   .1);

  // Si

  AliMaterial(31, "Si", 28.086, 14.,2.33, 9.36, 999.);

  AliMedium(7, "Al",31, 0, iSXFLD, sXMGMX, 10., .1, .1, .1,   .1);
  

  // Mylar C5H4O2

  amat[0]=12.011;
  amat[1]=1.;
  amat[2]=15.9994;

  zmat[0]=6.;
  zmat[1]=1.;
  zmat[2]=8.;

  wmat[0]=5.;
  wmat[1]=4.;
  wmat[2]=2.; 

  density = 1.39;
  
  AliMixture(32, "Mylar",amat,zmat,density,-3,wmat);

  AliMedium(5, "Mylar",32, 0, iSXFLD, sXMGMX, 10., .1, .1, .001, .01);




  // Carbon (normal)

  AliMaterial(33,"C normal",12.011,6.,2.265,18.8,999.);

  AliMedium(6,"C normal",33,0, iSXFLD, sXMGMX, 10., .1, .1, .001, .01);

  // G10 for inner and outr field cage
  // G10 is 60% SiO2 + 40% epoxy, right now I use A and Z for SiO2

  Float_t rhoFactor;

  amat[0]=28.086;
  amat[1]=15.9994;

  zmat[0]=14.;
  zmat[1]=8.;

  wmat[0]=1.;
  wmat[1]=2.;

  density = 1.7;
  

  AliMixture(34,"G10 aux.",amat,zmat,density,-2,wmat);


  gMC->Gfmate((*fIdmate)[34],namate,a,z,rho,x0,absl,buf,nbuf);

  Float_t thickX0 = 0.0052; // field cage in X0 units
  
  Float_t thick = 2.; // in cm

  x0=19.4; // G10 

  rhoFactor = x0*thickX0/thick;
  density = rho*rhoFactor;

  AliMaterial(35,"G10-fc",a,z,density,999.,999.);

  AliMedium(8,"G10-fc",35,0, iSXFLD, sXMGMX, 10., .1, .1, .001, .01);

  thickX0 = 0.0027; // inner vessel (eta <0.9)
  thick=0.5;
  rhoFactor = x0*thickX0/thick;
  density = rho*rhoFactor;

  AliMaterial(36,"G10-iv",a,z,density,999.,999.);  

  AliMedium(9,"G10-iv",36,0, iSXFLD, sXMGMX, 10., .1, .1, .001, .01);

  //  Carbon fibre  
  
  gMC->Gfmate((*fIdmate)[33],namate,a,z,rho,x0,absl,buf,nbuf);

  thickX0 = 0.0133; // outer vessel
  thick=3.0;
  rhoFactor = x0*thickX0/thick;
  density = rho*rhoFactor;


  AliMaterial(37,"C-ov",a,z,density,999.,999.);

  AliMedium(10,"C-ov",37,0, iSXFLD, sXMGMX, 10., .1, .1, .001, .01);  

  thickX0=0.015; // inner vessel (cone, eta > 0.9)
  thick=1.5;
  rhoFactor = x0*thickX0/thick;
  density = rho*rhoFactor;

  AliMaterial(38,"C-ivc",a,z,density,999.,999.);

  AliMedium(11,"C-ivc",38,0, iSXFLD, sXMGMX, 10., .1, .1, .001, .01);

  //

  AliMedium(12,"CO2",10,0, iSXFLD, sXMGMX, 10., 999.,.1, .001, .001);
    
}


void AliTPC::Digits2Clusters(TFile *of)
{
  //-----------------------------------------------------------------
  // This is a simple cluster finder.
  //-----------------------------------------------------------------
  AliTPCclusterer::Digits2Clusters(fTPCParam,of);
}

extern Double_t SigmaY2(Double_t, Double_t, Double_t);
extern Double_t SigmaZ2(Double_t, Double_t);
//_____________________________________________________________________________
void AliTPC::Hits2Clusters(TFile *of)
{
  //--------------------------------------------------------
  // TPC simple cluster generator from hits
  // obtained from the TPC Fast Simulator
  // The point errors are taken from the parametrization
  //--------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------
  // Adopted to Marian's cluster data structure by I.Belikov, CERN,
  // Jouri.Belikov@cern.ch
  //----------------------------------------------------------------
  
  /////////////////////////////////////////////////////////////////////////////
  //
  //---------------------------------------------------------------------
  //   ALICE TPC Cluster Parameters
  //--------------------------------------------------------------------
       
  

  // Cluster width in rphi
  const Float_t kACrphi=0.18322;
  const Float_t kBCrphi=0.59551e-3;
  const Float_t kCCrphi=0.60952e-1;
  // Cluster width in z
  const Float_t kACz=0.19081;
  const Float_t kBCz=0.55938e-3;
  const Float_t kCCz=0.30428;

  TDirectory *savedir=gDirectory; 

  if (!of->IsOpen()) {
     cerr<<"AliTPC::Hits2Clusters(): output file not open !\n";
     return;
  }

   if(fTPCParam == 0){
     printf("AliTPCParam MUST be created firstly\n");
     return;
   }

  Float_t sigmaRphi,sigmaZ,clRphi,clZ;
  //
  TParticle *particle; // pointer to a given particle
  AliTPChit *tpcHit; // pointer to a sigle TPC hit
  TClonesArray *particles; //pointer to the particle list
  Int_t sector,nhits;
  Int_t ipart;
  Float_t xyz[5];
  Float_t pl,pt,tanth,rpad,ratio;
  Float_t cph,sph;
  
  //---------------------------------------------------------------
  //  Get the access to the tracks 
  //---------------------------------------------------------------
  
  TTree *tH = gAlice->TreeH();
  Stat_t ntracks = tH->GetEntries();
  particles=gAlice->Particles();

  //Switch to the output file
  of->cd();

  fTPCParam->Write(fTPCParam->GetTitle());
  AliTPCClustersArray carray;
  carray.Setup(fTPCParam);
  carray.SetClusterType("AliTPCcluster");
  carray.MakeTree();

  Int_t nclusters=0; //cluster counter
  
  //------------------------------------------------------------
  // Loop over all sectors (72 sectors for 20 deg
  // segmentation for both lower and upper sectors)
  // Sectors 0-35 are lower sectors, 0-17 z>0, 17-35 z<0
  // Sectors 36-71 are upper sectors, 36-53 z>0, 54-71 z<0
  //
  // First cluster for sector 0 starts at "0"
  //------------------------------------------------------------
   
  for(Int_t isec=0;isec<fTPCParam->GetNSector();isec++){
    //MI change
    fTPCParam->AdjustCosSin(isec,cph,sph);
    
    //------------------------------------------------------------
    // Loop over tracks
    //------------------------------------------------------------
    
    for(Int_t track=0;track<ntracks;track++){
      ResetHits();
      tH->GetEvent(track);
      //
      //  Get number of the TPC hits
      //
      nhits=fHits->GetEntriesFast();
      //
      // Loop over hits
      //
      for(Int_t hit=0;hit<nhits;hit++){
	tpcHit=(AliTPChit*)fHits->UncheckedAt(hit);
        if (tpcHit->fQ == 0.) continue; //information about track (I.Belikov)
	sector=tpcHit->fSector; // sector number
	if(sector != isec) continue; //terminate iteration
	ipart=tpcHit->fTrack;
	particle=(TParticle*)particles->UncheckedAt(ipart);
	pl=particle->Pz();
	pt=particle->Pt();
	if(pt < 1.e-9) pt=1.e-9;
	tanth=pl/pt;
	tanth = TMath::Abs(tanth);
	rpad=TMath::Sqrt(tpcHit->fX*tpcHit->fX + tpcHit->fY*tpcHit->fY);
	ratio=0.001*rpad/pt; // pt must be in MeV/c - historical reason

	//   space-point resolutions
	
	sigmaRphi=SigmaY2(rpad,tanth,pt);
	sigmaZ   =SigmaZ2(rpad,tanth   );
	
	//   cluster widths
	
	clRphi=kACrphi-kBCrphi*rpad*tanth+kCCrphi*ratio*ratio;
	clZ=kACz-kBCz*rpad*tanth+kCCz*tanth*tanth;
	
	// temporary protection
	
	if(sigmaRphi < 0.) sigmaRphi=0.4e-3;
	if(sigmaZ < 0.) sigmaZ=0.4e-3;
	if(clRphi < 0.) clRphi=2.5e-3;
	if(clZ < 0.) clZ=2.5e-5;
	
	//
	
	//
	// smearing --> rotate to the 1 (13) or to the 25 (49) sector,
	// then the inaccuracy in a X-Y plane is only along Y (pad row)!
	//
        Float_t xprim= tpcHit->fX*cph + tpcHit->fY*sph;
	Float_t yprim=-tpcHit->fX*sph + tpcHit->fY*cph;
	xyz[0]=gRandom->Gaus(yprim,TMath::Sqrt(sigmaRphi));   // y
          Float_t alpha=(isec < fTPCParam->GetNInnerSector()) ?
	  fTPCParam->GetInnerAngle() : fTPCParam->GetOuterAngle();
          Float_t ymax=xprim*TMath::Tan(0.5*alpha);
          if (TMath::Abs(xyz[0])>ymax) xyz[0]=yprim; 
	xyz[1]=gRandom->Gaus(tpcHit->fZ,TMath::Sqrt(sigmaZ)); // z
          if (TMath::Abs(xyz[1])>fTPCParam->GetZLength()) xyz[1]=tpcHit->fZ; 
	xyz[2]=tpcHit->fQ;                                     // q
	xyz[3]=sigmaRphi;                                     // fSigmaY2
	xyz[4]=sigmaZ;                                        // fSigmaZ2

        AliTPCClustersRow *clrow=carray.GetRow(sector,tpcHit->fPadRow);
        if (!clrow) clrow=carray.CreateRow(sector,tpcHit->fPadRow);	

        Int_t tracks[3]={tpcHit->fTrack, -1, -1};
	AliTPCcluster cluster(xyz,tracks);

        clrow->InsertCluster(&cluster); nclusters++;

      } // end of loop over hits

    }   // end of loop over tracks

    Int_t nrows=fTPCParam->GetNRow(isec);
    for (Int_t irow=0; irow<nrows; irow++) {
        AliTPCClustersRow *clrow=carray.GetRow(isec,irow);
        if (!clrow) continue;
        carray.StoreRow(isec,irow);
        carray.ClearRow(isec,irow);
    }

  } // end of loop over sectors  

  cerr<<"Number of made clusters : "<<nclusters<<"                        \n";

  carray.GetTree()->Write();

  savedir->cd(); //switch back to the input file
  
} // end of function

//_________________________________________________________________
void AliTPC::Hits2ExactClustersSector(Int_t isec)
{
  //--------------------------------------------------------
  //calculate exact cross point of track and given pad row
  //resulting values are expressed in "digit" coordinata
  //--------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marian Ivanov  GSI Darmstadt, m.ivanov@gsi.de
  //-----------------------------------------------------------------
  //
  if (fClustersArray==0){    
    return;
  }
  //
  TParticle *particle; // pointer to a given particle
  AliTPChit *tpcHit; // pointer to a sigle TPC hit
  TClonesArray *particles; //pointer to the particle list
  Int_t sector,nhits;
  Int_t ipart;
  const Int_t kcmaxhits=30000;
  TVector * xxxx = new TVector(kcmaxhits*4);
  TVector & xxx = *xxxx;
  Int_t maxhits = kcmaxhits;
  //construct array for each padrow
  for (Int_t i=0; i<fTPCParam->GetNRow(isec);i++) 
    fClustersArray->CreateRow(isec,i);
  
  //---------------------------------------------------------------
  //  Get the access to the tracks 
  //---------------------------------------------------------------
  
  TTree *tH = gAlice->TreeH();
  Stat_t ntracks = tH->GetEntries();
  particles=gAlice->Particles();
  Int_t npart = particles->GetEntriesFast();
    
  //------------------------------------------------------------
  // Loop over tracks
  //------------------------------------------------------------
  
  for(Int_t track=0;track<ntracks;track++){
    ResetHits();
    tH->GetEvent(track);
    //
    //  Get number of the TPC hits and a pointer
    //  to the particles
    //
    nhits=fHits->GetEntriesFast();
    //
    // Loop over hits
    //
    Int_t currentIndex=0;
    Int_t lastrow=-1;  //last writen row
    for(Int_t hit=0;hit<nhits;hit++){
      tpcHit=(AliTPChit*)fHits->UncheckedAt(hit);
      if (tpcHit==0) continue;
      sector=tpcHit->fSector; // sector number
      if(sector != isec) continue; 
      ipart=tpcHit->fTrack;
      if (ipart<npart) particle=(TParticle*)particles->UncheckedAt(ipart);
      
      //find row number

      Float_t  x[3]={tpcHit->fX,tpcHit->fY,tpcHit->fZ};
      Int_t    index[3]={1,isec,0};
      Int_t    currentrow = fTPCParam->GetPadRow(x,index) ;	
      if (currentrow<0) continue;
      if (lastrow<0) lastrow=currentrow;
      if (currentrow==lastrow){
	if ( currentIndex>=maxhits){
	  maxhits+=kcmaxhits;
	  xxx.ResizeTo(4*maxhits);
	}     
	xxx(currentIndex*4)=x[0];
	xxx(currentIndex*4+1)=x[1];
	xxx(currentIndex*4+2)=x[2];	
	xxx(currentIndex*4+3)=tpcHit->fQ;
	currentIndex++;	
      }
      else 
	if (currentIndex>2){
	  Float_t sumx=0;
	  Float_t sumx2=0;
	  Float_t sumx3=0;
	  Float_t sumx4=0;
	  Float_t sumy=0;
	  Float_t sumxy=0;
	  Float_t sumx2y=0;
	  Float_t sumz=0;
	  Float_t sumxz=0;
	  Float_t sumx2z=0;
	  Float_t sumq=0;
	  for (Int_t index=0;index<currentIndex;index++){
	    Float_t x,x2,x3,x4;
	    x=x2=x3=x4=xxx(index*4);
	    x2*=x;
	    x3*=x2;
	    x4*=x3;
	    sumx+=x;
	    sumx2+=x2;
	    sumx3+=x3;
	    sumx4+=x4;
	    sumy+=xxx(index*4+1);
	    sumxy+=xxx(index*4+1)*x;
	    sumx2y+=xxx(index*4+1)*x2;
	    sumz+=xxx(index*4+2);
	    sumxz+=xxx(index*4+2)*x;
	    sumx2z+=xxx(index*4+2)*x2;	 
	    sumq+=xxx(index*4+3);
	  }
	  Float_t centralPad = (fTPCParam->GetNPads(isec,lastrow)-1)/2;
	  Float_t det=currentIndex*(sumx2*sumx4-sumx3*sumx3)-sumx*(sumx*sumx4-sumx2*sumx3)+
	    sumx2*(sumx*sumx3-sumx2*sumx2);
	  
	  Float_t detay=sumy*(sumx2*sumx4-sumx3*sumx3)-sumx*(sumxy*sumx4-sumx2y*sumx3)+
	    sumx2*(sumxy*sumx3-sumx2y*sumx2);
	  Float_t detaz=sumz*(sumx2*sumx4-sumx3*sumx3)-sumx*(sumxz*sumx4-sumx2z*sumx3)+
	    sumx2*(sumxz*sumx3-sumx2z*sumx2);
	  
	  Float_t detby=currentIndex*(sumxy*sumx4-sumx2y*sumx3)-sumy*(sumx*sumx4-sumx2*sumx3)+
	    sumx2*(sumx*sumx2y-sumx2*sumxy);
	  Float_t detbz=currentIndex*(sumxz*sumx4-sumx2z*sumx3)-sumz*(sumx*sumx4-sumx2*sumx3)+
	    sumx2*(sumx*sumx2z-sumx2*sumxz);
	  
	  Float_t y=detay/det+centralPad;
	  Float_t z=detaz/det;	
	  Float_t by=detby/det; //y angle
	  Float_t bz=detbz/det; //z angle
	  sumy/=Float_t(currentIndex);
	  sumz/=Float_t(currentIndex);
	  AliCluster cl;
	  cl.fX=z;
	  cl.fY=y;
	  cl.fQ=sumq;
	  cl.fSigmaX2=bz;
	  cl.fSigmaY2=by;
	  cl.fTracks[0]=ipart;
	  
	  AliTPCClustersRow * row = (fClustersArray->GetRow(isec,lastrow));
	  if (row!=0) row->InsertCluster(&cl);
	  currentIndex=0;
	  lastrow=currentrow;
	} //end of calculating cluster for given row
	
	
	
    } // end of loop over hits
  }   // end of loop over tracks 
  //write padrows to tree 
  for (Int_t ii=0; ii<fTPCParam->GetNRow(isec);ii++) {
    fClustersArray->StoreRow(isec,ii);    
    fClustersArray->ClearRow(isec,ii);        
  }
  xxxx->Delete();
 
}

//__________________________________________________________________  
void AliTPC::Hits2Digits()  
{ 
 //----------------------------------------------------
 // Loop over all sectors
 //----------------------------------------------------

  if(fTPCParam == 0){
    printf("AliTPCParam MUST be created firstly\n");
    return;
  } 

 for(Int_t isec=0;isec<fTPCParam->GetNSector();isec++) Hits2DigitsSector(isec);

}


//_____________________________________________________________________________
void AliTPC::Hits2DigitsSector(Int_t isec)
{
  //-------------------------------------------------------------------
  // TPC conversion from hits to digits.
  //------------------------------------------------------------------- 

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  //-------------------------------------------------------
  //  Get the access to the track hits
  //-------------------------------------------------------


  TTree *tH = gAlice->TreeH(); // pointer to the hits tree
  Stat_t ntracks = tH->GetEntries();

  if( ntracks > 0){

  //------------------------------------------- 
  //  Only if there are any tracks...
  //-------------------------------------------

    TObjArray **row;
    
      printf("*** Processing sector number %d ***\n",isec);

      Int_t nrows =fTPCParam->GetNRow(isec);

      row= new TObjArray* [nrows];
    
      MakeSector(isec,nrows,tH,ntracks,row);

      //--------------------------------------------------------
      //   Digitize this sector, row by row
      //   row[i] is the pointer to the TObjArray of TVectors,
      //   each one containing electrons accepted on this
      //   row, assigned into tracks
      //--------------------------------------------------------

      Int_t i;

      if (fDigitsArray->GetTree()==0) fDigitsArray->MakeTree();

      for (i=0;i<nrows;i++){

	AliDigits * dig = fDigitsArray->CreateRow(isec,i); 

	DigitizeRow(i,isec,row);

	fDigitsArray->StoreRow(isec,i);

	Int_t ndig = dig->GetDigitSize(); 
 
	printf("*** Sector, row, compressed digits %d %d %d ***\n",isec,i,ndig);
	
        fDigitsArray->ClearRow(isec,i);  

   
       } // end of the sector digitization

      for(i=0;i<nrows;i++){
        row[i]->Delete();     
      }
      
       delete [] row; // delete the array of pointers to TObjArray-s
        
  } // ntracks >0

} // end of Hits2DigitsSector


//_____________________________________________________________________________
void AliTPC::DigitizeRow(Int_t irow,Int_t isec,TObjArray **rows)
{
  //-----------------------------------------------------------
  // Single row digitization, coupling from the neighbouring
  // rows taken into account
  //-----------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  // Modified: Marian Ivanov GSI Darmstadt, m.ivanov@gsi.de
  //-----------------------------------------------------------------
 

  Float_t zerosup = fTPCParam->GetZeroSup();
  Int_t nrows =fTPCParam->GetNRow(isec);
  fCurrentIndex[1]= isec;
  

  Int_t nofPads = fTPCParam->GetNPads(isec,irow);
  Int_t nofTbins = fTPCParam->GetMaxTBin();
  Int_t indexRange[4];
  //
  //  Integrated signal for this row
  //  and a single track signal
  //    
  TMatrix *m1   = new TMatrix(0,nofPads,0,nofTbins); // integrated
  TMatrix *m2   = new TMatrix(0,nofPads,0,nofTbins); // single
  //
  TMatrix &total  = *m1;

  //  Array of pointers to the label-signal list

  Int_t nofDigits = nofPads*nofTbins; // number of digits for this row
  Float_t  **pList = new Float_t* [nofDigits]; 

  Int_t lp;
  Int_t i1;   
  for(lp=0;lp<nofDigits;lp++)pList[lp]=0; // set all pointers to NULL
  //
  //calculate signal 
  //
  Int_t row1 = TMath::Max(irow-fTPCParam->GetNCrossRows(),0);
  Int_t row2 = TMath::Min(irow+fTPCParam->GetNCrossRows(),nrows-1);
  for (Int_t row= row1;row<=row2;row++){
    Int_t nTracks= rows[row]->GetEntries();
    for (i1=0;i1<nTracks;i1++){
      fCurrentIndex[2]= row;
      fCurrentIndex[3]=irow;
      if (row==irow){
	m2->Zero();  // clear single track signal matrix
	Float_t trackLabel = GetSignal(rows[row],i1,m2,m1,indexRange); 
	GetList(trackLabel,nofPads,m2,indexRange,pList);
      }
      else   GetSignal(rows[row],i1,0,m1,indexRange);
    }
  }
         
  Int_t tracks[3];

  AliDigits *dig = fDigitsArray->GetRow(isec,irow);
  for(Int_t ip=0;ip<nofPads;ip++){
    for(Int_t it=0;it<nofTbins;it++){

      Float_t q = total(ip,it);

      Int_t gi =it*nofPads+ip; // global index

      q = gRandom->Gaus(q,fTPCParam->GetNoise()*fTPCParam->GetNoiseNormFac()); 

      q = (Int_t)q;

      if(q <=zerosup) continue; // do not fill zeros
      if(q > fTPCParam->GetADCSat()) q = fTPCParam->GetADCSat();  // saturation

      //
      //  "real" signal or electronic noise (list = -1)?
      //    

      for(Int_t j1=0;j1<3;j1++){
        tracks[j1] = (pList[gi]) ?(Int_t)(*(pList[gi]+j1)) : -1;
      }

//Begin_Html
/*
  <A NAME="AliDigits"></A>
  using of AliDigits object
*/
//End_Html
      dig->SetDigitFast((Short_t)q,it,ip);
      if (fDigitsArray->IsSimulated())
	{
	 ((AliSimDigits*)dig)->SetTrackIDFast(tracks[0],it,ip,0);
	 ((AliSimDigits*)dig)->SetTrackIDFast(tracks[1],it,ip,1);
	 ((AliSimDigits*)dig)->SetTrackIDFast(tracks[2],it,ip,2);
	}
     
    
    } // end of loop over time buckets
  }  // end of lop over pads 

  //
  //  This row has been digitized, delete nonused stuff
  //

  for(lp=0;lp<nofDigits;lp++){
    if(pList[lp]) delete [] pList[lp];
  }
  
  delete [] pList;

  delete m1;
  delete m2;
  //  delete m3;

} // end of DigitizeRow

//_____________________________________________________________________________

Float_t AliTPC::GetSignal(TObjArray *p1, Int_t ntr, TMatrix *m1, TMatrix *m2,
                          Int_t *indexRange)
{

  //---------------------------------------------------------------
  //  Calculates 2-D signal (pad,time) for a single track,
  //  returns a pointer to the signal matrix and the track label 
  //  No digitization is performed at this level!!!
  //---------------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  // Modified: Marian Ivanov 
  //-----------------------------------------------------------------

  TVector *tv;
 
  tv = (TVector*)p1->At(ntr); // pointer to a track
  TVector &v = *tv;
  
  Float_t label = v(0);
  Int_t centralPad = (fTPCParam->GetNPads(fCurrentIndex[1],fCurrentIndex[3])-1)/2;

  Int_t nElectrons = (tv->GetNrows()-1)/4;
  indexRange[0]=9999; // min pad
  indexRange[1]=-1; // max pad
  indexRange[2]=9999; //min time
  indexRange[3]=-1; // max time

  //  Float_t IneffFactor = 0.5; // inefficiency in the gain close to the edge, as above

  TMatrix &signal = *m1;
  TMatrix &total = *m2;
  //
  //  Loop over all electrons
  //
  for(Int_t nel=0; nel<nElectrons; nel++){
    Int_t idx=nel*4;
    Float_t aval =  v(idx+4);
    Float_t eltoadcfac=aval*fTPCParam->GetTotalNormFac(); 
    Float_t xyz[3]={v(idx+1),v(idx+2),v(idx+3)};
    Int_t n = fTPCParam->CalcResponse(xyz,fCurrentIndex,fCurrentIndex[3]);
    
    if (n>0) for (Int_t i =0; i<n; i++){
       Int_t *index = fTPCParam->GetResBin(i);        
       Int_t pad=index[1]+centralPad;  //in digit coordinates central pad has coordinate 0
       if ( ( pad<(fTPCParam->GetNPads(fCurrentIndex[1],fCurrentIndex[3]))) && (pad>0)) {
	 Int_t time=index[2];	 
	 Float_t weight = fTPCParam->GetResWeight(i); //we normalise response to ADC channel
	 weight *= eltoadcfac;
	 
	 if (m1!=0) signal(pad,time)+=weight; 
	 total(pad,time)+=weight;
	 indexRange[0]=TMath::Min(indexRange[0],pad);
	 indexRange[1]=TMath::Max(indexRange[1],pad);
	 indexRange[2]=TMath::Min(indexRange[2],time);
	 indexRange[3]=TMath::Max(indexRange[3],time); 
       }	 
    }
  } // end of loop over electrons
  
  return label; // returns track label when finished
}

//_____________________________________________________________________________
void AliTPC::GetList(Float_t label,Int_t np,TMatrix *m,Int_t *indexRange,
                     Float_t **pList)
{
  //----------------------------------------------------------------------
  //  Updates the list of tracks contributing to digits for a given row
  //----------------------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  TMatrix &signal = *m;

  // lop over nonzero digits

  for(Int_t it=indexRange[2];it<indexRange[3]+1;it++){
    for(Int_t ip=indexRange[0];ip<indexRange[1]+1;ip++){


        // accept only the contribution larger than 500 electrons (1/2 s_noise)

        if(signal(ip,it)<0.5) continue; 


        Int_t globalIndex = it*np+ip; // globalIndex starts from 0!
        
        if(!pList[globalIndex]){
        
          // 
	  // Create new list (6 elements - 3 signals and 3 labels),
	  //

          pList[globalIndex] = new Float_t [6];

	  // set list to -1 

          *pList[globalIndex] = -1.;
          *(pList[globalIndex]+1) = -1.;
          *(pList[globalIndex]+2) = -1.;
          *(pList[globalIndex]+3) = -1.;
          *(pList[globalIndex]+4) = -1.;
          *(pList[globalIndex]+5) = -1.;


          *pList[globalIndex] = label;
          *(pList[globalIndex]+3) = signal(ip,it);
        }
        else{

	  // check the signal magnitude

          Float_t highest = *(pList[globalIndex]+3);
          Float_t middle = *(pList[globalIndex]+4);
          Float_t lowest = *(pList[globalIndex]+5);

	  //
	  //  compare the new signal with already existing list
	  //

          if(signal(ip,it)<lowest) continue; // neglect this track

	  //

          if (signal(ip,it)>highest){
            *(pList[globalIndex]+5) = middle;
            *(pList[globalIndex]+4) = highest;
            *(pList[globalIndex]+3) = signal(ip,it);

            *(pList[globalIndex]+2) = *(pList[globalIndex]+1);
            *(pList[globalIndex]+1) = *pList[globalIndex];
            *pList[globalIndex] = label;
	  }
          else if (signal(ip,it)>middle){
            *(pList[globalIndex]+5) = middle;
            *(pList[globalIndex]+4) = signal(ip,it);

            *(pList[globalIndex]+2) = *(pList[globalIndex]+1);
            *(pList[globalIndex]+1) = label;
	  }
          else{
            *(pList[globalIndex]+5) = signal(ip,it);
            *(pList[globalIndex]+2) = label;
	  }
        }

    } // end of loop over pads
  } // end of loop over time bins



}//end of GetList
//___________________________________________________________________
void AliTPC::MakeSector(Int_t isec,Int_t nrows,TTree *TH,
                        Stat_t ntracks,TObjArray **row)
{

  //-----------------------------------------------------------------
  // Prepares the sector digitization, creates the vectors of
  // tracks for each row of this sector. The track vector
  // contains the track label and the position of electrons.
  //-----------------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  Float_t gasgain = fTPCParam->GetGasGain();
  Int_t i;
  Float_t xyz[4]; 

  AliTPChit *tpcHit; // pointer to a sigle TPC hit    
 
  //----------------------------------------------
  // Create TObjArray-s, one for each row,
  // each TObjArray will store the TVectors
  // of electrons, one TVector per each track.
  //---------------------------------------------- 
    
  for(i=0; i<nrows; i++){
    row[i] = new TObjArray;
  }
  Int_t *nofElectrons = new Int_t [nrows]; // electron counter for each row
  TVector **tracks = new TVector* [nrows]; //pointers to the track vectors

  //--------------------------------------------------------------------
  //  Loop over tracks, the "track" contains the full history
  //--------------------------------------------------------------------

  Int_t previousTrack,currentTrack;
  previousTrack = -1; // nothing to store so far!

  for(Int_t track=0;track<ntracks;track++){

    ResetHits();

    TH->GetEvent(track); // get next track
    Int_t nhits = fHits->GetEntriesFast(); // get number of hits for this track

    if(nhits == 0) continue; // no hits in the TPC for this track

    //--------------------------------------------------------------
    //  Loop over hits
    //--------------------------------------------------------------

    for(Int_t hit=0;hit<nhits;hit++){

      tpcHit = (AliTPChit*)fHits->UncheckedAt(hit); // get a pointer to a hit
      
      Int_t sector=tpcHit->fSector; // sector number
      if(sector != isec) continue; 

	currentTrack = tpcHit->fTrack; // track number
        if(currentTrack != previousTrack){
                          
           // store already filled fTrack
              
	   for(i=0;i<nrows;i++){
             if(previousTrack != -1){
	       if(nofElectrons[i]>0){
	         TVector &v = *tracks[i];
		 v(0) = previousTrack;
                 tracks[i]->ResizeTo(4*nofElectrons[i]+1); // shrink if necessary
	         row[i]->Add(tracks[i]);                     
	       }
               else{
                 delete tracks[i]; // delete empty TVector
                 tracks[i]=0;
	       }
	     }

             nofElectrons[i]=0;
             tracks[i] = new TVector(481); // TVectors for the next fTrack

	   } // end of loop over rows
	       
           previousTrack=currentTrack; // update track label 
	}
	   
	Int_t qI = (Int_t) (tpcHit->fQ); // energy loss (number of electrons)

       //---------------------------------------------------
       //  Calculate the electron attachment probability
       //---------------------------------------------------


        Float_t time = 1.e6*(fTPCParam->GetZLength()-TMath::Abs(tpcHit->fZ))
                                                        /fTPCParam->GetDriftV(); 
	// in microseconds!	
	Float_t attProb = fTPCParam->GetAttCoef()*
	  fTPCParam->GetOxyCont()*time; //  fraction! 
   
	//-----------------------------------------------
	//  Loop over electrons
	//-----------------------------------------------
	Int_t index[3];
	index[1]=isec;
        for(Int_t nel=0;nel<qI;nel++){
          // skip if electron lost due to the attachment
          if((gRandom->Rndm(0)) < attProb) continue; // electron lost!
	  xyz[0]=tpcHit->fX;
	  xyz[1]=tpcHit->fY;
	  xyz[2]=tpcHit->fZ;	  
	  xyz[3]= (Float_t) (-gasgain*TMath::Log(gRandom->Rndm()));
	  index[0]=1;
	  
	  TransportElectron(xyz,index); //MI change -august	  
	  Int_t rowNumber;
	  fTPCParam->GetPadRow(xyz,index); //MI change august
	  rowNumber = index[2];
	  //transform position to local digit coordinates
	  //relative to nearest pad row 
	  if ((rowNumber<0)||rowNumber>=fTPCParam->GetNRow(isec)) continue;	  
	  nofElectrons[rowNumber]++;	  
	  //----------------------------------
	  // Expand vector if necessary
	  //----------------------------------
	  if(nofElectrons[rowNumber]>120){
	    Int_t range = tracks[rowNumber]->GetNrows();
	    if((nofElectrons[rowNumber])>(range-1)/4){
        
	      tracks[rowNumber]->ResizeTo(range+400); // Add 100 electrons
	    }
	  }
	  
	  TVector &v = *tracks[rowNumber];
	  Int_t idx = 4*nofElectrons[rowNumber]-3;

	  v(idx)=  xyz[0];   // X - pad row coordinate
	  v(idx+1)=xyz[1];   // Y - pad coordinate (along the pad-row)
          v(idx+2)=xyz[2];   // Z - time bin coordinate
	  v(idx+3)=xyz[3];   // avalanche size  
	} // end of loop over electrons
        
      } // end of loop over hits
    } // end of loop over tracks

    //
    //   store remaining track (the last one) if not empty
    //

     for(i=0;i<nrows;i++){
       if(nofElectrons[i]>0){
          TVector &v = *tracks[i];
	  v(0) = previousTrack;
          tracks[i]->ResizeTo(4*nofElectrons[i]+1); // shrink if necessary
	  row[i]->Add(tracks[i]);  
	}
	else{
          delete tracks[i];
          tracks[i]=0;
	}  
      }  

          delete [] tracks;
          delete [] nofElectrons;
 

} // end of MakeSector


//_____________________________________________________________________________
void AliTPC::Init()
{
  //
  // Initialise TPC detector after definition of geometry
  //
  Int_t i;
  //
  printf("\n");
  for(i=0;i<35;i++) printf("*");
  printf(" TPC_INIT ");
  for(i=0;i<35;i++) printf("*");
  printf("\n");
  //
  for(i=0;i<80;i++) printf("*");
  printf("\n");
}

//_____________________________________________________________________________
void AliTPC::MakeBranch(Option_t* option)
{
  //
  // Create Tree branches for the TPC.
  //
  Int_t buffersize = 4000;
  char branchname[10];
  sprintf(branchname,"%s",GetName());

  AliDetector::MakeBranch(option);

  char *d = strstr(option,"D");

  if (fDigits   && gAlice->TreeD() && d) {
    gAlice->TreeD()->Branch(branchname,&fDigits, buffersize);
    printf("Making Branch %s for digits\n",branchname);
  }	
}
 
//_____________________________________________________________________________
void AliTPC::ResetDigits()
{
  //
  // Reset number of digits and the digits array for this detector
  //
  fNdigits   = 0;
  if (fDigits)   fDigits->Clear();
}

//_____________________________________________________________________________
void AliTPC::SetSecAL(Int_t sec)
{
  //---------------------------------------------------
  // Activate/deactivate selection for lower sectors
  //---------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  fSecAL = sec;
}

//_____________________________________________________________________________
void AliTPC::SetSecAU(Int_t sec)
{
  //----------------------------------------------------
  // Activate/deactivate selection for upper sectors
  //---------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  fSecAU = sec;
}

//_____________________________________________________________________________
void AliTPC::SetSecLows(Int_t s1,Int_t s2,Int_t s3,Int_t s4,Int_t s5, Int_t s6)
{
  //----------------------------------------
  // Select active lower sectors
  //----------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  fSecLows[0] = s1;
  fSecLows[1] = s2;
  fSecLows[2] = s3;
  fSecLows[3] = s4;
  fSecLows[4] = s5;
  fSecLows[5] = s6;
}

//_____________________________________________________________________________
void AliTPC::SetSecUps(Int_t s1,Int_t s2,Int_t s3,Int_t s4,Int_t s5, Int_t s6,
                       Int_t s7, Int_t s8 ,Int_t s9 ,Int_t s10, 
                       Int_t s11 , Int_t s12)
{
  //--------------------------------
  // Select active upper sectors
  //--------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  fSecUps[0] = s1;
  fSecUps[1] = s2;
  fSecUps[2] = s3;
  fSecUps[3] = s4;
  fSecUps[4] = s5;
  fSecUps[5] = s6;
  fSecUps[6] = s7;
  fSecUps[7] = s8;
  fSecUps[8] = s9;
  fSecUps[9] = s10;
  fSecUps[10] = s11;
  fSecUps[11] = s12;
}

//_____________________________________________________________________________
void AliTPC::SetSens(Int_t sens)
{

  //-------------------------------------------------------------
  // Activates/deactivates the sensitive strips at the center of
  // the pad row -- this is for the space-point resolution calculations
  //-------------------------------------------------------------

  //-----------------------------------------------------------------
  // Origin: Marek Kowalski  IFJ, Krakow, Marek.Kowalski@ifj.edu.pl
  //-----------------------------------------------------------------

  fSens = sens;
}
 
void AliTPC::SetSide(Float_t side=0.)
{
  // choice of the TPC side

  fSide = side;
 
}
//____________________________________________________________________________
void AliTPC::SetGasMixt(Int_t nc,Int_t c1,Int_t c2,Int_t c3,Float_t p1,
                           Float_t p2,Float_t p3)
{

  // gax mixture definition

 fNoComp = nc;
 
 fMixtComp[0]=c1;
 fMixtComp[1]=c2;
 fMixtComp[2]=c3;

 fMixtProp[0]=p1;
 fMixtProp[1]=p2;
 fMixtProp[2]=p3; 
 
 
}
//_____________________________________________________________________________

void AliTPC::TransportElectron(Float_t *xyz, Int_t *index)
{
  //
  // electron transport taking into account:
  // 1. diffusion, 
  // 2.ExB at the wires
  // 3. nonisochronity
  //
  // xyz and index must be already transformed to system 1
  //

  fTPCParam->Transform1to2(xyz,index);
  
  //add diffusion
  Float_t driftl=xyz[2];
  if(driftl<0.01) driftl=0.01;
  driftl=TMath::Sqrt(driftl);
  Float_t sigT = driftl*(fTPCParam->GetDiffT());
  Float_t sigL = driftl*(fTPCParam->GetDiffL());
  xyz[0]=gRandom->Gaus(xyz[0],sigT);
  xyz[1]=gRandom->Gaus(xyz[1],sigT);
  xyz[2]=gRandom->Gaus(xyz[2],sigL);

  // ExB
  
  if (fTPCParam->GetMWPCReadout()==kTRUE){
    Float_t x1=xyz[0];
    fTPCParam->Transform2to2NearestWire(xyz,index);
    Float_t dx=xyz[0]-x1;
    xyz[1]+=dx*(fTPCParam->GetOmegaTau());
  }
  //add nonisochronity (not implemented yet)
  
}
//_____________________________________________________________________________
void AliTPC::Streamer(TBuffer &R__b)
{
  //
  // Stream an object of class AliTPC.
  //
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(); if (R__v) { }
      AliDetector::Streamer(R__b);
      if (R__v < 2) return;
      R__b >> fNsectors;
   } else {
      R__b.WriteVersion(AliTPC::IsA());
      AliDetector::Streamer(R__b);
      R__b << fNsectors;
   }
}
  
ClassImp(AliTPCdigit)
 
//_____________________________________________________________________________
AliTPCdigit::AliTPCdigit(Int_t *tracks, Int_t *digits):
  AliDigit(tracks)
{
  //
  // Creates a TPC digit object
  //
  fSector     = digits[0];
  fPadRow     = digits[1];
  fPad        = digits[2];
  fTime       = digits[3];
  fSignal     = digits[4];
}

 
ClassImp(AliTPChit)
 
//_____________________________________________________________________________
AliTPChit::AliTPChit(Int_t shunt, Int_t track, Int_t *vol, Float_t *hits):
AliHit(shunt,track)
{
  //
  // Creates a TPC hit object
  //
  fSector     = vol[0];
  fPadRow     = vol[1];
  fX          = hits[0];
  fY          = hits[1];
  fZ          = hits[2];
  fQ          = hits[3];
}
 

