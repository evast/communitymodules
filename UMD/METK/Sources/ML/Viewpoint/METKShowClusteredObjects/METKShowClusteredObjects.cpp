// Local includes
#include "METK.h"
#include "METKShowClusteredObjects.h"

#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoMaterial.h>
#include "UMDVisDataViewer/SoVisDataViewer.h"

#include <iostream>
#include <stdio.h>

using namespace std;


ML_START_NAMESPACE


//! Implements code for the runtime type system of the ML
ML_CLASS_SOURCE(METKShowClusteredObjects, ObjMgrClient);


const char* METKShowClusteredObjects::prefRegionTypeStrings[2] = { "Vector region", "Point region" };
const char* METKShowClusteredObjects::sphereModeStrings[2] = { "Grid Sphere", "Full Sphere" };


//----------------------------------------------------------------------------------
//! Constructor / Destructor
//----------------------------------------------------------------------------------
METKShowClusteredObjects::METKShowClusteredObjects() : inherited(0,0,ObjMgrClient::EVENTS_SELECTED)
{
	ML_TRACE_IN("METKShowClusteredObjects::METKShowClusteredObjects()");

	// Suppress calls of handleNotification on field changes.
	handleNotificationOff();

	_outScene = getFieldContainer()->addSoNode("outScene");
	_calc = getFieldContainer()->addNotify("calc");
	_calcMultiple = getFieldContainer()->addNotify("calcMultiple");
	_init = getFieldContainer()->addNotify("init");
	_currentStructure = getFieldContainer()->addString("currentStructure");
	_currentStructures = getFieldContainer()->addString("currentStructures");
	_multipleStructures = getFieldContainer()->addString("multipleStructures");
	_debug = getFieldContainer()->addString("debug");
	(_viewerName = getFieldContainer()->addString("viewerName"))->setStringValue("METKViewer3D");
	_dataPath = getFieldContainer()->addString("dataPath");
	_showField = getFieldContainer()->addInt("showField");
	_sphereMode = getFieldContainer()->addEnum("sphereMode",sphereModeStrings, 2);
	_sphereMode->setEnumValue(SM_FULL);
	/*_camX = getFieldContainer()->addDouble("camX");
	_camY = getFieldContainer()->addDouble("camY");
	_camZ = getFieldContainer()->addDouble("camZ");*/
	_currentCam = getFieldContainer()->addVec3f("currentCam");
	_camRange = getFieldContainer()->addDouble("camRange");
	_visSta = getFieldContainer()->addDouble("visSta");
	_impSta = getFieldContainer()->addDouble("impSta");
	_inspect = getFieldContainer()->addInt("inspect");
	_wVis = getFieldContainer()->addDouble("wVis");
	_wImp = getFieldContainer()->addDouble("wImp");
	_wNum = getFieldContainer()->addDouble("wNum");
	_wEnt = getFieldContainer()->addDouble("wEnt");
	_wDis = getFieldContainer()->addDouble("wDis");
	_wCam = getFieldContainer()->addDouble("wCam");
	_wReg = getFieldContainer()->addDouble("wReg");
	_wVisSta = getFieldContainer()->addDouble("wVisSta");
	_wImpSta = getFieldContainer()->addDouble("wImpSta");
	_wSilhouette = getFieldContainer()->addDouble("wSilhouette");
	_wImageSpaceCenter = getFieldContainer()->addDouble("wImageSpaceCenter");
	
	_prefRegionVector = getFieldContainer()->addVec3f("prefRegionVector");
	_prefRegionVector->setVec3fValue(vec3(0,0,1));
	_prefRegionRange = getFieldContainer()->addDouble("prefRegionRange");
	_prefRegionRange->setDoubleValue(1);
	_prefRegionType = getFieldContainer()->addEnum("prefRegionType", prefRegionTypeStrings, 2);
	_prefRegionType->setEnumValue(PR_VECTOR);	
	_restrictToRegion = getFieldContainer()->addBool("restrictToRegion");
	_restrictToRegion->setBoolValue(false);

	/*_minX = getFieldContainer()->addDouble("minX");
	_minY = getFieldContainer()->addDouble("minY");
	_minZ = getFieldContainer()->addDouble("minZ");*/
	_minDistVec = getFieldContainer()->addVec3f("minDistVec");
	_minRange = getFieldContainer()->addDouble("minRange");
	_calcMin = getFieldContainer()->addNotify("calcMin");
	/*_resX = getFieldContainer()->addDouble("resX");
	_resY = getFieldContainer()->addDouble("resY");
	_resZ = getFieldContainer()->addDouble("resZ");*/
	_result = getFieldContainer()->addVec3f("result");
	_orient = getFieldContainer()->addVec4f("orient");
	_setViewerCamAtTheEnd = getFieldContainer()->addBool("setViewerCamAtTheEnd");
	_writeCamToObjMgr = getFieldContainer()->addNotify("writeCamToObjMgr");

	static const char* debugStates[3] = {"None","High","Low"};
	_debugState = getFieldContainer()->addEnum("debugState", debugStates, 3);
	//_debugState->setStringValue("None");
	kDebug::setDebugLevel(_debugState->getStringValue());

	
	oReceiver = new METKMsgReceiver();
	oReceiver->messageFld->setStringValue("clustering");
	_message = getFieldContainer()->addString("message");
	oReceiver->messageFld->attachField(_message,0);
	_messageData = getFieldContainer()->addString("data");
	oReceiver->getFieldContainer()->getField("data")->attachField(_messageData,1);
	/*_message = (StringField*) getFieldContainer()->addField(oReceiver->getFieldContainer()->getField("message"));
	_messageData = (StringField*) getFieldContainer()->addField(oReceiver->getFieldContainer()->getField("data"));*/
    timerSensor = new SoTimerSensor((SoSensorCB*)METKShowClusteredObjects::timerEvent, this);
    timerSensor->setInterval(SbTime(1.0/1000.0));	
    timerSensor->unschedule();
	
	myObjMgr = new ObjMgrCommunicator();
	Cam = new kCamera();

	m_soViewer = new SoVisDataViewer();
	_outScene->setSoNodeValue(m_soViewer);

	clearAcceptedObjectIDs();
	clearAcceptedInfoLayerNames();

	// Reactivate calls of handleNotification on field changes.
	handleNotificationOn();
}


METKShowClusteredObjects::~METKShowClusteredObjects() {}


void METKShowClusteredObjects::activateAttachments(){
	clearAcceptedObjectIDs();
	clearAcceptedInfoLayerNames();
	addAcceptedObjectID("*");
	addAcceptedInfoLayerName(LAY_APPEARANCE);
	addAcceptedInfoLayerName(LAY_GLOBALEVENTS);

	getFieldContainer()->getField("inObjectContainer")->attachField(oReceiver->getFieldContainer()->getField("inObjectContainer"),true);
	getFieldContainer()->getField("inObjectContainer")->attachField(myObjMgr->getFieldContainer()->getField("inObjectContainer"),true);

	kDebug::setDebugLevel(_debugState->getStringValue());

	// Don't forget to call the super class functionality, it enables field
	// notifications for your module again.
	// SUPER_CLASS is the class you derive from (usually BaseOp).
	ObjMgrClient::activateAttachments();
}

std::string METKShowClusteredObjects::calcPositions(std::vector<std::string> structs,int depth){
	m_calcVis.clearStack(1);
	m_calcVis.clearStack(2);
	m_calcVis.clearStack(3);
	m_calcVis.getStackField(3)->fillWith(1.0);
	m_calcVis.clearStack(4);
	float th=_impSta->getDoubleValue();
	std::vector<std::string> nv;
	std::vector<std::string> actobjs;
	for(size_t i=0;i<structs.size();i++){									
		if (!(structs[i].empty())){
			_currentStructure->setStringValue(structs[i]);
			m_calcVis.setFocusObject(structs[i]);
			m_calcVis.calc();
			m_calcVis.copyFieldToStack(SUM_FIELD,2);
			m_calcVis.getStackField(2)->normalize();
			//m_calcVis.multiplyStackField(2,kBasics::StringToDouble(splittedSinlgeObj[1])); //weight current object sum_field
			if(structs.size()>1  && i>0){
				m_calcVis.getStackField(2)->binarize(th);
				m_calcVis.multiplyStackFields(2,3,4);
				if(m_calcVis.getStackField(4)->getMaxValue()<1.0){
					nv.push_back(structs[i]);
				}else{
					m_calcVis.copyFieldToStack(4,3);
					m_calcVis.addStackFields(2,1,1); //Copy current object sum_field -> 1
					actobjs.push_back(structs[i]);
					kDebug::Debug("Calc viewpoint for structure: "+structs[i],kDebug::DL_NONE);
				}
			}else{
				m_calcVis.addStackFields(2,1,1); //Copy current object sum_field -> 1			
				actobjs.push_back(structs[i]);
				kDebug::Debug("Calc viewpoint for structure: "+structs[i],kDebug::DL_NONE);
			}
		}
	}
	m_calcVis.getStackField(1)->normalize();
	m_soViewer->touch();
	//m_soViewer->setDataField(m_calcVis.getField(STA_VIS_FIELD));
	setCamPosition(1,true);
	//updateObjectMgr();		
	//timerSensor->schedule();
	vec3 res=_result->getVec3fValue();
	vec4 orient=_orient->getVec4fValue();
	double height=1.23;
	/*myObjMgr->setObjAttribute("CameraSolver","Camera","Position",new vec3(_result->getVec3fValue()),omINFOTYPE_VEC3,true,false);
	myObjMgr->setObjAttribute("CameraSolver","Camera","Orientation",new vec4(_orient->getVec4fValue()),omINFOTYPE_VEC4,true,false);
	myObjMgr->setObjAttribute("CameraSolver","Camera","Height",new double(1.23),omINFOTYPE_DOUBLE ,true,false);
	myObjMgr->setObjAttribute("CameraSolver","Calculation","Success",new bool(1),omINFOTYPE_BOOL ,true,false);*/
	std::ostringstream os;
	os << res[0] << ";" << res[1] << ";" << res[2] << " ";
	os << orient[0] << ";" << orient[1] << ";" << orient[2] << ";" << orient[3] << " " << height << " ";
	std::string delim="";
	for(int i=0;i<actobjs.size();i++){
		os << delim << actobjs[i];
		delim=";";
	}
	omObjectContainer* oc = getObjContainer();
	if (oc){
		vec3 actualColor(max(0.0,1.0-depth/12.0),min(1.0,0.0+depth/5.0),max(0.3,1.0-depth/9.0));
		omObjectContainer::iterator iterObj;
		std::string objID;
		for (iterObj = oc->begin(); iterObj != oc->end(); iterObj++){
			omObject &obj = iterObj->second;
			myObjMgr->getObjAttributeString(obj.getID(),LAY_GLOBAL,INF_ID,objID);
			if (obj.isValid() && std::find(structs.begin(),structs.end(),objID)!=structs.end()){// && obj.getAttribute(LAY_APPEARANCE, obj.hasAttribute(LAY_APPEARANCE, INF_VISIBLE)){
				myObjMgr->setObjAttribute( obj.getID(), LAY_APPEARANCE, INF_SILHOUETTE, new bool(true), omINFOTYPE_BOOL,true,false);
				myObjMgr->setObjAttribute( obj.getID(), LAY_APPEARANCE, INF_SILHOUETTECOLOR, new vec3(actualColor), omINFOTYPE_VEC3,true,false);
				myObjMgr->setObjAttribute( obj.getID(), LAY_APPEARANCE, INF_SILHOUETTEWIDTH, new double(5.0), omINFOTYPE_DOUBLE,true,false);
			}
		}
	}
	if(!(nv.empty())){
		os << "#" << calcPositions(nv,depth+1);
	}
	m_soViewer->touch();
	return os.str();
}

//----------------------------------------------------------------------------------
//! Handle field changes of the field \c field.
//----------------------------------------------------------------------------------
void METKShowClusteredObjects::handleNotification(Field *field){
	if (field == _init){
		init();
	}else if (field == _calc){
		kDebug::Debug("Calc viewpoint for structures ("+_currentStructures->getStringValue()+")",kDebug::DL_NONE);
		std::vector<std::string> structs;
		kBasics::split(_currentStructures->getStringValue(),' ',-1,&structs);
		omObjectContainer* oc = getObjContainer();
		if (oc){
			omObjectContainer::iterator iterObj;
			for (iterObj = oc->begin(); iterObj != oc->end(); iterObj++){
				omObject &obj = iterObj->second;
				if (obj.isValid()){// && obj.getAttribute(LAY_APPEARANCE, obj.hasAttribute(LAY_APPEARANCE, INF_VISIBLE)){
					myObjMgr->setObjAttribute( obj.getID(), LAY_APPEARANCE, INF_SILHOUETTE, new bool(false), omINFOTYPE_BOOL,true,false);
					myObjMgr->setObjAttribute( obj.getID(), LAY_APPEARANCE, INF_SILHOUETTECOLOR, new vec3(0,0,0), omINFOTYPE_VEC3,true,false);
					myObjMgr->setObjAttribute( obj.getID(), LAY_APPEARANCE, INF_SILHOUETTEWIDTH, new double(0.0), omINFOTYPE_DOUBLE,true,false);
				}
			}
		}
		std::string erg=calcPositions(structs);
		myObjMgr->setObjAttribute("CameraSolver","Camera","ClusteringResult",&erg,omINFOTYPE_STRING,true,false);
		myObjMgr->setObjAttribute("CameraSolver","Calculation","Success",new bool(1),omINFOTYPE_BOOL ,true,false);
		sendNotification();
		timerSensor->schedule();
	}

	else if (field == _calcMultiple)
	{		
		kDebug::Debug("Calc viewpoint for multiple objects",kDebug::DL_HIGH);
		calcMultiple();
	}

	else if (field == _showField || field == _sphereMode)
	{
		m_soViewer->setSphereMode(_sphereMode->getEnumValue()+1); //Die ENUMs gehen 0..1 der sphereMode aber 1..2
		if (_showField->getIntValue()==0) m_soViewer->setDataField(m_calcVis.getField(VIS_FIELD));
		if (_showField->getIntValue()==1) m_soViewer->setDataField(m_calcVis.getField(STA_VIS_FIELD));
		if (_showField->getIntValue()==2) m_soViewer->setDataField(m_calcVis.getField(IMP_FIELD));
		if (_showField->getIntValue()==3) m_soViewer->setDataField(m_calcVis.getField(STA_IMP_FIELD));
		if (_showField->getIntValue()==4) m_soViewer->setDataField(m_calcVis.getField(NUM_FIELD));
		if (_showField->getIntValue()==5) m_soViewer->setDataField(m_calcVis.getField(ENT_FIELD));
		if (_showField->getIntValue()==6) m_soViewer->setDataField(m_calcVis.getField(DIS_FIELD));
		if (_showField->getIntValue()==7) m_soViewer->setDataField(m_calcVis.getField(CAM_FIELD));
		if (_showField->getIntValue()==8) m_soViewer->setDataField(m_calcVis.getField(REG_FIELD));		
		if (_showField->getIntValue()==9) m_soViewer->setDataField(m_calcVis.getField(SIL_FIELD));
		if (_showField->getIntValue()==10) m_soViewer->setDataField(m_calcVis.getField(CENTER_FIELD));
		if (_showField->getIntValue()==11) m_soViewer->setDataField(m_calcVis.getField(SUM_FIELD));
		
		if (_showField->getIntValue()==12) m_soViewer->setDataField(m_calcVis.getStackField(1));
		if (_showField->getIntValue()==13) m_soViewer->setDataField(m_calcVis.getStackField(2));
		if (_showField->getIntValue()==14) m_soViewer->setDataField(m_calcVis.getStackField(3));
		if (_showField->getIntValue()==15) m_soViewer->setDataField(m_calcVis.getStackField(4));
		if (_showField->getIntValue()==16) m_soViewer->setDataField(m_calcVis.getStackField(5));
		m_soViewer->touch();
	}

	else if (field == _dataPath){
		if (_dataPath->getStringValue() != ""){
			float fX,fY,fZ,fR;
			int iDiv;			
			m_calcVis.setData(_dataPath->getStringValue());
			//m_calcVis.logResults();
			if (m_calcVis.hasData()){
				m_calcVis.getSphereValues(fX,fY,fZ,fR,iDiv);
				m_soViewer->createSphere(fX,fY,fZ,fR,iDiv);
				m_soViewer->setSphereMode(2);
				m_vStructures = m_calcVis.getStructureNames ();
				for (int i=0; i<m_vStructures.size(); i++){
					m_calcVis.setImportance((*m_vStructures[i]),getImportance(m_vStructures[i]));
				}
				m_calcVis.setStackSize(10);
				m_calcVis.addVectorRegionToStackField(0,0,0,1,1); //eigentlich �berfl�ssig, da es jetzt �ber externe Felder eingestellt werden kann
				m_calcVis.setStackFieldAsRegionField(0);
			}
			else { std::cout << "!!!!! m_calcVis has NO data" << std::endl; }
		}
	}

	else if (field == _currentStructure)
	{
		m_calcVis.setFocusObject(_currentStructure->getStringValue());
	}

	else if ( field == _currentCam )
	{
		m_calcVis.setCamPos ( _currentCam->getVec3fValue()[0], _currentCam->getVec3fValue()[1], _currentCam->getVec3fValue()[2] );
	}

	else if ( field == _camRange ) { m_calcVis.setCamRange ( _camRange->getDoubleValue() ); }
	else if ( field == _visSta ) { m_calcVis.setVisStability(_visSta->getDoubleValue() ); }
	else if ( field == _impSta ) { m_calcVis.setImpStability(_impSta->getDoubleValue() ); }
	else if ( field == _inspect ) { m_soViewer->inspectPoint(_inspect->getIntValue()); }
	else if ( field == _wVis ) { m_calcVis.setWeight (VIS_FIELD, _wVis->getDoubleValue()); }
	else if ( field == _wImp ) { m_calcVis.setWeight (IMP_FIELD, _wImp->getDoubleValue()); }
	else if ( field == _wNum ) { m_calcVis.setWeight (NUM_FIELD, _wNum->getDoubleValue()); }
	else if ( field == _wEnt ) { m_calcVis.setWeight (ENT_FIELD, _wEnt->getDoubleValue()); }
	else if ( field == _wDis ) { m_calcVis.setWeight (DIS_FIELD, _wDis->getDoubleValue()); }
	else if ( field == _wCam ) { m_calcVis.setWeight (CAM_FIELD, _wCam->getDoubleValue()); }
	else if ( field == _wReg ) { m_calcVis.setWeight (REG_FIELD, _wReg->getDoubleValue()); }
	else if ( field == _wVisSta ) { m_calcVis.setWeight (STA_VIS_FIELD, _wVisSta->getDoubleValue()); }
	else if ( field == _wImpSta ) { m_calcVis.setWeight (STA_IMP_FIELD, _wImpSta->getDoubleValue()); }
	else if ( field == _wSilhouette ) { m_calcVis.setWeight (SIL_FIELD, _wSilhouette->getDoubleValue()); }
	else if ( field == _wImageSpaceCenter ) { m_calcVis.setWeight (CENTER_FIELD, _wImageSpaceCenter->getDoubleValue()); }

	else if ( field == _prefRegionType )
	{
		if (_prefRegionType->getEnumValue()==PR_VECTOR)
			m_calcVis.setPrefRegionType(m_calcVis.PR_VECTOR);
		else
			m_calcVis.setPrefRegionType(m_calcVis.PR_POINT);
	}

	else if ( field == _prefRegionVector )
	{		
		m_calcVis.setPrefRegionVector(_prefRegionVector->getVec3fValue()[0], _prefRegionVector->getVec3fValue()[1], _prefRegionVector->getVec3fValue()[2]);
	}

	else if ( field == _prefRegionRange )
	{
		m_calcVis.setPrefRegionRange(_prefRegionRange->getDoubleValue());
	}

	else if (field == _restrictToRegion )
	{
		m_calcVis.setRestrictToRegion(_restrictToRegion->getBoolValue());
	}

	else if ( field == _calcMin)
	{
		kDebug::Debug("Calc viewpoint for minimal distance",kDebug::DL_HIGH);
		calcMultiple(false); //Only the two objects for minimal distcance must be in the _multipleStructures field; the sum field is calculated but the viewer cam is not set
		//As result the sum field of the two structures is located in StackField 1
		
		//m_calcVis.copyFieldToStack ( SUM_FIELD, 1 );
		//m_calcVis.setFocusObject(_secondStructure->getStringValue());
		//m_calcVis.calc();
		//std::cout << "calc second" << std::endl;
		//m_calcVis.copyFieldToStack ( SUM_FIELD, 2 );
		std::cout << "create region" << std::endl;
		m_calcVis.clearStack(2);
		m_calcVis.addVectorRegionToStackField ( 2, _minDistVec->getVec3fValue()[0], _minDistVec->getVec3fValue()[1], _minDistVec->getVec3fValue()[2],  _minRange->getDoubleValue() );

		//m_calcVis.addStackFields ( 1, 2, 4 );
		//std::cout << "normalize" << std::endl;
		//m_calcVis.getStackField(4)->normalize();

		m_calcVis.clearStack(3);
		m_calcVis.multiplyStackFields ( 1, 2, 3 ); //Multiply the region with the sum field and store result in stack 3		
		std::cout << "final result is in stack 3 (Field 15)" << std::endl;
		m_soViewer->touch();
		setCamPosition(3,true);
		updateObjectMgr();		
		timerSensor->schedule();
	}

	else if (field == _messageData)
	{
		std::cout << "_messageData=" << _messageData->getStringValue() << std::endl;
		vector<string> params;
		kBasics::split(_messageData->getStringValue(),' ',-1,&params);
		std::list<kBasics::optionstruct> options;
		options.push_back(kBasics::optionstruct("structures",-1,'s'));
		options.push_back(kBasics::optionstruct("objects",-1,'o'));
		options.push_back(kBasics::optionstruct("weightvis",1,'v'));
		options.push_back(kBasics::optionstruct("weightreg",1,'r'));
		options.push_back(kBasics::optionstruct("weightcam",1,'c'));
		options.push_back(kBasics::optionstruct("weightent",1,'e'));
		options.push_back(kBasics::optionstruct("weightnum",1,'n'));
		options.push_back(kBasics::optionstruct("weightimp",1,'i'));
		options.push_back(kBasics::optionstruct("weightsta",1,'a'));
		options.push_back(kBasics::optionstruct("thressta",1,'t'));
		options.push_back(kBasics::optionstruct("weightsta2",1,'w'));
		options.push_back(kBasics::optionstruct("thressta2",1,'m'));
		options.push_back(kBasics::optionstruct("weightviewplanedist",1,'p'));
		options.push_back(kBasics::optionstruct("weightsilhouette",1,'h'));
		options.push_back(kBasics::optionstruct("weightimagecenter",1,'g'));
		std::string value;
		std::map<char,std::string> values;
		char c;
		while(-1!=(c=kBasics::getOptions(params,options,value))){
			switch(c){
				case 's':	case 'o':	case 'v':
				case 'r':	case 'c':	case 'e':
				case 'n':	case 'i':	case 'a':
				case 't':	case 'w':	case 'm':
				case 'p':	case 'h':
				case 'g':values[c]=value;break;
			}
		}
		if (!myObjMgr->getObjAttributeString(O_CASEOBJECT, LAY_CASEOBJECT_CASE, INF_CASEOBJECT_CASE_DIRECTORY, path))
		{
			cout<<"FEHLER: Keine Falldaten vorhanden!"<<endl;
			_debug->setStringValue("error2");
			myObjMgr->setObjAttribute("CameraSolver","Calculation","Success",new bool(0),omINFOTYPE_BOOL ,true,false);
			sendNotification();		
			timerSensor->schedule();
		}
		else	
		{
			if (values.size()!=options.size()) 
			{
				cout << "FEHLER: MessageString Format falsch!" << values.size() << "!=" << options.size() << std::endl;
				_debug->setStringValue("error1");
				myObjMgr->setObjAttribute("CameraSolver","Calculation","Success",new bool(0),omINFOTYPE_BOOL ,true,false);
				sendNotification();		
				timerSensor->schedule();
			}
			else
			{				
				SbVec3f tempPos;
				SbVec4f tempOrientVec4;
				bool result;
				result = myObjMgr->getObjAttributeVec3f(_viewerName->getStringValue(), LAY_VIEWER_CAMERA, INF_VIEWER_CAMERA_POSITION, tempPos);
				//result = result && myObjMgr->getObjAttributeVec4f(_viewerName->getStringValue(), LAY_VIEWER_CAMERA, INF_VIEWER_CAMERA_ORIENTATION, tempOrientVec4);
				
				float x,y,z;
				tempPos.getValue(x,y,z);
				m_calcVis.setCamPos( x, y, z );

				if (!result)
				{
					std::cout << "No Camera position found in METK for " << _viewerName->getStringValue() << std::endl;
					timerSensor->schedule();
				}else{
					_currentStructures->setStringValue(values['o']);
					_wVis->setDoubleValue(kBasics::StringToDouble(values['v']));
					_wReg->setDoubleValue(kBasics::StringToDouble(values['r']));
					_wCam->setDoubleValue(kBasics::StringToDouble(values['c']));					
					_wEnt->setDoubleValue(kBasics::StringToDouble(values['e']));
					_wNum->setDoubleValue(kBasics::StringToDouble(values['n']));
					_wImp->setDoubleValue(kBasics::StringToDouble(values['i']));
					_wVisSta->setDoubleValue(kBasics::StringToDouble(values['a']));
					_visSta->setDoubleValue(kBasics::StringToDouble(values['t']));
					_wImpSta->setDoubleValue(kBasics::StringToDouble(values['w']));
					_impSta->setDoubleValue(kBasics::StringToDouble(values['m']));					
					_wDis->setDoubleValue(kBasics::StringToDouble(values['p']));
					_wSilhouette->setDoubleValue(kBasics::StringToDouble(values['h']));
					_wImageSpaceCenter->setDoubleValue(kBasics::StringToDouble(values['g']));
					_calc->notify();
				}

				//New
				//m_soViewer->touch();
				//setCamPosition(SUM_FIELD,false);
			}
		}
	}

	else if (field == getFieldContainer()->getField("inObjectContainer"))
	{
		if (getFieldContainer()->getField("inObjectContainer")->getDestinationField (0) == NULL)
		{
			//Verbinden aller inObjectContainer innerhalb des Moduls!!!
			//Diese Zeile hat mich fast 2 Tage und sehr viele Nerven gekostet ;-) (aus METKObjContainer geklaut)			
			getFieldContainer()->getField("inObjectContainer")->attachField(myObjMgr->getFieldContainer()->getField("inObjectContainer"),1);
			getFieldContainer()->getField("inObjectContainer")->attachField(oReceiver->getFieldContainer()->getField("inObjectContainer"),1);
		}	
	}

	else if (field == _debugState)
	{		
		std::cout << "setDebugLevel=" << _debugState->getStringValue() << std::endl;
		kDebug::setDebugLevel(_debugState->getStringValue());
	}

	else if (field == _writeCamToObjMgr)
	{
		writeCamToObjMgr();
		sendNotification();
	}

	inherited::handleNotification(field);
}



//React on visible-Events from METKManager
void METKShowClusteredObjects::handleObjMgrNotification()
{	
	omEventContainer myEventList = getEventContainer();
		
	//Durchiterieren der EventList
	omEventContainer::const_iterator iter;		
	for ( iter = myEventList.begin();iter!=myEventList.end(); iter++)
	{
		ObjMgrEvent myEvent = (*iter);
		//std::cout << myEvent.objectID << "-" << myEvent.layerID << "-" << myEvent.infoID << ": " << string(myEvent.newValue) << std::endl;
		if (myEvent.layerID == LAY_APPEARANCE)
		{
			if (myEvent.infoID == INF_VISIBLE)
			{
				if (myEvent.newValue.getStringValue() == "TRUE")
				{					
					//std::cout << "ENABLE" << myEvent.objectID << std::endl;
					m_calcVis.setStatus(myEvent.objectID,true);
				}
				else
				{
					//std::cout << "DISABLE" << myEvent.objectID << std::endl;
					m_calcVis.setStatus(myEvent.objectID,false);
				}
			}		
		}				

		else if (myEvent.objectID == OBJ_COMMUNICATION && myEvent.layerID == LAY_GLOBALEVENTS && string(myEvent.newValue) == MSG_LOADED)
		{			
			std::cout << "INIT " << std::endl;
			init();
		}		
	}
	clearEventContainer();
}



void METKShowClusteredObjects::setCamPosition(const int stackOrFieldNr, bool isStackNr){
	float fX,fY,fZ;
	if (!isStackNr)
		m_calcVis.getFieldMaxPos(stackOrFieldNr,fX,fY,fZ);
	else
		m_calcVis.getStackMaxPos(stackOrFieldNr,fX,fY,fZ);
    
	/*_resX->setDoubleValue(fX);
	_resY->setDoubleValue(fY);
	_resZ->setDoubleValue(fZ);*/
	std::cout << "_result = " << fX << "," << fY << "," << fZ << std::endl;
	vec3 v(fX,fY,fZ);
	
	_result->setVec3fValue(vec3(fX,fY,fZ));
	
	Cam->setNormPlump(SbVec3f(0.0,0.0,1.0));
	Cam->setUpVecAngle(0.0);
	float fMX,fMY,fMZ,fbla;
	int ibla;
	m_calcVis.getSphereValues(fMX,fMY,fMZ,fbla,ibla);
	Cam->setCamPosition(SbVec3f(fX,fY,fZ),SbVec3f(fMX,fMY,fMZ));
	Cam->setHeight(3.0);
	SbRotation rot=Cam->getOrientation();
	SbVec3f axis;
	float angle,o1,o2,o3;
	rot.getValue(axis,angle);
	axis.getValue(o1,o2,o3);
	_orient->setVec4fValue(vec4(o1,o2,o3,angle));
}


void METKShowClusteredObjects::timerEvent(void* data, SoDataSensor* a){
	METKShowClusteredObjects* caller = (METKShowClusteredObjects*)data;//->render(a);    
    caller->timerSensor->unschedule();    
    caller->oReceiver->doneFld->notify();
}


void METKShowClusteredObjects::updateObjectMgr(){
	kDebug::Debug("Update Camera data in CameraSolver object",kDebug::DL_HIGH);
	myObjMgr->setObjAttribute("CameraSolver","Camera","Position",new vec3(_result->getVec3fValue()),omINFOTYPE_VEC3,true,false);
	myObjMgr->setObjAttribute("CameraSolver","Camera","Orientation",new vec4(_orient->getVec4fValue()),omINFOTYPE_VEC4,true,false);
	myObjMgr->setObjAttribute("CameraSolver","Camera","Height",new double(1.23),omINFOTYPE_DOUBLE ,true,false);
	myObjMgr->setObjAttribute("CameraSolver","Calculation","Success",new bool(1),omINFOTYPE_BOOL ,true,false);
	if (_setViewerCamAtTheEnd->getBoolValue())
	{		
		//writeCamToObjMgr();
	}
	sendNotification();
}



void METKShowClusteredObjects::writeCamToObjMgr(){
	kDebug::Debug("Write camera data to "+_viewerName->getStringValue(),kDebug::DL_HIGH);
	myObjMgr->setObjAttribute(_viewerName->getStringValue(),LAY_VIEWER_CAMERA,INF_VIEWER_CAMERA_POSITION,new vec3(_result->getVec3fValue()),omINFOTYPE_VEC3,true,false);
	myObjMgr->setObjAttribute(_viewerName->getStringValue(),LAY_VIEWER_CAMERA,INF_VIEWER_CAMERA_ORIENTATION,new vec4(_orient->getVec4fValue()),omINFOTYPE_VEC4,true,false);
	myObjMgr->setObjAttribute(_viewerName->getStringValue(),LAY_VIEWER_CAMERA,INF_VIEWER_CAMERA_NEWDATA,new omMessage("ping"),omINFOTYPE_MESSAGE,true,false);
}


void METKShowClusteredObjects::enableVisibleObjects(){
	omObjectContainer* oc = getObjContainer();
	if (oc)
	{
		omObjectContainer::iterator iterObj;
		
		for (iterObj = oc->begin(); iterObj != oc->end(); iterObj++)
		{
			omObject &obj = iterObj->second;
			if (obj.isValid() && obj.hasAttribute(LAY_APPEARANCE, INF_VISIBLE))
			{				
				bool visible;
				myObjMgr->getObjAttributeBool(obj.getID(), LAY_APPEARANCE, INF_VISIBLE, visible);				
				if (visible)
				{
					//std::cout << "ENABLE" << obj.getID() << std::endl;
					m_calcVis.setStatus(obj.getID(),true);
				}               
			}
		}
	}
}


const float METKShowClusteredObjects::getImportance(const std::string* name){
	omObjectContainer* oc = getObjContainer();
	if (oc) {
		if ((*oc).exists(*name)) {
         if ((*oc)[*name].hasAttribute(LAY_DESCRIPTION, INF_IMPORTANCE)) {
            return (*oc)[*name][LAY_DESCRIPTION][INF_IMPORTANCE].get_double();
         } else {
            if ((*oc)[*name].hasAttribute(LAY_APPEARANCE, INF_TRANSPARENCY)) {
               return 1.0 - (*oc)[*name][LAY_APPEARANCE][INF_TRANSPARENCY].get_double();
            }
         }
      }
   }
   return 0;
}


void METKShowClusteredObjects::init(){
	kDebug::Debug("init METKShowClusteredObjects",kDebug::DL_HIGH);
	string sPath;	
	if (!myObjMgr->getObjAttributeString(O_CASEOBJECT, LAY_CASEOBJECT_CASE, INF_CASEOBJECT_CASE_DIRECTORY, sPath))
	{
		cout<<"FEHLER: Keine Falldaten vorhanden!"<<endl;
	}
	else
	{
		_dataPath->setStringValue(sPath);
		enableVisibleObjects();
	}
}


void METKShowClusteredObjects::calcMultiple(bool setCamera){
	m_calcVis.clearStack(1);
	m_calcVis.clearStack(2);
	//Splitting the Multiple object vector and adding each object to final map
	//example: Lymphnode_1,0.7;Lymphnode_2,0.5;
	std::vector<string> splittedObjects;
	std::vector<string> splittedSinlgeObj;
	kBasics::split(_multipleStructures->getStringValue(), ';',100,&splittedObjects);
	for (unsigned int i=0;i<splittedObjects.size();i++)
	{									
		if (splittedObjects[i]!="")
		{
			kBasics::split(splittedObjects[i],',',2,&splittedSinlgeObj);
			if (splittedSinlgeObj.size()==2)
			{
				//std::cout << "Add " << splittedSinlgeObj[0] << std::endl;
				m_calcVis.setFocusObject(splittedSinlgeObj[0]);
				m_calcVis.calc();
				m_calcVis.copyFieldToStack(SUM_FIELD,2);
				m_calcVis.multiplyStackField(2,kBasics::StringToDouble(splittedSinlgeObj[1])); //weight current object sum_field
				m_calcVis.addStackFields(2,1,1); //Copy current object sum_field -> 1				
			}
		}
	}
	m_calcVis.getStackField(1)->normalize();	
	m_soViewer->touch();
	if (setCamera)
	{
		setCamPosition(1,true);
		updateObjectMgr();		
		timerSensor->schedule();
	}
}

ML_END_NAMESPACE
