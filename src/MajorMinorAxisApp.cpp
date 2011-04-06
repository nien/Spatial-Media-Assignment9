//
//  MajorMinorAxisApp.cpp
//  MajorMinorAxis
//
//  Created by Nien Lam on 4/6/11.
//  Copyright 2011 Chimera Island. All rights reserved.
//
//
//  Caculating the Major and Minor Axes
//
//  1. Determine centroid of the shape.
//  2. For all pixels in the shape 
//	   a. Compute dX and dY between each pixel and the centroid 
//     b. Square dX and dY 
//     c. Add result of squares to the sums DX2, DY2 
//     d. Add dX*dY to the sum DXDY
//  3. After obtaining sum 
//	   a. Divide by pixel count 
//     b. If the sum DXDY is less than 0, multiply DX2 by -1 
//     c. Major axis = atan2( DY2,DX2 )	...in radians (convert to degrees) 
//     d. Minor axis is equal to major axis+90°
//

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIO.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class MajorMinorAxis : public AppBasic 
{
  public:
	void setup();
	void keyDown( KeyEvent event );
	void update();
	void draw();
	void prepareSettings( Settings *settings );
  
  private:
	// Size of the input images.
	static const int IMG_WIDTH  = 320;
	static const int IMG_HEIGHT = 240;
	static const int IMG_SPACER = 10;

	// For drawing the centroid, major and minor axis.
	static const int MAJOR_AXIS_RADIUS = 100;
	static const int MINOR_AXIS_RADIUS = 50;
	Vec2f mCentroid;
	int   mPixelCount;
	float mMajorAxisAngle;
		
	// Images for background subtraction.
	Channel32f mImageBg;
	Channel32f mImage1;
	Channel32f mImage2;
	Channel32f mImage3;
	Channel32f mImage4;
	Channel32f mImage5;
	
	// Channels for outputing to the screen.
	Channel32f mBackground, mObject;
	Channel32f mResult;
	
	// Threshold value for background subtraction.
	float mThreshold;
	
	// Background subtraction with threshold.
	void subtractBg( Channel32f const &channel1, Channel32f const &channel2, 
					 Channel32f &channelResult, float threshold );

	// Get the centroid of the image.
	Vec2f getCentroid( Channel32f const &channel );
	
	// Get the Major Axis Angle based on image and centroid.
	float getMajorAxisAngle( Channel32f const &channel, Vec2f const &centroid );
};

void MajorMinorAxis::prepareSettings( Settings *settings )
{
	settings->setWindowSize( IMG_WIDTH * 3 + IMG_SPACER * 2, IMG_HEIGHT );
	settings->setFrameRate( 60.0f );
}

void MajorMinorAxis::setup()
{
	// Get image files.
	mImageBg = Channel32f( loadImage( loadResource( "image-bg.jpg" ) ) );
	mImage1  = Channel32f( loadImage( loadResource( "image1.jpg" ) ) );
	mImage2  = Channel32f( loadImage( loadResource( "image2.jpg" ) ) );
	mImage3  = Channel32f( loadImage( loadResource( "image3.jpg" ) ) );
	mImage4  = Channel32f( loadImage( loadResource( "image4.jpg" ) ) );
	mImage5  = Channel32f( loadImage( loadResource( "image5.jpg" ) ) );
	
	// Initialize threshold value.
	mThreshold = .2;

	// Initialize output image.
	mBackground = mImageBg;
	mObject     = mImage1;
	mResult     = Channel32f( IMG_WIDTH, IMG_HEIGHT );
}

void MajorMinorAxis::update()
{
	// Apply background subtraction.
	subtractBg( mBackground, mObject, mResult, mThreshold );
	
	// Get the centroid from the background subtracted result.
	mCentroid = getCentroid( mResult );

	// Get the Major Axis Angle.
	mMajorAxisAngle = getMajorAxisAngle( mResult, mCentroid );
}

void MajorMinorAxis::draw()
{
	gl::clear( Color( 0, 0, 0 ), true );
	gl::color( Color( 1.0f, 1.0f, 1.0f ) );

	// Draw background and object image.
	gl::draw( mBackground, Vec2f(0, 0) );
	gl::draw( mObject, Vec2f( IMG_WIDTH + IMG_SPACER, 0 ) );

	// Draw background subtracted image.
	Vec2f imagePosition = Vec2f( ( IMG_WIDTH + IMG_SPACER ) * 2, 0 );
	gl::draw( mResult, imagePosition );
	
	// Offset center by the image position. 
	Vec2f center = mCentroid + imagePosition;

	// Draw the centroid on the subtracted image.
	gl::color( Color( 1.0f, 0.0f, 0.0f ) );
	gl::drawSolidCircle( center, 7.0f );

	// Draw the major axis.
	Vec2f majorAxis;
	majorAxis.x = cos( mMajorAxisAngle ) * MAJOR_AXIS_RADIUS;
	majorAxis.y = sin( mMajorAxisAngle ) * MAJOR_AXIS_RADIUS;
	gl::color( Color( 1.0f, 0.0f, 1.0f ) );
	glLineWidth( 2.0f );
	gl::drawLine( majorAxis + center, -majorAxis + center );

	// Draw the minor axis which the major axis rotated 90 degrees.
	Vec2f minorAxis;
	minorAxis.x = cos( mMajorAxisAngle + .5 * M_PI ) * MINOR_AXIS_RADIUS;
	minorAxis.y = sin( mMajorAxisAngle + .5 * M_PI ) * MINOR_AXIS_RADIUS;
	gl::color( Color( 0.0f, 1.0f, 0.0f ) );
	glLineWidth( 2.0f );
	gl::drawLine( minorAxis + center, -minorAxis + center );
}

void MajorMinorAxis::keyDown( KeyEvent event ) 
{
	// Change image based on keyboard input.
    if ( event.getChar() == '1' )
	{
		mObject = mImage1;
	}
    else if ( event.getChar() == '2' )
	{	
		mObject = mImage2;
	}
	else if ( event.getChar() == '3' )
	{
		mObject = mImage3;
	}
	else if ( event.getChar() == '4' )
	{
		mObject = mImage4;
	}
	else if ( event.getChar() == '5' )
	{
		mObject = mImage5;
	}
	
	// Change threshold with key press.       
    if ( event.getCode() == KeyEvent::KEY_UP  )
	{
		mThreshold = fminf( 1.0, mThreshold + .01 ); 
	}
    else if ( event.getCode() == KeyEvent::KEY_DOWN  )
	{	
		mThreshold = fmaxf( 0.0, mThreshold - .01 );
	}
}	

void MajorMinorAxis::subtractBg( Channel32f const &channel1, Channel32f const &channel2, 
								 Channel32f &channelResult, float threshold )
{	
	Channel32f::ConstIter iterCh1 = channel1.getIter();
	Channel32f::ConstIter iterCh2 = channel2.getIter();
	Channel32f::Iter      iterCh3 = channelResult.getIter();
	
	while ( iterCh1.line() && iterCh2.line() && iterCh3.line() ) 
	{
		while ( iterCh1.pixel() && iterCh2.pixel() && iterCh3.pixel()  ) 
		{
			if ( abs( iterCh1.v() - iterCh2.v() ) < threshold )
				iterCh3.v() = 0;
			else	
				iterCh3.v() = 255;
		}
	}
}

Vec2f MajorMinorAxis::getCentroid( Channel32f const &channel )
{
	Channel32f::ConstIter iter = channel.getIter();

	float sumX  = 0.0f;
	float sumY  = 0.0f;
	float total = 0;
	
	while ( iter.line() ) 
	{
		while ( iter.pixel() ) 
		{
			if ( iter.v() ) 
			{
				sumX += iter.getPos().x;
				sumY += iter.getPos().y;
				total++;
			}
		}
	}

	// Save the total pixel count.
	mPixelCount = total;
	
	return Vec2f( (sumX / total), (sumY / total) );
}

float MajorMinorAxis::getMajorAxisAngle( Channel32f const &channel, Vec2f const &centroid )
{
	double DX2 = 0;
	double DY2 = 0;
	double DXDY = 0;
	
	Channel32f::ConstIter iter = channel.getIter();
	while ( iter.line() ) 
	{
		while ( iter.pixel() ) 
		{
			if ( iter.v() ) 
			{
				// Get pixel distance from centroid.
				float dx = centroid.x - iter.getPos().x;
				float dy = centroid.y - iter.getPos().y;
				
				// Sum of squares of the distances.
				DX2 += dx * dx;
				DY2 += dy * dy;

				// Keep running sum of dx * dy.
				DXDY += dx * dy;
			}
		}
	}

	// Flip DY2 sign if DXDY is negative.
	if ( DXDY < 0 )
		DY2 *= -1;
	
	return atan2( DY2 / mPixelCount,  DX2 / mPixelCount );
}


CINDER_APP_BASIC( MajorMinorAxis, RendererGl )
