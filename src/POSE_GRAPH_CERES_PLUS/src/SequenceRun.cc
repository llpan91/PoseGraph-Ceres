#include<iostream>
#include "SequenceRun.h"
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "ORBmatcher.h"
#include "converter.h"
#include "config.h"

using namespace std;

namespace POSE_GRAPH
{

SequenceRun::SequenceRun()
{
    // Load camera parameters from config file
    // we keep the timestamp file, but you can fix the timestamp
    float fx = Config::get<float>("Camera.fx");
    float fy = Config::get<float>("Camera.fy");
    float cx = Config::get<float>("Camera.cx");
    float cy = Config::get<float>("Camera.cy");

    cv::Mat K = cv::Mat::eye(3,3,CV_32FC1);
    K.at<float>(0,0) = fx;
    K.at<float>(1,1) = fy;
    K.at<float>(0,2) = cx;
    K.at<float>(1,2) = cy;
    K.copyTo(mK);

    cv::Mat DistCoef(4,1,CV_32FC1);
    DistCoef.at<float>(0) = Config::get<float>("Camera.k1");
    DistCoef.at<float>(1) = Config::get<float>("Camera.k2");
    DistCoef.at<float>(2) = Config::get<float>("Camera.p1");
    DistCoef.at<float>(3) = Config::get<float>("Camera.p1");
    const float k3 = Config::get<float>("Camera.k3");
    if(k3!=0)
    {
        DistCoef.resize(5);
        DistCoef.at<float>(4) = k3;
    }
    DistCoef.copyTo(mDistCoef);
    mbf = Config::get<float>("Camera.bf");


    cout << endl << "Camera Parameters: " << endl;
    cout << "- fx: " << fx << endl;
    cout << "- fy: " << fy << endl;
    cout << "- cx: " << cx << endl;
    cout << "- cy: " << cy << endl;
    cout << "- k1: " << DistCoef.at<float>(0) << endl;
    cout << "- k2: " << DistCoef.at<float>(1) << endl;
    if(DistCoef.rows==5)
        cout << "- k3: " << DistCoef.at<float>(4) << endl;
    cout << "- p1: " << DistCoef.at<float>(2) << endl;
    cout << "- p2: " << DistCoef.at<float>(3) << endl;

    // Load ORB parameters
    int nFeatures = Config::get<int>("ORBextractor.nFeatures");
    float fScaleFactor = Config::get<float>("ORBextractor.scaleFactor");
    int nLevels = Config::get<int>("ORBextractor.nLevels");
    int fIniThFAST = Config::get<int>("ORBextractor.iniThFAST");
    int fMinThFAST = Config::get<int>("ORBextractor.minThFAST");

    mpORBextractorLeft = new ORBextractor(nFeatures,fScaleFactor,nLevels,fIniThFAST,fMinThFAST);


    mpORBextractorRight = new ORBextractor(nFeatures,fScaleFactor,nLevels,fIniThFAST,fMinThFAST);


    cout << endl  << "ORB Extractor Parameters: " << endl;
    cout << "- Number of Features: " << nFeatures << endl;
    cout << "- Scale Levels: " << nLevels << endl;
    cout << "- Scale Factor: " << fScaleFactor << endl;
    cout << "- Initial Fast Threshold: " << fIniThFAST << endl;
    cout << "- Minimum Fast Threshold: " << fMinThFAST << endl;

    mThDepth = mbf*(float)Config::get<int>("ThDepth")/fx;
    cout << endl << "Depth Threshold (Close/Far Points): " << mThDepth << endl;



}

bool SequenceRun::GrabImageStereo(const cv::Mat &imRectLeft, const cv::Mat &imRectRight, const double &timestamp)
{
    mImGray = imRectLeft;
    cv::Mat imGrayRight = imRectRight;

    if(mImGray.channels()==3)
    {

        cvtColor(mImGray,mImGray,CV_BGR2GRAY);
        cvtColor(imGrayRight,imGrayRight,CV_BGR2GRAY);

    }
    else if(mImGray.channels()==4)
    {

        cvtColor(mImGray,mImGray,CV_BGRA2GRAY);
        cvtColor(imGrayRight,imGrayRight,CV_BGRA2GRAY);

    }

    mCurrentFrame = Frame(mImGray,imGrayRight,timestamp,mpORBextractorLeft,mpORBextractorRight,mK,mDistCoef,mbf,mThDepth);

    /*now we need to set the pose of current frame*/

    mCurrentFrame.SetPose(gd.getFrameTcw(mCurrentFrame.mnId));

    /*after set the frame pose we can generate the mappoints*/

    /*new add we modified it in 2017.09.12*/
    /*notice: the mappoint is in the current camera coordination*/

    GenerateFrameMappoints();

    return true;
}

bool SequenceRun::GenerateFrameMappoints()
{
    /*in GrabImageStereo() we have set the frame pose(groundtruth)*/

    /*now we need to create the mappoints*/

    // We sort points according to their measured depth by the stereo sensor
    vector<pair<float,int> > vDepthIdx;
    vDepthIdx.reserve(mCurrentFrame.N);

    for(int i=0; i<mCurrentFrame.N;i++)
    {
        float z = mCurrentFrame.mvDepth[i];
        if(z>0)
        {
            vDepthIdx.push_back(make_pair(z,i));
        }
    }

    if(vDepthIdx.empty())
        return false;

    sort(vDepthIdx.begin(),vDepthIdx.end());

    float middleDistance = vDepthIdx[vDepthIdx.size()/2 +1].first;
    float minDistance,maxDistance;
    minDistance = middleDistance * 0.3;
    maxDistance = middleDistance * 1.4;
    // We insert points (minDistance < depth < MaxDistance)

    int nPoints = 0;
    for(size_t j=0; j<vDepthIdx.size();j++)
    {
        int i = vDepthIdx[j].second;
        float depth = vDepthIdx[j].first;

        if(depth > minDistance && depth < maxDistance)
        {
            /*generate the mappoints*/
            /*modified !! notice*/
            cv::Mat x3D = mCurrentFrame.pixel2Camera(i);
            //cout << "mappoints 3d = \n" << x3D << endl;
            MapPoint* pMap = new MapPoint(x3D, &mCurrentFrame, i);

            mCurrentFrame.mvpMapPoints[i] = pMap;
            mCurrentFrame.mvbOutlier[i] = false;
            nPoints++;

        }

    }
    cout << "In frame " << mCurrentFrame.mnId << " We got " << nPoints << " Mappoints with depth!" << endl;
//    cout << "minDistance = " << minDistance << endl;
//    cout << "maxDistance = " << maxDistance << endl;
//    cout << "generate mappoints: " << nPoints << endl;


    return true;
}


}



