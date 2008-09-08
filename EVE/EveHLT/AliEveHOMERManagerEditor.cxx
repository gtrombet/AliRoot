// $Id$
// Main authors: Matevz Tadel & Alja Mrak-Tadel: 2006, 2007

/**************************************************************************
 * Copyright(c) 1998-2008, ALICE Experiment at CERN, all rights reserved. *
 * See http://aliceinfo.cern.ch/Offline/AliRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

#include "AliEveHOMERManagerEditor.h"
#include "AliEveHOMERManager.h"

#include <TVirtualPad.h>
#include <TColor.h>
#include <TROOT.h>

#include <TGLabel.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TGColorSelect.h>
#include <TGDoubleSlider.h>

//______________________________________________________________________________
// AliEveHOMERManagerEditor
//

ClassImp(AliEveHOMERManagerEditor)

AliEveHOMERManagerEditor::AliEveHOMERManagerEditor(const TGWindow *p, Int_t width, Int_t height,
	     UInt_t options, Pixel_t back) :
  TGedFrame(p, width, height, options | kVerticalFrame, back),
  fM(0),
  fButtonConnect(0),
  fButtonNextEvent(0),
  fButtonEventLoop(0),
  fEventLoopStarted(kFALSE) {

  MakeTitle("AliEveHOMERManager");

  // Create widgets
  // fXYZZ = new TGSomeWidget(this, ...);
  // AddFrame(fXYZZ, new TGLayoutHints(...));
  // fXYZZ->Connect("SignalName()", "AliEveHOMERManagerEditor", this, "DoXYZZ()");

  fButtonConnect = new TGTextButton(this, "  Connect to HLT  ");
  AddFrame(fButtonConnect); //, new TGLayoutHints(...));
  fButtonConnect->Connect("Clicked()", "AliEveHOMERManagerEditor", this, "ConnectToHLT()");

  fButtonNextEvent = new TGTextButton(this, "  NextEvent  ");
  AddFrame(fButtonNextEvent); //, new TGLayoutHints(...));
  fButtonNextEvent->Connect("Clicked()", "AliEveHOMERManagerEditor", this, "NextEvent()");

  fButtonEventLoop = new TGTextButton(this, "  not yet used  ");
  AddFrame(fButtonEventLoop); //, new TGLayoutHints(...));
  fButtonEventLoop->Connect("Clicked()", "AliEveHOMERManagerEditor", this, "EventLoop()");

}

/******************************************************************************/

void AliEveHOMERManagerEditor::SetModel(TObject* obj)
{
  fM = dynamic_cast<AliEveHOMERManager*>(obj);

  // Set values of widgets
  // fXYZZ->SetValue(fM->GetXYZZ());
}

/******************************************************************************/

// Implements callback/slot methods

// void AliEveHOMERManagerEditor::DoXYZZ()
// {
//   fM->SetXYZZ(fXYZZ->GetValue());
//   Update();
// }

void AliEveHOMERManagerEditor::ConnectToHLT()
{
  // Connects to HOMER sources -> to HLT.

  fM->ConnectHOMER();
}

void AliEveHOMERManagerEditor::NextEvent()
{
  // call next event from macro
  gROOT->ProcessLineFast("nextEvent();");

}

void AliEveHOMERManagerEditor::EventLoop()
{
  // Start/stop event loop

  fM->ConnectHOMER();
}
