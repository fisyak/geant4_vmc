#ifndef A01DetectorConstMessenger_h
#define A01DetectorConstMessenger_h 1

//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//

//
//------------------------------------------------
// The Virtual Monte Carlo examples
// Copyright (C) 2007 - 2012 Ivana Hrivnacova
// All rights reserved.
//
// For the licensing terms see geant4_vmc/LICENSE.
// Contact: vmc@pcroot.cern.ch
//-------------------------------------------------

/// \file A01DetectorConstMessenger.hh 
/// \brief Definition of the A01DetectorConstMessenger class 
///
/// Geant4 example A01 adapted to Virtual Monte Carlo

class A01DetectorConstruction;
class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;

#include "G4UImessenger.hh"
#include "globals.hh"

/// \ingroup A01
/// \brief The detector construction messenger (defined via Geant4)

class A01DetectorConstMessenger: public G4UImessenger
{
  public:
    A01DetectorConstMessenger(A01DetectorConstruction* mpga);
    ~A01DetectorConstMessenger();

    virtual void SetNewValue(G4UIcommand * command,G4String newValues);
    virtual G4String GetCurrentValue(G4UIcommand * command);

  private:
    A01DetectorConstruction * fTarget;

    G4UIdirectory *             fMydetDirectory;
    G4UIcmdWithADoubleAndUnit*  fArmCmd;
};

#endif

