// $Id: TG4TrackInformation.h,v 1.3 2004/11/10 11:39:27 brun Exp $
/// \ingroup event
//
/// \class TG4TrackInformation
/// \brief Defines additional track information.
///
/// Author: I. Hrivnacova

#ifndef TG4_TRACK_INFORMATION_H
#define TG4_TRACK_INFORMATION_H

#include <G4VUserTrackInformation.hh>
#include <G4Allocator.hh>
#include <globals.hh>

class TG4TrackInformation : public G4VUserTrackInformation
{
  public:
    TG4TrackInformation();
    TG4TrackInformation(G4int trackParticleID);
    TG4TrackInformation(G4int trackParticleID, G4int parentParticleID);
    virtual ~TG4TrackInformation();
   
    // operators
    // required by G4
    inline void *operator new(size_t);
      // Override "new" for "G4Allocator".
    inline void operator delete(void *trackInformation);
      // Override "delete" for "G4Allocator".
      
    // methods
    virtual void Print() const;  

    // set methods
    void SetTrackParticleID(G4int trackParticleID);
    void SetParentParticleID(G4int parentParticleID);

    // get methods
    G4int GetTrackParticleID() const;
    G4int GetParentParticleID() const;

  private:
    // data members
    G4int  fTrackParticleID; //the index of track particle in VMC stack
    G4int  fParentParticleID;//the index of parent track in VMC stack
};

// inline methods
#include "TG4TrackInformation.icc"

#endif //TG4_TRACK_INFORMATION_H
