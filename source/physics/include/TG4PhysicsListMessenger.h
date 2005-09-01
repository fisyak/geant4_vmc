// $Id: TG4PhysicsListMessenger.h,v 1.4 2005/05/17 13:43:57 brun Exp $
/// \ingroup physics
//
/// \class TG4PhysicsListMessenger
/// \brief Messenger class that defines commands for the Geant4 VMC default 
/// physics list.
///
/// Author: I. Hrivnacova

#ifndef TG4_PHYSICS_LIST_MESSENGER_H
#define TG4_PHYSICS_LIST_MESSENGER_H 

#include <G4UImessenger.hh>
#include <globals.hh>

class TG4ModularPhysicsList;

class G4UIdirectory;
class G4UIcmdWithoutParameter;
class G4UIcmdWithABool;
class G4UIcmdWithADouble;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;

class TG4PhysicsListMessenger: public G4UImessenger
{
  public:
    TG4PhysicsListMessenger(TG4ModularPhysicsList* physicsList); 
    // --> protected   
    // TG4PhysicsListMessenger();
    // TG4PhysicsListMessenger(const TG4PhysicsListMessenger& right);
    virtual ~TG4PhysicsListMessenger();
   
    // methods 
    virtual void SetNewValue(G4UIcommand* command, G4String string);
    
  protected:
    TG4PhysicsListMessenger();  
    TG4PhysicsListMessenger(const TG4PhysicsListMessenger& right);

    // operators
    TG4PhysicsListMessenger& operator=(const TG4PhysicsListMessenger& right);

  private:
    // data members
    TG4ModularPhysicsList*  fPhysicsList;     //associated class

    // commands    
    G4UIcmdWithABool*  fSetEMCmd;             //setEM command   
    G4UIcmdWithABool*  fSetMuonCmd;           //setMuon command   
    G4UIcmdWithABool*  fSetHadronCmd;         //setHadron command   
    G4UIcmdWithABool*  fSetOpticalCmd;        //setOptical command   
    G4UIcmdWithABool*  fSetSpecialCutsCmd;    //setSpecialCuts command   
    G4UIcmdWithABool*  fSetSpecialControlsCmd;//setSpecialControls command 
    G4UIcmdWithABool*  fSetStepLimiterCmd;    //setStepLimiter command 
    G4UIcmdWithADouble*       fRangeCutCmd;   //rangeCut command  
    G4UIcmdWithoutParameter*  fProcessActivationCmd; //.
                                              //setProcessActivation command
    G4UIcmdWithAnInteger*     fSetCerenkovMaxPhotonsCmd; 					          
};                                            //setCerenkovMaxPhotons command

#endif //TG4_PHYSICS_LIST_MESSENGER_H
