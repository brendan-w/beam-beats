#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

	ofSetupOpenGL(1024,768, OF_WINDOW); // <-------- setup the GL context

    ofGLFWWindowSettings settings;
    settings.decorated = false;

    auto window = static_pointer_cast<ofAppGLFWWindow>(ofCreateWindow(settings));
    shared_ptr<ofApp> app (new ofApp());
    ofRunApp(window, app);

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	//ofRunApp( new ofApp());
    ofRunMainLoop();
}
