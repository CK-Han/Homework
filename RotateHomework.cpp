#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#include <Ogre.h>
#include <OIS/OIS.h>


using namespace Ogre;

class ESCListener : public FrameListener {
	OIS::Keyboard *mKeyboard;

public:
	ESCListener(OIS::Keyboard *keyboard) : mKeyboard(keyboard) {}
	bool frameStarted(const FrameEvent &evt)
	{
		mKeyboard->capture();
		return !mKeyboard->isKeyDown(OIS::KC_ESCAPE);
	}
};


class MainListener : public FrameListener {
	OIS::Keyboard *mKeyboard;
	Root* mRoot;
	SceneNode *mProfessorNode, *mFishRotateCenterNode;

public:
	MainListener(Root* root, OIS::Keyboard *keyboard) : mKeyboard(keyboard), mRoot(root)
	{
		mProfessorNode = mRoot->getSceneManager("main")->getSceneNode("Professor");
		mFishRotateCenterNode = mRoot->getSceneManager("main")->getSceneNode("FishRotateCenter");
	}

	bool frameStarted(const FrameEvent &evt)
	{
#define		FISH_ANGULAR_SPEED			200.0f
#define		PROFESSOR_ANGULAR_SPEED		80.0f
#define		PROFESSOR_MAX_ZPOS			250.0f
#define		PROFESSOR_MIN_ZPOS			-250.0f

		static float fProfessorSpeed = 100.0f;
		static bool bProfessorRotating = false;
		static float fProfessorAccumulatedDeg = 0.0f;
		static float fFishAccumulatedDeg = 0.0f;

		Vector3 vAxisY(0.0f, 1.0f, 0.0f);

		if (bProfessorRotating)
		{
			float thisFrameRotatingDegree = PROFESSOR_ANGULAR_SPEED  * evt.timeSinceLastFrame;
			fProfessorAccumulatedDeg += thisFrameRotatingDegree;
			mProfessorNode->rotate(vAxisY, Degree(thisFrameRotatingDegree));

			if (fProfessorAccumulatedDeg >= 180.0f)
			{
				bProfessorRotating = false;
				fProfessorAccumulatedDeg = 0.0f;
			}
		}
		else  // false == bProfessorRotating
		{
			mProfessorNode->translate(0.0f, 0.0f, fProfessorSpeed * evt.timeSinceLastFrame);

			if (mProfessorNode->getPosition().z < PROFESSOR_MIN_ZPOS || PROFESSOR_MAX_ZPOS < mProfessorNode->getPosition().z)
			{
				Vector3 vProfessorPosition = mProfessorNode->getPosition();
				vProfessorPosition.z = vProfessorPosition.z > 0 ? PROFESSOR_MAX_ZPOS : PROFESSOR_MIN_ZPOS;
				mProfessorNode->setPosition(vProfessorPosition);

				fProfessorSpeed *= -1.0f;
				bProfessorRotating = true;
			}
		}

		mFishRotateCenterNode->rotate(vAxisY, -Degree(FISH_ANGULAR_SPEED * evt.timeSinceLastFrame));
		return true;
	}

};

class LectureApp {

	Root* mRoot;
	RenderWindow* mWindow;
	SceneManager* mSceneMgr;
	Camera* mCamera;
	Viewport* mViewport;
	OIS::Keyboard* mKeyboard;
	OIS::InputManager *mInputManager;

	MainListener* mMainListener;
	ESCListener* mESCListener;

public:

	LectureApp() {}

	~LectureApp() {}

	void go(void)
	{
		// OGRE의 메인 루트 오브젝트 생성
#if !defined(_DEBUG)
		mRoot = new Root("plugins.cfg", "ogre.cfg", "ogre.log");
#else
		mRoot = new Root("plugins_d.cfg", "ogre.cfg", "ogre.log");
#endif


		// 초기 시작의 컨피규레이션 설정 - ogre.cfg 이용
		if (!mRoot->restoreConfig()) {
			if (!mRoot->showConfigDialog()) return;
		}

		mWindow = mRoot->initialise(true, "Rotate : Copyleft by Dae-Hyun Lee");


		// ESC key를 눌렀을 경우, 오우거 메인 렌더링 루프의 탈출을 처리
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
		OIS::ParamList pl;
		mWindow->getCustomAttribute("WINDOW", &windowHnd);
		windowHndStr << windowHnd;
		pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
		mInputManager = OIS::InputManager::createInputSystem(pl);
		mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, false));


		mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "main");
		mCamera = mSceneMgr->createCamera("main");


		mCamera->setPosition(0.0f, 100.0f, 700.0f);
		mCamera->lookAt(0.0f, 100.0f, 0.0f);

		mCamera->setNearClipDistance(5.0f);

		mViewport = mWindow->addViewport(mCamera);
		mViewport->setBackgroundColour(ColourValue(0.0f, 0.0f, 0.5f));
		mCamera->setAspectRatio(Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));


		ResourceGroupManager::getSingleton().addResourceLocation("resource.zip", "Zip");
		ResourceGroupManager::getSingleton().addResourceLocation("fish.zip", "Zip");
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		mSceneMgr->setAmbientLight(ColourValue(1.0f, 1.0f, 1.0f));

		// 좌표축 표시
		Ogre::Entity* mAxesEntity = mSceneMgr->createEntity("Axes", "axes.mesh");
		mSceneMgr->getRootSceneNode()->createChildSceneNode("AxesNode", Ogre::Vector3(0, 0, 0))->attachObject(mAxesEntity);
		mSceneMgr->getSceneNode("AxesNode")->setScale(5, 5, 5);

		_drawGridPlane();


		Entity* entity1 = mSceneMgr->createEntity("Professor", "DustinBody.mesh");
		SceneNode* node1 = mSceneMgr->getRootSceneNode()->createChildSceneNode("Professor", Vector3(0.0f, 0.0f, 0.0f));
		node1->attachObject(entity1);
		
		SceneNode* node2 = node1->createChildSceneNode("FishRotateCenter", Vector3::ZERO);
		node2->setInheritOrientation(false);

		Entity* entity3 = mSceneMgr->createEntity("Fish", "fish.mesh");
		SceneNode* node3 = node2->createChildSceneNode("Fish", Vector3(0.0f, 0.0f, 100.0f));
		node3->attachObject(entity3);
		node3->setScale(5.0f, 5.0f, 5.0f);
		
		mESCListener = new ESCListener(mKeyboard);
		mRoot->addFrameListener(mESCListener);

		mMainListener = new MainListener(mRoot, mKeyboard);
		mRoot->addFrameListener(mMainListener);


		mRoot->startRendering();

		mInputManager->destroyInputObject(mKeyboard);
		OIS::InputManager::destroyInputSystem(mInputManager);

		delete mRoot;
	}

private:
	void _drawGridPlane(void)
	{
		Ogre::ManualObject* gridPlane = mSceneMgr->createManualObject("GridPlane");
		Ogre::SceneNode* gridPlaneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("GridPlaneNode");

		Ogre::MaterialPtr gridPlaneMaterial = Ogre::MaterialManager::getSingleton().create("GridPlanMaterial", "General");
		gridPlaneMaterial->setReceiveShadows(false);
		gridPlaneMaterial->getTechnique(0)->setLightingEnabled(true);
		gridPlaneMaterial->getTechnique(0)->getPass(0)->setDiffuse(1, 1, 1, 0);
		gridPlaneMaterial->getTechnique(0)->getPass(0)->setAmbient(1, 1, 1);
		gridPlaneMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(1, 1, 1);

		gridPlane->begin("GridPlaneMaterial", Ogre::RenderOperation::OT_LINE_LIST);
		for (int i = 0; i < 21; i++)
		{
			gridPlane->position(-500.0f, 0.0f, 500.0f - i * 50);
			gridPlane->position(500.0f, 0.0f, 500.0f - i * 50);

			gridPlane->position(-500.f + i * 50, 0.f, 500.0f);
			gridPlane->position(-500.f + i * 50, 0.f, -500.f);
		}

		gridPlane->end();

		gridPlaneNode->attachObject(gridPlane);
	}
};


#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
	int main(int argc, char *argv[])
#endif
	{
		LectureApp app;

		try {

			app.go();

		}
		catch (Ogre::Exception& e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
			std::cerr << "An exception has occured: " <<
				e.getFullDescription().c_str() << std::endl;
#endif
		}

		return 0;
	}

#ifdef __cplusplus
}
#endif

