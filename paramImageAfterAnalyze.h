#ifndef PARAMIMAGEAFTERANALYZE_H
#define PARAMIMAGEAFTERANALYZE_H

struct paramImageAfterAnalyze {
    paramImageAfterAnalyze() {}
    paramImageAfterAnalyze(const int &_width,
                           const int &_height,
                           const int &_x,
                           const int &_y,
                           const double &_fullFaceMask,
                           const double &_lowerFaceMask,
                           const double &_noMask,
                           const double &_otherMask,
                           const QVector<QPoint> &_cordLst ) :
    width(_width), height(_height), x(_x), y(_y),
    fullFaceMask(_fullFaceMask), lowerFaceMask(_lowerFaceMask),
    noMask(_noMask), otherMask(_otherMask),
    cord(_cordLst)
    { }

    void setData(const int &_width,
                 const int &_height,
                 const int &_x,
                 const int &_y,
                 const double &_fullFaceMask,
                 const double &_lowerFaceMask,
                 const double &_noMask,
                 const double &_otherMask,
                 const QVector<QPoint> &_cordLst ) {
        width = _width;
        height = _height;
        x = _x;
        y = _y;
        fullFaceMask = _fullFaceMask;
        lowerFaceMask =_lowerFaceMask;
        noMask = _noMask;
        otherMask = _otherMask;
        cord = _cordLst;
    }

    void clear() {
        width = 0;
        height = 0;
        x = 0;
        y = 0;
        fullFaceMask = 0.0;
        lowerFaceMask = 0.0;
        noMask = 0.0;
        otherMask = 0.0;
        cord.clear();
    }

    int height, width, x, y;

    double fullFaceMask, lowerFaceMask,
           noMask, otherMask;

    QVector<QPoint> cord;
};

struct inputJsonData
{
    inputJsonData():
    sign_rotate(false), sign_orientation(false),
    sign_demographic(true), sign_attributes(true),
    sign_landmarks(true), sign_liveness(true),
    sign_quality(true), sign_masks(true),
    minHeight(0), maxHeight(0), threshold(0.8) { }

    QString sign_rotate, sign_orientation,
            sign_demographic, sign_attributes,
            sign_landmarks, sign_liveness,
            sign_quality, sign_masks;
    int minHeight, maxHeight;
    double threshold;
};

#endif // PARAMIMAGEAFTERANALYZE_H
