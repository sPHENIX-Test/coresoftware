/*!
 *  \file		Fitter.cc
 *  \brief		Fitter class handles setups for the fitting.
 *  \details	Fitter class handles setups for the fitting like Geometry, Fields, fitter choice, etc.
 *  \author		Haiwang Yu <yuhw@nmsu.edu>
 */

// PHGenFit
#include "Fitter.h"

#include "Track.h"

// ROOT
#include <RVersion.h>  // for ROOT_VERSION, ROOT_VERSION...
#include <TGeoManager.h>

// GenFit
#include <GenFit/AbsKalmanFitter.h>
#include <GenFit/DAF.h>
#include <GenFit/EventDisplay.h>
#include <GenFit/FieldManager.h>
#include <GenFit/FitStatus.h>
#include <GenFit/KalmanFitter.h>
#include <GenFit/KalmanFitterRefTrack.h>
#include <GenFit/MaterialEffects.h>
#include <GenFit/TGeoMaterialInterface.h>
#include <GenFit/Track.h>

// GenFitExp
#include <genfitexp/Field.h>

#include <cassert>
#include <cstddef>
#include <iostream>

namespace genfit
{
  class AbsTrackRep;
}

#define LogDEBUG(exp) std::cout << "DEBUG: " << __FILE__ << ": " << __LINE__ << ": " << (exp) << std::endl
#define LogERROR(exp) std::cout << "ERROR: " << __FILE__ << ": " << __LINE__ << ": " << (exp) << std::endl
#define LogWARNING(exp) std::cout << "WARNING: " << __FILE__ << ": " << __LINE__ << ": " << (exp) << std::endl

namespace PHGenFit
{
  /**
   * @brief Constructs a Fitter using a geometry file and configures field, materials, fitter type, and optional event display.
   *
   * Initializes the geometry from the given TGeo file, installs the provided PHField into GenFit, configures material effects,
   * optionally enables the GenFit event display, and selects the internal GenFit fitter implementation according to fitter_choice.
   *
   * @param tgeo_file_name Path to a ROOT TGeo geometry file to import.
   * @param field Pointer to a PHField used to construct the GenFit field mapping; must not be null.
   * @param fitter_choice Name of the fitter implementation to instantiate (e.g., "KalmanFitterRefTrack", "KalmanFitter", "DafSimple", "DafRef").
   * @param /*track_rep_choice*/ Placeholder for track representation choice (ignored by this constructor).
   * @param doEventDisplay If true, enables and retains a GenFit EventDisplay instance for later use.
   */
  Fitter::Fitter(
      const std::string& tgeo_file_name,
      const PHField* field,
      const std::string& fitter_choice,
      const std::string& /*track_rep_choice*/,
      const bool doEventDisplay)
    : verbosity(1000)
    , _tgeo_manager(new TGeoManager("Default", "Geane geometry"))
    , _doEventDisplay(doEventDisplay)
  {
    TGeoManager::Import(tgeo_file_name.data());

    assert(field);
    genfit::Field* fieldMap = new genfit::Field(field);

    genfit::FieldManager::getInstance()->init(fieldMap);
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());

    // init event display
    if (_doEventDisplay)
    {
      _display = genfit::EventDisplay::getInstance();
    }
    else
    {
      _display = nullptr;
    }

    // init fitter
    if (fitter_choice == "KalmanFitterRefTrack")
    {
      _fitter = new genfit::KalmanFitterRefTrack();
    }
    else if (fitter_choice == "KalmanFitter")
    {  // NOLINT(bugprone-branch-clone)
      _fitter = new genfit::KalmanFitter();
    }
    else if (fitter_choice == "DafSimple")
    {
      _fitter = new genfit::DAF(false);
    }
    else if (fitter_choice == "DafRef")
    {
      _fitter = new genfit::DAF(true);
    }
    else
    {  // NOLINT(bugprone-branch-clone)
      _fitter = new genfit::KalmanFitter();
    }

    genfit::Exception::quiet(true);
  }

  Fitter::~Fitter()
  {
    delete _fitter;
    // delete _tgeo_manager;
    //_tgeo_manager->Delete();
    delete _display;
  }

  int Fitter::processTrack(PHGenFit::Track* track, const bool save_to_evt_disp)
  {
    genfit::Track* fitTrack = track->getGenFitTrack();

#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 00, 0)
    try
    {
      fitTrack->checkConsistency();
    }
    catch (genfit::Exception& e)
    {
      if (verbosity >= 2)
      {
        std::cerr << "genfit::Track::checkConsistency() failed!" << std::endl;
        std::cerr << e.what();
      }
      return -1;
    }
#else
    if (!fitTrack->checkConsistency())
    {
      if (verbosity >= 2) LogWARNING("genfit::Track::checkConsistency() failed!");
      return -1;
    }
#endif
    try
    {
      _fitter->processTrack(fitTrack);
    }
    catch (genfit::Exception& e)
    {
      if (verbosity >= 1)
      {
        std::cerr << "PHGenFit::Fitter::processTrack::Exception: \n";
        std::cerr << e.what();
        std::cerr << "Exception, next track" << std::endl;
      }
      return -1;
    }
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 00, 0)
    try
    {
      fitTrack->checkConsistency();
    }
    catch (genfit::Exception& e)
    {
      if (verbosity >= 2)
      {
        std::cerr << "genfit::Track::checkConsistency() failed!" << std::endl;
        std::cerr << e.what();
      }
      return -1;
    }
#else

    if (!fitTrack->checkConsistency())
    {
      if (verbosity >= 2) LogWARNING("genfit::Track::checkConsistency() failed!");
      return -1;
    }
#endif
    genfit::AbsTrackRep* rep = fitTrack->getCardinalRep();
    if (!fitTrack->getFitStatus(rep)->isFitConverged())
    {
      if (verbosity >= 2)
      {
        LogWARNING("Track could not be fitted successfully! Fit is not converged!");
      }
      return -1;
    }

    if (_display && save_to_evt_disp)
    {
      _display->addEvent(track->getGenFitTrack());
    }

    return 0;
  }

  Fitter* Fitter::getInstance(const std::string& tgeo_file_name,
                              const PHField* field,
                              const std::string& fitter_choice, const std::string& track_rep_choice,
                              const bool doEventDisplay)
  {
    TGeoManager* tgeo_manager = TGeoManager::Import(tgeo_file_name.data(), "Default");
    if (!tgeo_manager)
    {
      LogERROR("No TGeoManager found!");
      return nullptr;
    }

    assert(field);
    genfit::Field* fieldMap = new genfit::Field(field);
    return new Fitter(tgeo_manager, fieldMap, fitter_choice, track_rep_choice, doEventDisplay);
  }

  /**
   * @brief Constructs a Fitter using an existing geometry manager and field map, and configures the chosen GenFit fitter and optional event display.
   *
   * Initializes the GenFit FieldManager and MaterialEffects, creates the selected fitter implementation, and enables event display if requested.
   *
   * @param tgeo_manager Pointer to an existing TGeoManager that provides detector geometry for fitting.
   * @param fieldMap Pointer to a GenFit magnetic field map used during propagation and fitting.
   * @param fitter_choice Enum value selecting which GenFit fitter implementation to instantiate (e.g., KalmanFitter, KalmanFitterRefTrack, DafSimple, DafRef).
   * @param /*track_rep_choice*/ Unused parameter retained for API compatibility; the constructor ignores track representation choice.
   * @param doEventDisplay If true, initializes and retains a GenFit EventDisplay instance for visualizing fitted tracks.
   */
  Fitter::Fitter(TGeoManager* tgeo_manager, genfit::AbsBField* fieldMap,
                 const PHGenFit::Fitter::FitterType& fitter_choice,
                 const PHGenFit::Fitter::TrackRepType& /*track_rep_choice*/,
                 const bool doEventDisplay)
    : verbosity(0)
    , _tgeo_manager(tgeo_manager)
    , _doEventDisplay(doEventDisplay)
  {
    genfit::FieldManager::getInstance()->init(
        fieldMap);
    genfit::MaterialEffects::getInstance()->init(
        new genfit::TGeoMaterialInterface());

    // init event display
    if (_doEventDisplay)
    {
      _display = genfit::EventDisplay::getInstance();
    }
    else
    {
      _display = nullptr;
    }

    // init fitter
    if (fitter_choice == PHGenFit::Fitter::KalmanFitter)
    {
      _fitter = new genfit::KalmanFitter();
    }
    else if (fitter_choice == PHGenFit::Fitter::KalmanFitterRefTrack)
    {
      _fitter = new genfit::KalmanFitterRefTrack();
    }
    else if (fitter_choice == PHGenFit::Fitter::DafSimple)
    {
      _fitter = new genfit::DAF(false);
    }
    else if (fitter_choice == PHGenFit::Fitter::DafRef)
    {
      _fitter = new genfit::DAF(true);
    }
    else
    {
      _fitter = nullptr;
      LogERROR("This fitter not implemented!");
    }
  }

  Fitter* Fitter::getInstance(TGeoManager* tgeo_manager,
                              const PHField* field,
                              const std::string& fitter_choice, const std::string& track_rep_choice,
                              const bool doEventDisplay)
  {
    if (!tgeo_manager)
    {
      LogERROR("No TGeoManager found!");
      return nullptr;
    }

    assert(field);
    genfit::Field* fieldMap = new genfit::Field(field);
    return new Fitter(tgeo_manager, fieldMap, fitter_choice, track_rep_choice, doEventDisplay);
  }

  /**
   * @brief Constructs a Fitter using an existing geometry manager and field map.
   *
   * Initializes the GenFit field manager and material effects, optionally enables
   * the event display, and instantiates the requested fitter implementation.
   *
   * @param tgeo_manager Pointer to an existing TGeoManager that provides geometry.
   * @param fieldMap GenFit magnetic field map used for propagation.
   * @param fitter_choice Name of the fitter to create; supported values:
   *        "KalmanFitterRefTrack", "KalmanFitter", "DafSimple", "DafRef".
   *        If the value is unrecognized, no fitter is created and an error is logged.
   * @param doEventDisplay If true, acquires the GenFit EventDisplay singleton for visualization.
   */
  Fitter::Fitter(TGeoManager* tgeo_manager, genfit::AbsBField* fieldMap,
                 const std::string& fitter_choice, const std::string& /*track_rep_choice*/,
                 const bool doEventDisplay)
    : verbosity(0)
    , _tgeo_manager(tgeo_manager)
    , _doEventDisplay(doEventDisplay)
  {
    genfit::FieldManager::getInstance()->init(
        fieldMap);
    genfit::MaterialEffects::getInstance()->init(
        new genfit::TGeoMaterialInterface());

    // init event display
    if (_doEventDisplay)
    {
      _display = genfit::EventDisplay::getInstance();
    }
    else
    {
      _display = nullptr;
    }

    // init fitter
    if (fitter_choice == "KalmanFitterRefTrack")
    {
      _fitter = new genfit::KalmanFitterRefTrack();
    }
    else if (fitter_choice == "KalmanFitter")
    {
      _fitter = new genfit::KalmanFitter();
    }
    else if (fitter_choice == "DafSimple")
    {
      _fitter = new genfit::DAF(false);
    }
    else if (fitter_choice == "DafRef")
    {
      _fitter = new genfit::DAF(true);
    }
    else
    {
      _fitter = nullptr;
      LogERROR("This fitter not implemented!");
    }
  }

  int Fitter::displayEvent()
  {
    if (_display)
    {
      _display->open();
    }
    else if (verbosity >= 0)
    {
      LogERROR("No genfit::EventDisplay found!");
    }

    return 0;
  }

  Fitter* Fitter::getInstance(TGeoManager* tgeo_manager,
                              const PHField* field,
                              const PHGenFit::Fitter::FitterType& fitter_choice,
                              const PHGenFit::Fitter::TrackRepType& track_rep_choice,
                              const bool doEventDisplay)
  {
    if (!tgeo_manager)
    {
      LogERROR("No TGeoManager found!");
      return nullptr;
    }

    assert(field);
    genfit::Field* fieldMap = new genfit::Field(field);

    return new Fitter(tgeo_manager, fieldMap, fitter_choice, track_rep_choice, doEventDisplay);
  }

}  // namespace PHGenFit
