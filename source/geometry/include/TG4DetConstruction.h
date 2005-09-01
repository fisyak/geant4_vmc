// $Id: TG4DetConstruction.h,v 1.8 2005/05/17 13:43:57 brun Exp $
/// \ingroup geometry
//
/// \class TG4DetConstruction
/// \brief Detector construction for building geometry using 
/// TVirtualMCApplication.
///
/// Author: I. Hrivnacova

#ifndef TG4_DET_CONSTRUCTION_H
#define TG4_DET_CONSTRUCTION_H

#include "TG4Verbose.h"
#include "TG4DetConstructionMessenger.h"
#include "TG4MagneticFieldType.h"

#ifdef USE_VGM
#include "TG4XmlVGMMessenger.h"
#endif    

#include <G4VUserDetectorConstruction.hh>
#include <globals.hh>
#include <vector>

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4MagneticField;

class TG4DetConstruction : public G4VUserDetectorConstruction,
                           public TG4Verbose
{
  public:
    TG4DetConstruction();
    // --> protected
    // TG4DetConstruction(const TG4DetConstruction& right);
    virtual ~TG4DetConstruction();

    // methods
    virtual G4VPhysicalVolume* Construct();

    // set methods
    void SetFieldType(TG4MagneticFieldType fieldType);
    void SetUniformFieldValue(G4double fieldValue);
    void SetReadGeometry(G4bool readGeometry);
    void SetWriteGeometry(G4bool writeGeometry);
    
  protected:
    TG4DetConstruction(const TG4DetConstruction& right);

    // operators
    TG4DetConstruction& operator=(const TG4DetConstruction& right);
    
  private:    
    // methods
    void CreateMagneticField();

    // data members
    TG4DetConstructionMessenger  fMessenger;        //messenger
#ifdef USE_VGM
    TG4XmlVGMMessenger           fAGDDMessenger;    //XML messenger
    TG4XmlVGMMessenger           fGDMLMessenger;    //XML messenger
#endif
    TG4MagneticFieldType         fMagneticFieldType;//magnetic field type
    G4MagneticField*             fMagneticField;    //magnetic field
    G4bool             fReadGeometry;     // option for reading geometry 
                                          // from g3calls.dat file (g3tog4)
    G4bool             fWriteGeometry;    // option for writiong geometry
                                          // to g3calls.dat file (g3tog4)   
}; 

// inline methods

inline void TG4DetConstruction::SetReadGeometry(G4bool readGeometry) {
  /// Set option for reading geometry from g3calls.dat file (g3tog4)
  fReadGeometry = readGeometry; 
}

inline void TG4DetConstruction::SetWriteGeometry(G4bool writeGeometry) { 
  /// Set option for writing geometry to g3calls.dat file (g3tog4)
  fWriteGeometry = writeGeometry; 
}

#endif //TG4_DET_CONSTRUCTION_H

