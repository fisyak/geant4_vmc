// $Id: TG4VRunConfiguration.h,v 1.4 2004/11/10 11:39:28 brun Exp $
/// \ingroup run
//
/// \class TG4VRunConfiguration
/// \brief Abstract base class for creating Geant4 user action classes
/// using VMC
///
/// Abstract base class that takes care of creating  all user defined classes 
/// that will be initialized and managed by Geant4 kernel (G4RunManager).
/// It has two pure virtual methods CreateUserConfiguration(), 
/// IsSpecialStacking() that have to be implemented by a derived class.
//
// Author: I. Hrivnacova

#ifndef TG4V_RUN_CONFIGURATION_H
#define TG4V_RUN_CONFIGURATION_H

#include <globals.hh>

class TG4TrackingAction;
class TG4SteppingAction;
class TG4SDConstruction;

class G4VUserDetectorConstruction;
class G4VUserPrimaryGeneratorAction;
class G4VUserPhysicsList;
class G4UserRunAction;
class G4UserEventAction;
class G4UserStackingAction;
class G4RunManager;

class TG4VRunConfiguration
{
  public:
    TG4VRunConfiguration();
    // --> protected
    // TG4VRunConfiguration(const TG4VRunConfiguration& right);
    virtual ~TG4VRunConfiguration();

    // methods
    void ConfigureRunManager(G4RunManager* runManager);

    // get methods
    virtual G4bool      IsSpecialStacking() const = 0;
    G4VUserPhysicsList* GetPhysicsList() const;
    TG4SDConstruction*  GetSDConstruction() const;

  protected:
    TG4VRunConfiguration(const TG4VRunConfiguration& right);

    // operators
    TG4VRunConfiguration& operator=(const TG4VRunConfiguration& right);

    // methods
    virtual void CreateUserConfiguration() = 0;

    // data members
    G4VUserDetectorConstruction*    fDetectorConstruction; //det construction
    TG4SDConstruction*              fSDConstruction;       //sensitive detectors 
                                                           //construction
    G4VUserPhysicsList*             fPhysicsList;          //physics list
    G4VUserPrimaryGeneratorAction*  fPrimaryGenerator;     //primary generator
    G4UserRunAction*                fRunAction;            //run action
    G4UserEventAction*              fEventAction;          //event action
    TG4TrackingAction*              fTrackingAction;       //tracking action
    TG4SteppingAction*              fSteppingAction;       //stepping action
    G4UserStackingAction*           fStackingAction;       //stacking action
};

#endif //TG4V_RUN_CONFIGURATION_H

