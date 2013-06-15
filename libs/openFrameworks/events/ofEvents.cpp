#include "ofEvents.h"
#include "ofAppRunner.h"
#include "ofBaseApp.h"
#include "ofUtils.h"
#include "ofGraphics.h"
#include <set>

static const double MICROS_TO_SEC = .000001;
static const double MICROS_TO_MILLIS = .001;

static unsigned long long   timeThen = 0, oneSec = 0;
static float    			targetRate = 0;
static float				fps = 60;
static unsigned long long	microsForFrame = 0;
static unsigned long long	lastFrameTime = 0;
static bool      			bFrameRateSet = 0;
static int      			nFramesForFPS = 0;
static int      			nFrameCount	  = 0;

// core events instance & arguments
ofCoreEvents & ofEvents(){
	static ofCoreEvents * events = new ofCoreEvents;
	return *events;
}

ofEventArgs voidEventArgs;


static int	currentMouseX=0, currentMouseY=0;
static int	previousMouseX=0, previousMouseY=0;
static bool		bPreMouseNotSet;
static set<int> pressedMouseButtons;
static set<int> pressedKeys;

static bool bEscQuits = true;


//--------------------------------------
void ofSetFrameRate(int _targetRate){
	// given this FPS, what is the amount of millis per frame
	// that should elapse?

	// --- > f / s

	if (_targetRate == 0){
		bFrameRateSet = false;
	}else{
		bFrameRateSet	= true;
		targetRate		= _targetRate;
		microsForFrame	= 1000000.0 / (double)targetRate;
	}
}

//--------------------------------------
float ofGetFrameRate(){
	return fps;
}

//--------------------------------------
float ofGetTargetFrameRate(){
	return targetRate;
}

//--------------------------------------
double ofGetLastFrameTime(){
	return lastFrameTime*MICROS_TO_SEC;
}

//--------------------------------------
int ofGetFrameNum(){
	return nFrameCount;
}

//--------------------------------------
bool ofGetMousePressed(int button){ //by default any button
	if(button==-1) return pressedMouseButtons.size();
	return pressedMouseButtons.find(button)!=pressedMouseButtons.end();
}

//--------------------------------------
bool ofGetKeyPressed(int key){
	if(key==-1) return pressedKeys.size();
	return pressedKeys.find(key)!=pressedKeys.end();
}

//--------------------------------------
int ofGetMouseX(){
	return currentMouseX;
}

//--------------------------------------
int ofGetMouseY(){
	return currentMouseY;
}

//--------------------------------------
int ofGetPreviousMouseX(){
	return previousMouseX;
}

//--------------------------------------
int ofGetPreviousMouseY(){
	return previousMouseY;
}

//--------------------------------------
void ofSetEscapeQuitsApp(bool bQuitOnEsc){
	bEscQuits = bQuitOnEsc;
}

void exitApp(){
	ofLog(OF_LOG_VERBOSE,"OF app is being terminated!");
	OF_EXIT_APP(0);
}



//------------------------------------------
void ofNotifySetup(){
	ofBaseApp * ofAppPtr = ofGetAppPtr();
	
	if(ofAppPtr){
		ofAppPtr->setup();
	}
	ofNotifyEvent( ofEvents().setup, voidEventArgs );
}

//------------------------------------------
void ofNotifyUpdate(){
	// calculate sleep time to adjust to target fps
	unsigned long long timeNow = ofGetElapsedTimeMicros();
	if (nFrameCount != 0 && bFrameRateSet == true){
		unsigned long long diffMicros = timeNow - timeThen;
		if(diffMicros < microsForFrame){
			unsigned long long waitMicros = microsForFrame - diffMicros;
			#ifdef TARGET_WIN32
				Sleep(waitMicros*MICROS_TO_MILLIS);
			#else
				usleep(waitMicros);
			#endif
		}
	}

	// calculate fps
	timeNow = ofGetElapsedTimeMicros();

	if(nFrameCount==0){
		timeThen = timeNow;
		if(bFrameRateSet)	fps = targetRate;
	}else{
		unsigned long long oneSecDiff = timeNow-oneSec;

		if( oneSecDiff  >= 1000000 ){
			fps = nFramesForFPS/(oneSecDiff*MICROS_TO_SEC);
			oneSec  = timeNow;
			nFramesForFPS = 0;
		}else{
			fps = fps*.99 + nFramesForFPS/(oneSecDiff*MICROS_TO_SEC)*.01;
		}
		nFramesForFPS++;


		lastFrameTime 	= timeNow-timeThen;
		timeThen    	= timeNow;
	}

	// update renderer, application and notify update event
	ofGetCurrentRenderer()->update();

	ofBaseApp * ofAppPtr = ofGetAppPtr();
	
	if(ofAppPtr){
		ofAppPtr->update();
	}
	ofNotifyEvent( ofEvents().update, voidEventArgs );
}

//------------------------------------------
void ofNotifyDraw(){
	if(ofGetCurrentRenderer()){
		ofBaseApp * ofAppPtr = ofGetAppPtr();

		if(ofAppPtr){
			ofAppPtr->draw();
		}
		ofNotifyEvent( ofEvents().draw, voidEventArgs );
	}

	nFrameCount++;
}

//------------------------------------------
void ofNotifyKeyPressed(int key){
	ofBaseApp * ofAppPtr = ofGetAppPtr();
	static ofKeyEventArgs keyEventArgs;

	pressedKeys.insert(key);

	if(ofAppPtr){
		ofAppPtr->keyPressed(key);
	}
	
	keyEventArgs.key = key;
	ofNotifyEvent( ofEvents().keyPressed, keyEventArgs );
	
	
	if (key == OF_KEY_ESC && bEscQuits == true){				// "escape"
		exitApp();
	}
	
	
}

//------------------------------------------
void ofNotifyKeyReleased(int key){
	ofBaseApp * ofAppPtr = ofGetAppPtr();
	static ofKeyEventArgs keyEventArgs;

	pressedKeys.erase(key);

	if(ofAppPtr){
		ofAppPtr->keyReleased(key);
	}
	
	keyEventArgs.key = key;
	ofNotifyEvent( ofEvents().keyReleased, keyEventArgs );
}


//------------------------------------------
void ofNotifyKeyEvent(const ofKeyEventArgs & keyEvent){
	switch(keyEvent.type){
		case ofKeyEventArgs::Pressed:
			ofNotifyKeyPressed(keyEvent.key);
			break;
		case ofKeyEventArgs::Released:
			ofNotifyKeyReleased(keyEvent.key);
			break;
		
	}
}

//------------------------------------------
void ofNotifyMouseEvent(const ofMouseEventArgs & mouseEvent){
	switch(mouseEvent.type){
		case ofMouseEventArgs::Moved:
			ofNotifyMouseMoved(mouseEvent.x,mouseEvent.y);
			break;
		case ofMouseEventArgs::Dragged:
			ofNotifyMouseDragged(mouseEvent.x,mouseEvent.y,mouseEvent.button);
			break;
		case ofMouseEventArgs::Pressed:
			ofNotifyMousePressed(mouseEvent.x,mouseEvent.y,mouseEvent.button);
			break;
		case ofMouseEventArgs::Released:
			ofNotifyMouseReleased(mouseEvent.x,mouseEvent.y,mouseEvent.button);
			break;
		
	}
}

//------------------------------------------
void ofNotifyMousePressed(int x, int y, int button){
	ofBaseApp * ofAppPtr = ofGetAppPtr();
	static ofMouseEventArgs mouseEventArgs;
    if( bPreMouseNotSet ){
		previousMouseX	= x;
		previousMouseY	= y;
		bPreMouseNotSet	= false;
	}else{
		previousMouseX = currentMouseX;
		previousMouseY = currentMouseY;
	}
    
	currentMouseX = x;
	currentMouseY = y;
	pressedMouseButtons.insert(button);

	if(ofAppPtr){
		ofAppPtr->mousePressed(x,y,button);
		ofAppPtr->mouseX = x;
		ofAppPtr->mouseY = y;
	}

	mouseEventArgs.x = x;
	mouseEventArgs.y = y;
	mouseEventArgs.button = button;
	ofNotifyEvent( ofEvents().mousePressed, mouseEventArgs );
}

//------------------------------------------
void ofNotifyMouseReleased(int x, int y, int button){
	ofBaseApp * ofAppPtr = ofGetAppPtr();
	static ofMouseEventArgs mouseEventArgs;

	if( bPreMouseNotSet ){
		previousMouseX	= x;
		previousMouseY	= y;
		bPreMouseNotSet	= false;
	}else{
		previousMouseX = currentMouseX;
		previousMouseY = currentMouseY;
	}

	currentMouseX = x;
	currentMouseY = y;
	pressedMouseButtons.erase(button);

	if(ofAppPtr){
		ofAppPtr->mouseReleased(x,y,button);
		ofAppPtr->mouseReleased();
		ofAppPtr->mouseX = x;
		ofAppPtr->mouseY = y;
	}

	mouseEventArgs.x = x;
	mouseEventArgs.y = y;
	mouseEventArgs.button = button;
	ofNotifyEvent( ofEvents().mouseReleased, mouseEventArgs );
}

//------------------------------------------
void ofNotifyMouseDragged(int x, int y, int button){
	ofBaseApp * ofAppPtr = ofGetAppPtr();
	static ofMouseEventArgs mouseEventArgs;

	if( bPreMouseNotSet ){
		previousMouseX	= x;
		previousMouseY	= y;
		bPreMouseNotSet	= false;
	}else{
		previousMouseX = currentMouseX;
		previousMouseY = currentMouseY;
	}

	currentMouseX = x;
	currentMouseY = y;
	
	if(ofAppPtr){
		ofAppPtr->mouseDragged(x,y,button);
		ofAppPtr->mouseX = x;
		ofAppPtr->mouseY = y;
	}

	mouseEventArgs.x = x;
	mouseEventArgs.y = y;
	mouseEventArgs.button = button;
	ofNotifyEvent( ofEvents().mouseDragged, mouseEventArgs );
}

//------------------------------------------
void ofNotifyMouseMoved(int x, int y){
	ofBaseApp * ofAppPtr = ofGetAppPtr();
	static ofMouseEventArgs mouseEventArgs;
	if( bPreMouseNotSet ){
		previousMouseX	= x;
		previousMouseY	= y;
		bPreMouseNotSet	= false;
	}else{
		previousMouseX = currentMouseX;
		previousMouseY = currentMouseY;
	}

	currentMouseX = x;
	currentMouseY = y;
	
	if(ofAppPtr){
		ofAppPtr->mouseMoved(x,y);
		ofAppPtr->mouseX = x;
		ofAppPtr->mouseY = y;
	}

	mouseEventArgs.x = x;
	mouseEventArgs.y = y;
	ofNotifyEvent( ofEvents().mouseMoved, mouseEventArgs );
}

//------------------------------------------
void ofNotifyExit(){
	ofBaseApp * ofAppPtr = ofGetAppPtr();
	if(ofAppPtr){
		ofAppPtr->exit();
	}
	ofNotifyEvent( ofEvents().exit, voidEventArgs );
}

//------------------------------------------
void ofNotifyWindowResized(int width, int height){
	static ofResizeEventArgs resizeEventArgs;

	ofBaseApp * ofAppPtr = ofGetAppPtr();
	if(ofAppPtr){
		ofAppPtr->windowResized(width, height);
	}
	
	resizeEventArgs.width	= width;
	resizeEventArgs.height	= height;
	ofNotifyEvent( ofEvents().windowResized, resizeEventArgs );
}

//------------------------------------------
void ofNotifyDragEvent(ofDragInfo info){
	ofBaseApp * ofAppPtr = ofGetAppPtr();
	if(ofAppPtr){
		ofAppPtr->dragEvent(info);
	}
	
	ofNotifyEvent(ofEvents().fileDragEvent, info);
}

//------------------------------------------
void ofSendMessage(ofMessage msg){
	ofBaseApp * ofAppPtr = ofGetAppPtr();
	if(ofAppPtr){
		ofAppPtr->gotMessage(msg);
	}
	
	ofNotifyEvent(ofEvents().messageEvent, msg);
}

//------------------------------------------
void ofSendMessage(string messageString){
	ofMessage msg(messageString);
	ofSendMessage(msg);
}

void ofNotifyWindowEntry( int state ) {
	
	static ofEntryEventArgs entryArgs;

	ofBaseApp * ofAppPtr = ofGetAppPtr();
	if(ofAppPtr){
		ofAppPtr->windowEntry(state);
	}
	
	entryArgs.state = state;
	ofNotifyEvent(ofEvents().windowEntered, entryArgs);
	
}
