#include <opencv2/opencv.hpp>

#include "lbpfeatures.h"
#include "cascadeclassifier.h"

#include <iostream>

using namespace cv;

CvLBPFeatureParams::CvLBPFeatureParams()
{
    maxCatCount = 256;
    name = LBPF_NAME;
}

void CvLBPEvaluator::init(const CvFeatureParams *_featureParams, int _maxSampleCount, Size _winSize)
{
    CV_Assert( _maxSampleCount > 0);
    sum.create((int)_maxSampleCount, (_winSize.width + 1) * (_winSize.height + 1), CV_32SC1);
    CvFeatureEvaluator::init( _featureParams, _maxSampleCount, _winSize );
}

void CvLBPEvaluator::setImage(const Mat &img, uchar clsLabel, int idx,
    const std::vector<int> &feature_ind)
{
    CV_DbgAssert( !sum.empty() );
    CvFeatureEvaluator::setImage( img, clsLabel, idx, feature_ind );
    integral( img, sum );
    cur_sum = sum;
    offset_ = int(sum.ptr<int>(1) - sum.ptr<int>());
    for (size_t i = 0; i < feature_ind.size(); ++i) {
        features[feature_ind[i]].calcPoints(offset_);
    }
}

void CvLBPEvaluator::writeFeatures( FileStorage &fs, const Mat& featureMap ) const
{
    _writeFeatures( features, fs, featureMap );
}

void CvLBPEvaluator::generateFeatures()
{
    int offset = winSize.width + 1;
    for( int x = 0; x < winSize.width; x++ )
        for( int y = 0; y < winSize.height; y++ )
            for( int w = 1; w <= winSize.width / 3; w++ )
                for( int h = 1; h <= winSize.height / 3; h++ )
                    if ( (x+3*w <= winSize.width) && (y+3*h <= winSize.height) )
                        features.push_back( Feature(offset, x, y, w, h ) );
    numFeatures = (int)features.size();
}

CvLBPEvaluator::Feature::Feature()
{
    rect = cvRect(0, 0, 0, 0);
}

CvLBPEvaluator::Feature::Feature( int offset, int x, int y, int _blockWidth, int _blockHeight )
{
    x_ = x;
    y_ = y;
    block_w_ = _blockWidth;
    block_h_ = _blockHeight;
    offset_ = offset;
    calcPoints(offset);
}

void CvLBPEvaluator::Feature::calcPoints(int offset)
{
    Rect tr = rect = cvRect(x_, y_, block_w_, block_h_);
    CV_SUM_OFFSETS( p[0], p[1], p[4], p[5], tr, offset )
    tr.x += 2*rect.width;
    CV_SUM_OFFSETS( p[2], p[3], p[6], p[7], tr, offset )
    tr.y +=2*rect.height;
    CV_SUM_OFFSETS( p[10], p[11], p[14], p[15], tr, offset )
    tr.x -= 2*rect.width;
    CV_SUM_OFFSETS( p[8], p[9], p[12], p[13], tr, offset )
    offset_ = offset;
}

void CvLBPEvaluator::Feature::write(FileStorage &fs) const
{
    fs << CC_RECT << "[:" << rect.x << rect.y << rect.width << rect.height << "]";
}
