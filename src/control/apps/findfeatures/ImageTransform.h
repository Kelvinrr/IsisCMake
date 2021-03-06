#ifndef ImageTransform_h
#define ImageTransform_h
/**
 * @file
 * $Revision: 7311 $ 
 * $Date: 2016-12-26 22:19:31 -0700 (Mon, 26 Dec 2016) $ 
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

#include <QList>
#include <QString>
#include <QSharedPointer>

#include <opencv2/opencv.hpp>

#include "FeatureMatcherTypes.h"

namespace Isis {

/**
 * @brief OpenCV-based image transformation base class
 *  
 *  
 * @author 2014-07-01 Kris Becker 
 * @internal 
 *   @history 2014-07-01 Kris Becker - Original Version 
 */

class ImageTransform {
  public:
    typedef cv::Rect  RectArea;

    ImageTransform();
    ImageTransform(const QString &name);

    virtual ~ImageTransform();

    QString name() const;

    virtual cv::Mat render(const cv::Mat &image) const;
    virtual cv::Point2f forward(const cv::Point2f &point) const;
    virtual cv::Point2f inverse(const cv::Point2f &point) const;


    static RectArea boundingBox(const cv::Mat &tform, 
                                const RectArea &region,
                                const cv::Size &bounds);
    static RectArea transformedSize(const cv::Mat &tmat, 
                                    const cv::Size &imSize, 
                                    cv::Mat &tmat_t);

    static std::vector<cv::Point2f> corners(const cv::Size &imSize);
    static std::vector<cv::Point2f> corners(const RectArea &region);


    static cv::Mat translation(const double xoffset, const double yoffset);

  private:
    QString          m_name;
};

///!<   Shared ImageTransformpointer that everyone can use 
typedef QSharedPointer<ImageTransform> SharedImageTransform;

///!<  Define a ImageTransform list
typedef QList<SharedImageTransform> ImageTransformQList;

}  // namespace Isis
#endif
