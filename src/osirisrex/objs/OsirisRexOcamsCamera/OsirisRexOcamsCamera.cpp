/**
 * @file
 *
 *   Unless noted otherwise, the portions of Isis written by the USGS are public
 *   domain. See individual third-party library and package descriptions for 
 *   intellectual property information,user agreements, and related information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or implied,
 *   is made by the USGS as to the accuracy and functioning of such software 
 *   and related material nor shall the fact of distribution constitute any such 
 *   warranty, and no responsibility is assumed by the USGS in connection 
 *   therewith.
 *
 *   For additional information, launch
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html in a browser or see 
 *   the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */

#include "OsirisRexOcamsCamera.h"

#include <QString>

#include "CameraDetectorMap.h"
#include "CameraDistortionMap.h"
#include "CameraFocalPlaneMap.h"
#include "CameraGroundMap.h"
#include "CameraSkyMap.h"
#include "IString.h"
#include "iTime.h"
#include "NaifStatus.h"

using namespace std;

namespace Isis {

  /**
   * Constructs an OSIRIS-REx Camera Model using the image labels. This model supports MapCam,
   * PolyCam, and SamCam images.
   *  
   * @param lab Pvl label from an Osiris Rex MapCam image. 
   */
  OsirisRexOcamsCamera::OsirisRexOcamsCamera(Cube &cube) : FramingCamera(cube) {

    NaifStatus::CheckErrors();

    m_spacecraftNameLong = "OSIRIS-REx";
    m_spacecraftNameShort = "OSIRIS-REx";

    int ikCode = naifIkCode();

    if (ikCode == -64361) {
      m_instrumentNameLong = "Mapping Camera";
      m_instrumentNameShort = "MapCam";
    }
    else if (ikCode <= -64362) {
      m_instrumentNameLong = "Sampling Camera";
      m_instrumentNameShort = "SamCam";
    }
    else if (ikCode <= -64360) {
      m_instrumentNameLong = "PolyMath Camera";
      m_instrumentNameShort = "PolyCam";
    }


    SetFocalLength();

    // The instrument kernel contains pixel pitch in microns, so convert it to mm.
    QString pitch = "INS" + toString(naifIkCode()) + "_PIXEL_SIZE";
    SetPixelPitch(getDouble(pitch) / 1000.0);

    // Get the start time in et
    Pvl &lab = *cube.label();
    PvlGroup inst = lab.findGroup("Instrument", Pvl::Traverse);

    // Set the observation time and exposure duration
    QString clockCount = inst["SpacecraftClockStartCount"];
    double startTime = getClockTime(clockCount).Et();
    double exposureDuration = ((double) inst["ExposureDuration"]) / 1000.0;
    pair<iTime, iTime> shuttertimes = ShutterOpenCloseTimes(startTime, exposureDuration);

    // Add half exposure duration to get time at center of image
    iTime centerTime = shuttertimes.first.Et() + exposureDuration / 2.0;

    // Setup detector map
    new CameraDetectorMap(this);

    // Setup focal plane map
    CameraFocalPlaneMap *focalMap = new CameraFocalPlaneMap(this, naifIkCode());

    // The instrument kernel contains a CCD_CENTER keyword instead of BORESIGHT_LINE
    // and BORESIGHT_SAMPLE keywords.
    focalMap->SetDetectorOrigin(
        Spice::getDouble("INS" + toString(naifIkCode()) + "_CCD_CENTER", 0) + 1.0,
        Spice::getDouble("INS" + toString(naifIkCode()) + "_CCD_CENTER", 1) + 1.0);

    // Setup distortion map
    new CameraDistortionMap(this);

    // Setup the ground and sky map
    new CameraGroundMap(this);
    new CameraSkyMap(this);

    setTime(centerTime);
    LoadCache();
    NaifStatus::CheckErrors();
  }


  //! Destroys the MapCamera object.
  OsirisRexOcamsCamera::~OsirisRexOcamsCamera() {
  }


  /**
   * The frame ID for the spacecraft (or instrument) used by the Camera-matrix 
   * Kernel. For this camera model, the spacecraft frame is used, represented 
   * by the frame ID -64000. 
   *  
   * @return @b int The appropriate code for the Camera-matrix Kernel. 
   */
  int OsirisRexOcamsCamera::CkFrameId() const { 
    return -64000; 
  }


  /**
   * The frame ID for the reference coordinate system used by the Camera-matrix 
   * Kernel. For this mission, the reference frame J2000, represented by the 
   * frame ID 1. 
   *
   * @return @b int The appropriate reference frame ID code for the 
   *         Camera-matrix Kernel.
   */
  int OsirisRexOcamsCamera::CkReferenceId() const { 
    return 1; 
  }


  /** 
   * The reference frame ID for the Spacecraft Kernel is 1, representing J2000.
   *
   * @return @b int The appropriate frame ID code for the Spacecraft Kernel.
   */
  int OsirisRexOcamsCamera::SpkReferenceId() const { 
    return 1; 
  }


  /**
   * Returns the shutter open and close times. The user should pass in the
   * ExposureDuration keyword value, converted to seconds, and the StartTime
   * keyword value, converted to ephemeris time. The StartTime keyword value
   * from the labels represents the time at the start of the observation, as
   * noted in the Osiris Rex EDR image SIS. This method uses the FramingCamera
   * class implementation, returning the given time value as the shutter open
   * and the sum of the time value and exposure duration as the shutter close.
   *
   * @param exposureDuration ExposureDuration keyword value from the labels,
   *                         converted to seconds.
   * @param time The StartTime keyword value from the labels, converted to
   *             ephemeris time.
   *
   * @return @b pair < @b iTime, @b iTime > The first value is the shutter
   *         open time and the second is the shutter close time.
   *
   * @see http://pds-imaging.jpl.nasa.gov/documentation/clementine_edrsis.pdf
   * @author 2011-05-03 Jeannie Walldren
   * @internal
   *   @history 2011-05-03 Jeannie Walldren - Original version.
   */
  pair<iTime, iTime> OsirisRexOcamsCamera::ShutterOpenCloseTimes(double time,
                                                               double exposureDuration) {
    return FramingCamera::ShutterOpenCloseTimes(time, exposureDuration);
  }

}


/**
 * This is the function that is called in order to instantiate a MapCamera
 * object.
 *
 * @param lab Cube labels
 *
 * @return Isis::Camera* OsirisRexOcamsCamera
 */
extern "C" Isis::Camera *OsirisRexOcamsCameraPlugin(Isis::Cube &cube) {
  return new Isis::OsirisRexOcamsCamera(cube);
}
