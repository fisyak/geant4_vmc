// $Id: TG4StepManager.cxx,v 1.7 2004/05/19 19:44:12 brun Exp $
// Category: digits+hits
//
// Author: I.Hrivnacova
//
// Class TG4StepManager
// --------------------
// See the class description in the header file.

#include "TG4StepManager.h"
#include "TG4SteppingAction.h"
#include "TG4GeometryServices.h"
#include "TG4SDServices.h"
#include "TG4ParticlesManager.h"
#include "TG4PhysicsManager.h"
#include "TG4Globals.h"
#include "TG4G3Units.h"

#include <G4SteppingManager.hh>
#include <G4UserLimits.hh>
#include <G4UImanager.hh>
#include <G4AffineTransform.hh>
#include <G4TransportationManager.hh>
#include <G4Navigator.hh>
#include <G4VProcess.hh>
#include <G4ProcessManager.hh>
#include <G4ProcessVector.hh>
#include <G4VTouchable.hh>

#include <TLorentzVector.h>

TG4StepManager* TG4StepManager::fgInstance = 0;

//_____________________________________________________________________________
TG4StepManager::TG4StepManager() 
  : fTrack(0),
    fStep(0),
    fStepStatus(kNormalStep),
    fTouchableHistory(0),
    fSteppingManager(0)
{
// 
  if (fgInstance) {
    TG4Globals::Exception(
      "TG4StepManager: attempt to create two instances of singleton.");
  }
      
  fgInstance = this;  
  
  fTouchableHistory = new G4TouchableHistory();
}

//_____________________________________________________________________________
TG4StepManager::TG4StepManager(const TG4StepManager& right) {
// 
  TG4Globals::Exception(
    "Attempt to copy TG4StepManager singleton.");
}

//_____________________________________________________________________________
TG4StepManager::~TG4StepManager() {
//
  delete fTouchableHistory;
}

// operators

//_____________________________________________________________________________
TG4StepManager& TG4StepManager::operator=(const TG4StepManager& right)
{
  // check assignement to self
  if (this == &right) return *this;

  TG4Globals::Exception(
    "Attempt to assign TG4StepManager singleton.");
    
  return *this;  
}    
          
//
// private methods
//

//_____________________________________________________________________________
void TG4StepManager::CheckTrack() const
{
// Gives exception in case the track is not defined.
// ---

  if (!fTrack) 
    TG4Globals::Exception("TG4StepManager: Track is not defined.");
}     


//_____________________________________________________________________________
void TG4StepManager::CheckStep(const G4String& method) const
{
// Gives exception in case the step is not defined.
// ---

  if (!fStep) {
    G4String text = "TG4StepManager::";
    text = text + method + ": Step is not defined.";
    TG4Globals::Exception(text);
  }
}     


//_____________________________________________________________________________
void TG4StepManager::CheckSteppingManager() const
{
// Gives exception in case the step is not defined.
// ---

  if (!fSteppingManager) 
    TG4Globals::Exception("TG4StepManager: Stepping manager is not defined.");
}     


//_____________________________________________________________________________
void TG4StepManager::SetTLorentzVector(G4ThreeVector xyz, G4double t, 
                                       TLorentzVector& lv) const				       
{
// Fills TLorentzVector with G4ThreeVector and G4double.
// ---

   lv[0] = xyz.x();  				       
   lv[1] = xyz.y();  				       
   lv[2] = xyz.z();  				       
   lv[3] = t;
}     				       

//_____________________________________________________________________________
G4VPhysicalVolume* TG4StepManager::GetCurrentOffPhysicalVolume(G4int off) const 
{
// Returns the physical volume of the off-th mother's
// of the current volume.
// ---
 
  // Get current touchable
  //
  const G4VTouchable* touchable; 
  if (fStepStatus == kNormalStep) {

#ifdef MCDEBUG
    CheckStep("GetCurrentOffPhysicalVolume");
#endif    

    touchable = fStep->GetPreStepPoint()->GetTouchable();
  }  
  else if (fStepStatus == kBoundary) {

#ifdef MCDEBUG
    CheckStep("GetCurrentOffPhysicalVolume");
#endif 

    touchable = fStep->GetPostStepPoint()->GetTouchable();
  }  
  else {

#ifdef MCDEBUG
    CheckTrack();
#endif 
    const G4ThreeVector& position = fTrack->GetPosition();
    //const G4ThreeVector& direction = fTrack->GetMomentumDirection();
    G4Navigator* navigator =
      G4TransportationManager::GetTransportationManager()->
        GetNavigatorForTracking();
    
    navigator->LocateGlobalPointAndUpdateTouchable(
                     position, fTouchableHistory);

    touchable = fTouchableHistory;
  }   
    
  // Check touchable depth
  //
  if (touchable->GetHistoryDepth() < off) {
    G4String text = "TG4StepManager::GetCurrentOffPhysicalVolume: \n";
    text = text + "    Volume ";
    text = text + touchable->GetVolume()->GetName();
    text = text + " has not defined mother in the required level.";
    TG4Globals::Warning(text);    
    return 0;
  }  

  if (off == 0)
    return touchable->GetVolume();
  
  // Get the off-th mother
  // 
  G4int index = touchable->GetHistoryDepth() - off;
        // in the touchable history volumes are ordered
	// from top volume up to mother volume;
	// the touchable volume is not in the history
  
  return touchable->GetHistory()->GetVolume(index); 
}     

//
// public methods
//

//_____________________________________________________________________________
void TG4StepManager::StopTrack()
{
// Stops the current track and skips to the next.
// ?? do we want to invoke rest processes?
// ?? do we want to stop secondaries too?
//   possible "stop" track status from G4:
//   fStopButAlive,      // Invoke active rest physics processes and
//                       // and kill the current track afterward
//   fStopAndKill,       // Kill the current track
//   fKillTrackAndSecondaries,
//                       // Kill the current track and also associated
//                       // secondaries.
// ---

  if (fTrack) {
    fTrack->SetTrackStatus(fStopAndKill);
    // fTrack->SetTrackStatus(fStopButAlive);
    // fTrack->SetTrackStatus(fKillTrackAndSecondaries);
  }
  else {
    G4String text = "TG4StepManager::StopTrack(): \n";
    text = text + "    There is no current track to be stopped.";
    TG4Globals::Warning(text);
  }    
}

//_____________________________________________________________________________
void TG4StepManager::StopEvent()
{
// Aborts the current event processing.
// ---

  if (fTrack) {
    fTrack->SetTrackStatus(fKillTrackAndSecondaries);
            //StopTrack();   // cannot be used as it keeps secondaries
  }
  	    
  G4UImanager::GetUIpointer()->ApplyCommand("/event/abort");
}

//_____________________________________________________________________________
void TG4StepManager::StopRun()
{
// Aborts the current event processing.
// ---

  TG4SDServices::Instance()->SetIsStopRun(true);

  StopEvent();
  G4UImanager::GetUIpointer()->ApplyCommand("/run/abort");
}

//_____________________________________________________________________________
void TG4StepManager::SetMaxStep(Double_t step)
{
// Maximum step allowed in the current logical volume.
// ---

  G4UserLimits* userLimits 
    = GetCurrentPhysicalVolume()->GetLogicalVolume()->GetUserLimits();
  
#ifdef MCDEBUG
  if (!userLimits) {
    G4String text = "TG4StepManager::SetMaxStep:\n";
    text = text + "   User limits not defined.";
    TG4Globals::Exception(text);
    return;
  }
#endif  

  // set max step
  userLimits->SetMaxAllowedStep(step*TG4G3Units::Length()); 

}

//_____________________________________________________________________________
void TG4StepManager::SetMaxNStep(Int_t maxNofSteps)
{
// Sets maximum number of steps.
// ---

  TG4SteppingAction::Instance()->SetMaxNofSteps(maxNofSteps);
}

//_____________________________________________________________________________
void TG4StepManager::SetUserDecay(Int_t pdg)
{
// Not yet implemented.
// ---

  TG4Globals::Exception(
    "TG4StepManager::SetUserDecay(..) is not yet implemented.");
}

//_____________________________________________________________________________
G4VPhysicalVolume* TG4StepManager::GetCurrentPhysicalVolume() const 
{
// Returns the current physical volume.
// According to fStepStatus the volume from track vertex,
// pre step point or post step point is returned.
// ---

  G4VPhysicalVolume* physVolume; 
  if (fStepStatus == kNormalStep) {

#ifdef MCDEBUG
    CheckStep("GetCurrentPhysicalVolume");
#endif    

    physVolume = fStep->GetPreStepPoint()->GetPhysicalVolume();
  }  
  else if (fStepStatus == kBoundary) {

#ifdef MCDEBUG
    CheckStep("GetCurrentPhysicalVolume");
#endif 

    physVolume = fStep->GetPostStepPoint()->GetPhysicalVolume();
  }  
  else {

#ifdef MCDEBUG
    CheckTrack();
#endif 

    G4ThreeVector position = fTrack->GetPosition();
    G4Navigator* navigator =
      G4TransportationManager::GetTransportationManager()->
        GetNavigatorForTracking();
    physVolume
     = navigator->LocateGlobalPointAndSetup(position);  
  }   
    
  return physVolume;
}     

//_____________________________________________________________________________
Int_t TG4StepManager::CurrentVolID(Int_t& copyNo) const
{
// Returns the current sensitive detector ID
// and the copy number of the current physical volume.
// ---

  G4VPhysicalVolume* physVolume = GetCurrentPhysicalVolume(); 
  copyNo = physVolume->GetCopyNo() + 1;

  // sensitive detector ID
  TG4SDServices* sdServices = TG4SDServices::Instance();
  return sdServices->GetVolumeID(physVolume->GetLogicalVolume());
} 

//_____________________________________________________________________________
Int_t TG4StepManager::CurrentVolOffID(Int_t off, Int_t&  copyNo) const
{ 
// Returns the off-th mother's of the current volume
// the sensitive detector ID and the copy number.
// ---

  if (off == 0) return CurrentVolID(copyNo);

  G4VPhysicalVolume* mother = GetCurrentOffPhysicalVolume(off); 

  if (mother) {
    copyNo = mother->GetCopyNo() + 1;

    // sensitive detector ID
    TG4SDServices* sdServices = TG4SDServices::Instance();
    return sdServices->GetVolumeID(mother->GetLogicalVolume());
  }
  else {
    copyNo = 0;
    return 0;
  }  
}

//_____________________________________________________________________________
const char* TG4StepManager::CurrentVolName() const
{
// Returns the current physical volume name.
// ---

  return GetCurrentPhysicalVolume()->GetName();
}

//_____________________________________________________________________________
const char* TG4StepManager::CurrentVolOffName(Int_t off) const
{ 
// Returns the off-th mother's physical volume name.
// ---

  if (off == 0) return CurrentVolName();

  G4VPhysicalVolume* mother = GetCurrentOffPhysicalVolume(off); 

  if (mother) 
    return mother->GetName();
  else 
    return 0;
}

//_____________________________________________________________________________
Int_t TG4StepManager::CurrentMaterial(Float_t &a, Float_t &z, Float_t &dens, 
                          Float_t &radl, Float_t &absl) const
{
// Returns the parameters of the current material during transport
// the return value is the number of elements in the mixture.
// ---

  G4VPhysicalVolume* physVolume = GetCurrentPhysicalVolume(); 
    
  G4Material* material 
    = physVolume->GetLogicalVolume()->GetMaterial();

#ifdef MCDEBUG
  if (!material) {
    TG4Globals::Exception(
     "TG4StepManager::CurrentMaterial(..): material is not defined.");
    return 0;
  }
#endif  

  G4int nofElements = material->GetNumberOfElements();
  TG4GeometryServices* geometryServices = TG4GeometryServices::Instance();
  a = geometryServices->GetEffA(material);
  z = geometryServices->GetEffZ(material);
      
  // density 
  dens = material->GetDensity();
  dens /= TG4G3Units::MassDensity();      
      
  // radiation length
  radl = material->GetRadlen();
  radl /= TG4G3Units::Length();
      
  absl = 0.;  // this parameter is not defined in Geant4
  return nofElements;
}

//_____________________________________________________________________________
void TG4StepManager::Gmtod(Float_t* xm, Float_t* xd, Int_t iflag) 
{ 
// Transforms a position from the world reference frame
// to the current volume reference frame.
//
//  Geant3 desription:
//  ==================
//       Computes coordinates XD (in DRS) 
//       from known coordinates XM in MRS 
//       The local reference system can be initialized by
//         - the tracking routines and GMTOD used in GUSTEP
//         - a call to GMEDIA(XM,NUMED)
//         - a call to GLVOLU(NLEVEL,NAMES,NUMBER,IER) 
//             (inverse routine is GDTOM) 
//
//        If IFLAG=1  convert coordinates 
//           IFLAG=2  convert direction cosinus
//
// ---

  G4double* dxm = TG4GeometryServices::Instance()->CreateG4doubleArray(xm, 3);
  G4double* dxd = TG4GeometryServices::Instance()->CreateG4doubleArray(xd, 3);

  Gmtod(dxm, dxd, iflag);

  for (G4int i=0; i<3; i++) {
    xm[i] = dxm[i]; 
    xd[i] = dxd[i];
  }   

  delete [] dxm;
  delete [] dxd;
} 
 
//_____________________________________________________________________________
void TG4StepManager::Gmtod(Double_t* xm, Double_t* xd, Int_t iflag) 
{ 
// Transforms a position from the world reference frame
// to the current volume reference frame.
//
//  Geant3 desription:
//  ==================
//       Computes coordinates XD (in DRS) 
//       from known coordinates XM in MRS 
//       The local reference system can be initialized by
//         - the tracking routines and GMTOD used in GUSTEP
//         - a call to GMEDIA(XM,NUMED)
//         - a call to GLVOLU(NLEVEL,NAMES,NUMBER,IER) 
//             (inverse routine is GDTOM) 
//
//        If IFLAG=1  convert coordinates 
//           IFLAG=2  convert direction cosinus
//
// ---

#ifdef MCDEBUG
  if (iflag != 1 && iflag != 2) {
      TG4Globals::Exception(
        "TG4StepManager::Gmtod(..,iflag): iflag is not in 1..2");
      return;	
  }	
#endif

  G4AffineTransform affineTransform;

  if (fStepStatus == kVertex) {
    G4Navigator* navigator =
      G4TransportationManager::GetTransportationManager()->
        GetNavigatorForTracking();
	
    affineTransform = navigator->GetGlobalToLocalTransform();
  }
  else if (fStepStatus == kBoundary) {

#ifdef MCDEBUG
    CheckStep("Gmtod");
#endif
 
    affineTransform
      = fStep->GetPostStepPoint()->GetTouchable()->GetHistory()
        ->GetTopTransform();
  }	
  else {

#ifdef MCDEBUG
    CheckStep("Gmtod");
#endif
 
    affineTransform
      = fStep->GetPreStepPoint()->GetTouchable()->GetHistory()
        ->GetTopTransform();
  }	

  G4ThreeVector theGlobalPoint(xm[0]* TG4G3Units::Length(),
                               xm[1]* TG4G3Units::Length(),		       
			       xm[2]* TG4G3Units::Length()); 
  G4ThreeVector theLocalPoint;
  if (iflag == 1) 
    theLocalPoint = affineTransform.TransformPoint(theGlobalPoint);
  else
    // if ( iflag == 2)
    theLocalPoint = affineTransform.TransformAxis(theGlobalPoint);

  xd[0] = theLocalPoint.x()/TG4G3Units::Length();
  xd[1] = theLocalPoint.y()/TG4G3Units::Length();
  xd[2] = theLocalPoint.z()/TG4G3Units::Length();
} 
 
//_____________________________________________________________________________
void TG4StepManager::Gdtom(Float_t* xd, Float_t* xm, Int_t iflag) 
{ 
// Transforms a position from the current volume reference frame
// to the world reference frame.
//
//  Geant3 desription:
//  ==================
//  Computes coordinates XM (Master Reference System
//  knowing the coordinates XD (Detector Ref System)
//  The local reference system can be initialized by
//    - the tracking routines and GDTOM used in GUSTEP
//    - a call to GSCMED(NLEVEL,NAMES,NUMBER)
//        (inverse routine is GMTOD)
// 
//   If IFLAG=1  convert coordinates
//      IFLAG=2  convert direction cosinus
//
// ---


  G4double* dxd = TG4GeometryServices::Instance()->CreateG4doubleArray(xd, 3);
  G4double* dxm = TG4GeometryServices::Instance()->CreateG4doubleArray(xm, 3);

  Gdtom(dxd, dxm, iflag);

  for (G4int i=0; i<3; i++) {
    xd[i] = dxd[i];
    xm[i] = dxm[i]; 
  }   

  delete [] dxd;
  delete [] dxm;
} 
 
//_____________________________________________________________________________
void TG4StepManager::Gdtom(Double_t* xd, Double_t* xm, Int_t iflag) 
{ 
// Transforms a position from the current volume reference frame
// to the world reference frame.
//
//  Geant3 desription:
//  ==================
//  Computes coordinates XM (Master Reference System
//  knowing the coordinates XD (Detector Ref System)
//  The local reference system can be initialized by
//    - the tracking routines and GDTOM used in GUSTEP
//    - a call to GSCMED(NLEVEL,NAMES,NUMBER)
//        (inverse routine is GMTOD)
// 
//   If IFLAG=1  convert coordinates
//      IFLAG=2  convert direction cosinus
//
// ---

  G4AffineTransform affineTransform;

  if (fStepStatus == kVertex) {
    G4Navigator* navigator =
      G4TransportationManager::GetTransportationManager()->
        GetNavigatorForTracking();
	
    affineTransform = navigator->GetLocalToGlobalTransform();
  }
  else if (fStepStatus == kBoundary) {

#ifdef MCDEBUG
    CheckStep("Gdtom");
#endif

    affineTransform
      = fStep->GetPostStepPoint()->GetTouchable()->GetHistory()
        ->GetTopTransform().Inverse();
  }	
  else {

#ifdef MCDEBUG
    CheckStep("Gdtom");
#endif

    affineTransform
      = fStep->GetPreStepPoint()->GetTouchable()->GetHistory()
        ->GetTopTransform().Inverse();
  }	

  G4ThreeVector theLocalPoint(xd[0]*TG4G3Units::Length(),
                              xd[1]*TG4G3Units::Length(),
			      xd[2]*TG4G3Units::Length()); 
  G4ThreeVector theGlobalPoint;
  if(iflag == 1)
       theGlobalPoint = affineTransform.TransformPoint(theLocalPoint);
  else if( iflag == 2)
       theGlobalPoint = affineTransform.TransformAxis(theLocalPoint);
  else 
    TG4Globals::Warning(
      "TG4StepManager::Gdtom(...,iflag): iflag is not in 1..2");

  xm[0] = theGlobalPoint.x()/TG4G3Units::Length();
  xm[1] = theGlobalPoint.y()/TG4G3Units::Length();
  xm[2] = theGlobalPoint.z()/TG4G3Units::Length();
} 
 
//_____________________________________________________________________________
Double_t TG4StepManager::MaxStep() const
{   
// Returns maximum step allowed in the current logical volume
// by User Limits.
// ---

  G4LogicalVolume* curLogVolume
    = GetCurrentPhysicalVolume()->GetLogicalVolume();

  // check this
  G4UserLimits* userLimits 
    = curLogVolume->GetUserLimits();

  G4double maxStep;
  if (userLimits == 0) { 
    G4String text = "User Limits are not defined for log volume ";
    text = text + curLogVolume->GetName();
    TG4Globals::Warning(text);
    return FLT_MAX;
  }
  else { 
    const G4Track& trackRef = *(fTrack);
    maxStep = userLimits->GetMaxAllowedStep(trackRef); 
    maxStep /= TG4G3Units::Length(); 
    return maxStep;
  }  
}

//_____________________________________________________________________________
Int_t TG4StepManager::GetMaxNStep() const
{   
// Not yet implemented.
// ---

  return TG4SteppingAction::Instance()->GetMaxNofSteps(); 
}

//_____________________________________________________________________________
void TG4StepManager::TrackPosition(TLorentzVector& position) const
{ 
// Current particle position (in the world reference frame)
// and the local time since the current track is created
// (position of the PostStepPoint).
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  // get position
  // check if this is == to PostStepPoint position !!
  G4ThreeVector positionVector = fTrack->GetPosition();
  positionVector *= 1./(TG4G3Units::Length());   
     
  // local time   
  G4double time = fTrack->GetLocalTime();
  time /= TG4G3Units::Time();
    
  SetTLorentzVector(positionVector, time, position);
}

//_____________________________________________________________________________
void TG4StepManager::TrackPosition(Double_t& x, Double_t& y, Double_t& z) const
{ 
// Current particle position (in the world reference frame)
// and the local time since the current track is created
// (position of the PostStepPoint).
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  // get position
  // check if this is == to PostStepPoint position !!
  G4ThreeVector positionVector = fTrack->GetPosition();
  positionVector *= 1./(TG4G3Units::Length());   
     
  // local time   
  G4double time = fTrack->GetLocalTime();
  time /= TG4G3Units::Time();
    
  x = positionVector.x();
  y = positionVector.y();
  z = positionVector.z();
}

//_____________________________________________________________________________
Int_t TG4StepManager::GetMedium() const
{   
// Returns the second index of the current material (corresponding to
// G3 tracking medium index).
// --- 

  // current logical volume
  G4LogicalVolume* curLV = GetCurrentPhysicalVolume()->GetLogicalVolume();

  // medium index  
  TG4GeometryServices* geometryServices = TG4GeometryServices::Instance();
  return geometryServices->GetMediumId(curLV);
}

//_____________________________________________________________________________
void TG4StepManager::TrackMomentum(TLorentzVector& momentum) const
{  
// Current particle "momentum" (px, py, pz, Etot).
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  G4ThreeVector momentumVector = fTrack->GetMomentum(); 
  momentumVector *= 1./(TG4G3Units::Energy());   

  G4double energy = fTrack->GetDynamicParticle()->GetTotalEnergy();
  energy /= TG4G3Units::Energy();  

  SetTLorentzVector(momentumVector, energy, momentum);
}

//_____________________________________________________________________________
void TG4StepManager::TrackMomentum(Double_t& px, Double_t& py, Double_t&pz,
                                   Double_t& etot) const
{  
// Current particle "momentum" (px, py, pz, Etot).
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  G4ThreeVector momentumVector = fTrack->GetMomentum(); 
  momentumVector *= 1./(TG4G3Units::Energy());   

  px = momentumVector.x();
  py = momentumVector.y();
  pz = momentumVector.z();

  etot = fTrack->GetDynamicParticle()->GetTotalEnergy();
  etot /= TG4G3Units::Energy();  
}

//_____________________________________________________________________________
void TG4StepManager::TrackVertexPosition(TLorentzVector& position) const
{ 
// The vertex particle position (in the world reference frame)
// and the local time since the current track is created.
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  // position
  G4ThreeVector positionVector = fTrack->GetVertexPosition();
  positionVector *= 1./(TG4G3Units::Length());   
     
  // local time 
  // to be checked  
  G4double time = fTrack->GetLocalTime();
  time /= TG4G3Units::Time();
      
  SetTLorentzVector(positionVector, time, position);
}

//_____________________________________________________________________________
void TG4StepManager::TrackVertexMomentum(TLorentzVector& momentum) const
{  
// The vertex particle "momentum" (px, py, pz, Ekin)
// to do: change Ekin -> Etot 
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  G4ThreeVector momentumVector = fTrack->GetVertexMomentumDirection(); 
  momentumVector *= 1./(TG4G3Units::Energy());   

  G4double energy = fTrack->GetVertexKineticEnergy();
  energy /= TG4G3Units::Energy();  

  SetTLorentzVector(momentumVector, energy, momentum);
}

//_____________________________________________________________________________
Double_t TG4StepManager::TrackStep() const
{   
// Returns the current step length.
// ---

  G4double length;
  if (fStepStatus == kNormalStep) {

#ifdef MCDEBUG
    CheckStep("TrackStep");    
#endif

    length = fStep->GetStepLength();
    length /= TG4G3Units::Length();
  }  
  else 
    length = 0;

  return length;
}

//_____________________________________________________________________________
Double_t TG4StepManager::TrackLength() const
{
// Returns the length of the current track from its origin.
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  G4double length = fTrack->GetTrackLength();
  length /= TG4G3Units::Length();
  return length;
}

//_____________________________________________________________________________
Double_t TG4StepManager::TrackTime() const
{
// Returns the local time since the current track is created.
// Comment:
// in Geant4: there is also defined proper time as
// the proper time of the dynamical particle of the current track.
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif
  
  G4double time = fTrack->GetLocalTime();
  time /= TG4G3Units::Time();
  return time;
}

//_____________________________________________________________________________
Double_t TG4StepManager::Edep() const
{   
// Returns total energy deposit in this step.
// ---

  G4double energyDeposit;
  if (fStepStatus == kNormalStep) {

#ifdef MCDEBUG
    CheckStep("Edep");
#endif

    energyDeposit = fStep->GetTotalEnergyDeposit();
    energyDeposit /= TG4G3Units::Energy();
  }
  else   
    energyDeposit = 0;

  return energyDeposit;
}

//_____________________________________________________________________________
Int_t TG4StepManager::TrackPid() const
{   
// Returns the current particle PDG encoding.
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  G4ParticleDefinition* particle
    = fTrack->GetDynamicParticle()->GetDefinition();
  G4int charge 
    = G4int(fTrack->GetDynamicParticle()->GetCharge()/eplus);
    
  // ask TG4ParticlesManager to get PDG encoding 
  // (in order to get PDG from extended TDatabasePDG
  // in case the standard PDG code is not defined)
  G4int pdgEncoding 
    = TG4ParticlesManager::Instance()
      ->GetPDGEncodingFast(particle, charge);

  return pdgEncoding;
}

//_____________________________________________________________________________
Double_t TG4StepManager::TrackCharge() const
{   
// Returns the current particle charge.
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  G4double charge
    = fTrack->GetDynamicParticle()->GetDefinition()
      ->GetPDGCharge();
  charge /= TG4G3Units::Charge();	
  return charge;
}

//_____________________________________________________________________________
Double_t TG4StepManager::TrackMass() const
{   
// Returns current particle rest mass.
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  G4double mass
    = fTrack->GetDynamicParticle()->GetDefinition()
      ->GetPDGMass();
  mass /= TG4G3Units::Mass();	
  return mass;
}

//_____________________________________________________________________________
Double_t TG4StepManager::Etot() const
{   
// Returns total energy of the current particle.
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  G4double energy
    = fTrack->GetDynamicParticle()->GetTotalEnergy();
  energy /= TG4G3Units::Energy();  
  return energy;
}

//_____________________________________________________________________________
Bool_t TG4StepManager::IsTrackInside() const
{   
// Returns true if particle does not cross geometrical boundary
// and is not in vertex.
// ---

  if (fStepStatus == kNormalStep  && !(IsTrackExiting()) ) {
    // track is always inside during a normal step
    return true; 
  }    

  return false;    
}

//_____________________________________________________________________________
Bool_t TG4StepManager::IsTrackEntering() const
{   
// Returns true if particle cross a geometrical boundary
// or is in vertex.
// ---

  if (fStepStatus != kNormalStep) {
    // track is entering during a vertex or boundary step
    return true;  
  }
  
  return false;  
}

//_____________________________________________________________________________
Bool_t TG4StepManager::IsTrackExiting() const
{   
// Returns true if particle cross a geometrical boundary.
// ---

  if (fStepStatus == kNormalStep) {

#ifdef MCDEBUG
    CheckStep("IsTrackExiting");
#endif    

    if (fStep->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) 
       return true;  
  }
  
  return false;  
}

//_____________________________________________________________________________
Bool_t TG4StepManager::IsTrackOut() const
{   
// Returns true if particle cross the world boundary
// at post-step point.
// ---

  if (fStepStatus == kVertex) return false;

#ifdef MCDEBUG
  CheckStep("IsTrackCut");
#endif

  // check
  G4StepStatus status
    = fStep->GetPostStepPoint()->GetStepStatus();
  if (status == fWorldBoundary)
    return true; 
  else
    return false;
}

//_____________________________________________________________________________
Bool_t TG4StepManager::IsTrackStop() const
{   
// Returns true if particle has stopped 
// or has been killed, suspended or postponed to next event.
//
// Possible track status from G4:
//   fAlive,             // Continue the tracking
//   fStopButAlive,      // Invoke active rest physics processes and
//                            // and kill the current track afterward
//   fStopAndKill,       // Kill the current track
//   fKillTrackAndSecondaries,
//                       // Kill the current track and also associated
//                       // secondaries.
//   fSuspend,           // Suspend the current track
//   fPostponeToNextEvent
//                       // Postpones the tracking of thecurrent track 
//                       // to the next event.
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  // check
  G4TrackStatus status
     = fTrack->GetTrackStatus();
  if ((status == fStopAndKill) ||  
      (status == fKillTrackAndSecondaries) ||
      (status == fSuspend) ||
      (status == fPostponeToNextEvent)) {
    return true; 
  }
  else
    return false; 
}

//_____________________________________________________________________________
Bool_t TG4StepManager::IsTrackDisappeared() const
{ 
// Returns true if particle has disappeared 
// (due to any physical process)
// or has been killed, suspended or postponed to next event.
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  // check
  G4TrackStatus status
     = fTrack->GetTrackStatus();
  if ((status == fStopButAlive) ||  
      (status == fKillTrackAndSecondaries) ||
      (status == fSuspend) ||
      (status == fPostponeToNextEvent)) {
    return true; 
  }
  else
    return false;
}

//_____________________________________________________________________________
Bool_t TG4StepManager::IsTrackAlive() const
{   
// Returns true if particle continues tracking.
// ---

#ifdef MCDEBUG
  CheckTrack();
#endif

  G4TrackStatus status
     = fTrack->GetTrackStatus();
  if (status == fAlive)
    return true; 
  else
    return false; 
}

//_____________________________________________________________________________
Bool_t TG4StepManager::IsNewTrack() const
{
// Returns true when track performs the first step.
// ---

  if (fStepStatus == kVertex)
    return true;
  else  
    return false;
}

//_____________________________________________________________________________
Int_t TG4StepManager::NSecondaries() const
{
// Returns the number of secondary particles generated 
// in the current step.
// ---

#ifdef MCDEBUG
  CheckSteppingManager();
#endif

  G4int nofSecondaries = 0;
  nofSecondaries += fSteppingManager->GetfN2ndariesAtRestDoIt();
  nofSecondaries += fSteppingManager->GetfN2ndariesAlongStepDoIt();
  nofSecondaries += fSteppingManager->GetfN2ndariesPostStepDoIt();

  return nofSecondaries;
}

//_____________________________________________________________________________
void TG4StepManager::GetSecondary(Int_t index, Int_t& particleId, 
                          TLorentzVector& position, TLorentzVector& momentum)
{
// Fills the parameters (particle encoding, position, momentum)
// of the generated secondary particle which is specified by index.
// !! Check if indexing of secondaries is same !!
// ---

#ifdef MCDEBUG
  CheckSteppingManager();
#endif

  G4int nofSecondaries = NSecondaries();
  G4TrackVector* secondaryTracks = fSteppingManager->GetSecondary();

#ifdef MCDEBUG
  if (!secondaryTracks) {
    TG4Globals::Exception(
      "TG4StepManager::GetSecondary(): secondary tracks vector is empty");
  }
  
  if (index >= nofSecondaries) {
    TG4Globals::Exception(
      "TG4StepManager::GetSecondary(): wrong secondary track index.");
  }
#endif
  
  // the index of the first secondary of this step
  G4int startIndex 
    = secondaryTracks->size() - nofSecondaries;
         // (the secondaryTracks vector contains secondaries 
         // produced by the track at previous steps, too)
  G4Track* track 
    = (*secondaryTracks)[startIndex + index]; 
   
  // particle encoding
  particleId 
    = track->GetDynamicParticle()->GetDefinition()->GetPDGEncoding();
 
  // position & time
  G4ThreeVector positionVector = track->GetPosition();
  positionVector *= 1./(TG4G3Units::Length());
  G4double time = track->GetLocalTime();
  time /= TG4G3Units::Time();
  SetTLorentzVector(positionVector, time, position);

  // momentum & energy
  G4ThreeVector momentumVector = track->GetMomentum();	
  G4double energy = track->GetDynamicParticle()->GetTotalEnergy();
  energy /= TG4G3Units::Energy();
  SetTLorentzVector(momentumVector, energy, momentum);
}

//_____________________________________________________________________________
TMCProcess TG4StepManager::ProdProcess(Int_t isec) const
{
// The process that has produced the secondary particles specified 
// with isec index in the current step.
// ---

  G4int nofSecondaries = NSecondaries();
  if (fStepStatus == kVertex || !nofSecondaries) return kPNoProcess;

#ifdef MCDEBUG
  CheckStep("ProdProcess");
#endif

  G4TrackVector* secondaryTracks = fSteppingManager->GetSecondary();
 
#ifdef MCDEBUG
  // should never happen
  if (!secondaryTracks) {
    TG4Globals::Exception(
      "TG4StepManager::ProdProcess(): secondary tracks vector is empty.");

    return kPNoProcess;  
  }    

  if (isec >= nofSecondaries) {
    TG4Globals::Exception(
      "TG4StepManager::GetSecondary(): wrong secondary track index.");

    return kPNoProcess;  
  }
#endif

  // the index of the first secondary of this step
  G4int startIndex 
    = secondaryTracks->size() - nofSecondaries;
         // the secondaryTracks vector contains secondaries 
         // produced by the track at previous steps, too

  // the secondary track with specified isec index
  G4Track* track = (*secondaryTracks)[startIndex + isec]; 
   
  const G4VProcess* kpProcess = track->GetCreatorProcess(); 
  
  TMCProcess mcProcess 
   = TG4PhysicsManager::Instance()->GetMCProcess(kpProcess);
  
  // distinguish kPDeltaRay from kPEnergyLoss  
  if (mcProcess == kPEnergyLoss) mcProcess = kPDeltaRay;
  
  return mcProcess;
}

//_____________________________________________________________________________
Int_t TG4StepManager::StepProcesses(TArrayI& processes) const
{
// Fills the array of processes that were active in the current step
// and returns the number of them.
// TBD: Distinguish between kPDeltaRay and kPEnergyLoss
// ---

 if (fStepStatus == kVertex) {
   G4int nofProcesses = 1;
   processes.Set(nofProcesses);
   processes[0] = kPNull;
   return nofProcesses;
 }  
   
#ifdef MCDEBUG
  CheckSteppingManager();
  CheckStep("StepProcesses");
#endif

  // along step processes
  G4ProcessVector* processVector 
    = fStep->GetTrack()->GetDefinition()->GetProcessManager()
        ->GetAlongStepProcessVector();
  G4int nofAlongStep = processVector->entries();
  
  // process defined step
  const G4VProcess* kpLastProcess 
    = fStep->GetPostStepPoint()->GetProcessDefinedStep();

  // set array size
  processes.Set(nofAlongStep+1);
     // maximum number of processes:
     // nofAlongStep (along step) - 1 (transportations) + 1 (post step process)
     // + possibly 1 (additional process if kPLightScattering)
     // => nofAlongStep + 1
 
  // fill array with (nofAlongStep-1) along step processes 
  TG4PhysicsManager* physicsManager = TG4PhysicsManager::Instance();
  G4int counter = 0;  
  for (G4int i=0; i<nofAlongStep; i++) {
    G4VProcess* g4Process = (*processVector)[i];    
    // do not fill transportation along step process
    if (g4Process->GetProcessName() != "Transportation")
      processes[counter++] = physicsManager->GetMCProcess(g4Process);
  }
    
  // fill array with 1 or 2 (if kPLightScattering) last process
  processes[counter++] = physicsManager->GetMCProcess(kpLastProcess);
  if (processes[counter-1] == kPLightScattering) {
     // add reflection/absorption as additional process
     processes[counter++] = physicsManager->GetOpBoundaryStatus(kpLastProcess);
  }	
    
  return counter;  
}
