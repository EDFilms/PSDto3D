//----------------------------------------------------------------------------------------------
//
//  @file MayaStub.h
//  @author Michaelson Britt
//  @date 01-Feb-2020
//
//  @section DESCRIPTION
//  Provides stub implementations of all Maya dependencies,
//  used for porting from Maya to FBX
//
//----------------------------------------------------------------------------------------------

#ifndef MAYASTUB_H
#define MAYASTUB_H

typedef unsigned long ULONG; // TODO: Remove this; with older Maya vesions, and with Visual Studio 2017 toolset, Qt headers depend on windows.h or require this
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QObject>

#ifndef NULL
#define NULL 0L
#endif

class MString
{
public:
	MString( const char* charString ) { charString; }
	MString( const wchar_t* charString ) { charString; }
	MString( const MString& other ) { other; }
	MString     operator + (const MString& other ) const { other; return MString(""); }
	MString     operator + (const char * other ) const { other; return MString(""); }
	MString     operator + ( double value ) const { value; return MString(""); } 
	friend MString operator+(const char * other, const MString& self ) { other; self; return MString("");}
	const char* 	asUTF8() const {return nullptr;}
	const char*		asChar() const {return nullptr;}
};
class MStringArray
{
public:
	MString		operator[]( unsigned int index ) const { index; return MString(""); }
	unsigned int	length() const {return 0;}
};

class MQtUtil
{
public:
	static QWidget*	mainWindow() { return nullptr; }
	static MString	toMString(const QString& qstr) { qstr; return MString(""); }
	static QString	toQString(const MString& mstr) { mstr; return QString(""); }
};

class MStatus
{
public:
	//! Available Status Codes
	enum MStatusCode
	{
		kSuccess = 0,
		kFailure,
		kInsufficientMemory,
		kInvalidParameter,
	};
						MStatus() {}
						MStatus( MStatusCode ) {}
						MStatus( const MStatus& ) {}
						bool				operator==( const MStatusCode rhs ) const
						{ rhs; return true; }
						MString				errorString() const {return MString("");}
	operator			bool() const {return true;}
};
typedef MStatus MS;

class MSpace
{
public:
	enum Space {
		kInvalid = 0,				//!< Invalid value.
	 	kTransform = 1,				//!< Transform matrix (relative) space.
	};
};

class MVector
{
public:
	inline MVector(double xx, double yy, double zz)
 		: x(xx), y(yy), z(zz)
	{ }
	double x;
	double y;
	double z;
};

class MFloatPoint
{
public:
	float				x,y,z,w;
};

class MIntArray
{
public:
	MStatus			setLength( unsigned int length ) { length; return MStatus::kSuccess; }
	int&	 		operator[]( unsigned int index ) { index; return stub; }
	int stub;
};

class MFloatArray
{
public:
	MStatus			setLength( unsigned int length ) { length; return MStatus::kSuccess; }
	float &	 	    operator[]( unsigned int index ) { index; return stub; }
	float stub;
};

class MFloatPointArray
{
public:
	MStatus			setLength( unsigned int length ) { length; return MStatus::kSuccess; }
	MFloatPoint&	    operator[]( unsigned int index ) { index; return stub; }
	MFloatPoint stub;
};

class MTypeId
{
public:
};

class MFn
{
public:
	enum Type {
		kInvalid = 0,						//!< \nop
		kTransform=110,
		kShape=248,
		kLambert=367,
		kSet=468
	};
};

class MObject
{
public:
	bool			hasFn( MFn::Type fs ) const { fs; return false; }
	static MObject  kNullObj;
};

class MPlug;
class MPlugArray;

class MPlug
{
public:
	MObject 	node( MStatus* ReturnStatus = NULL ) const { ReturnStatus; return MObject(); }
	bool		connectedTo( MPlugArray & array, bool asDst, bool asSrc,
							 MStatus* ReturnStatus = NULL ) const { array; asDst; asSrc; ReturnStatus; return false;}
	MStatus     setValue( const MString& ) {return MStatus::kSuccess;}
};

class MPlugArray
{
public:
	const MPlug&	operator[]( unsigned int index ) const { index; return stub; }
	MPlug stub;
};

class MArgList
{
};

class MSyntax
{
public:
};
typedef MSyntax	 (*MCreateSyntaxFunction)(); 
typedef void *   (*MCreatorFunction)();

class MSelectionList
{
public:
	bool			isEmpty	( MStatus * ReturnStatus = NULL ) const { ReturnStatus; return true; }
	unsigned int	length	( MStatus * ReturnStatus = NULL ) const { ReturnStatus; return 0; }
	MStatus		    getDependNode ( unsigned int index, MObject &depNode ) const
					{ index; depNode; return MStatus::kSuccess; }
	MStatus         add     ( const MString & matchString,
							  const bool searchChildNamespacesToo = false )
					{ matchString; searchChildNamespacesToo; return MStatus::kSuccess;}
};

class MPxCommand
{
public:
	virtual MStatus   	doIt( const MArgList& args ) { args; return MStatus::kSuccess;}
};

class MDGModifier
{
public:
	MStatus     deleteNode( const MObject & node ) { node; return MStatus::kSuccess; }
	MStatus     renameNode( const MObject & node, const MString &newName ) { node; newName; return MStatus::kSuccess;}
	MStatus		commandToExecute	  ( const MString& command ) { command; return MStatus::kSuccess; }
	MStatus		doIt() {return MStatus::kSuccess;}
};

class MDagModifier : public MDGModifier
{
public:
    MObject     createNode( const MString &type,
                            const MObject & parent =  MObject::kNullObj,
                            MStatus* ReturnStatus = NULL )
				{ type; parent; ReturnStatus; return MObject();}
};

class MItDependencyNodes
{
public:
	MItDependencyNodes( MFn::Type filter = MFn::kInvalid,
						MStatus * ReturnStatus = NULL ) { filter; ReturnStatus; }
	MStatus		next() {return MStatus::kSuccess;}
	bool		isDone( MStatus * ReturnStatus = NULL ) const { ReturnStatus; return true; }
	MObject		item( MStatus * ReturnStatus = NULL ) const { ReturnStatus; return MObject(); }
};

class MFnDependencyNode
{
public:
	MFnDependencyNode() {}
	MFnDependencyNode( MObject & object, MStatus * ret = NULL ) { object; ret; }
	MTypeId         typeId( MStatus* ReturnStatus = NULL ) const { ReturnStatus; return MTypeId(); }
	MString         typeName( MStatus* ReturnStatus = NULL ) const { ReturnStatus; return MString(""); }
	MString			name( MStatus * ReturnStatus = NULL ) const { ReturnStatus; return MString(""); }
	MString			absoluteName( MStatus * ReturnStatus = NULL ) const { ReturnStatus; return MString(""); }
	MString			setName( const MString &name,
							 bool createNamespace = false,
							 MStatus * ReturnStatus = NULL )
					{  name; createNamespace; ReturnStatus; return MString("");}
	MPlug			findPlug( const MString & attrName, bool wantNetworkedPlug,
							  MStatus* ReturnStatus=NULL) const
					{  attrName; wantNetworkedPlug; ReturnStatus; return MPlug(); }
	MObject		item( MStatus * ReturnStatus = NULL ) const { ReturnStatus; return MObject(); }
};

class MFnDagNode : public MFnDependencyNode
{
public:
	MFnDagNode() {}
	MFnDagNode( MObject & object, MStatus * ret = NULL ) { object; ret; }
	unsigned int	parentCount( MStatus * ReturnStatus = NULL ) const { ReturnStatus; return 0; }
	MObject 		parent( unsigned int i,
							MStatus * ReturnStatus = NULL ) const { i; ReturnStatus; return MObject(); }
	MString         partialPathName(MStatus *ReturnStatus = NULL) const { ReturnStatus; return MString(""); }
};

class MFnSet : public MFnDependencyNode
{
public:
	MFnSet() {}
	MFnSet( MObject & object, MStatus * ret = NULL ) { object; ret; }
	enum Restriction {
		kNone,			//!< \nop
		kVerticesOnly,		//!< \nop
		kEdgesOnly,		//!< \nop
		kFacetsOnly,		//!< \nop
		kEditPointsOnly,	//!< \nop
		kRenderableOnly		//!< \nop
	};
	MStatus     addMember( const MObject &obj ) { obj; return MStatus(); }
	MObject     create( const MSelectionList & members,
						         Restriction restriction = kNone,
						         bool isLayer = false,
						         MStatus * ReturnStatus = NULL )
				{ members; restriction; isLayer; ReturnStatus; return MObject(); }
	bool        isMember( const MObject &object,
						  MStatus * ReturnStatus = NULL ) const
				{ object; ReturnStatus; return false; }
	Restriction restriction( MStatus * ReturnStatus = NULL ) const
				{ ReturnStatus; return Restriction(); }
};

class MFnBase
{
public:
};
class MFnPlugin : public MFnBase
{
public:
					MFnPlugin( MObject& object,
							   const char* vendor = "Unknown",
							   const char* version = "Unknown",
							   const char* requiredApiVersion = "Any",
							   MStatus* ReturnStatus = 0L )
					{ object; vendor; version; requiredApiVersion; ReturnStatus; }
	MStatus			registerCommand(const MString& commandName,
									MCreatorFunction creatorFunction,
									MCreateSyntaxFunction createSyntaxFunction = NULL)
					{ commandName; creatorFunction; createSyntaxFunction; return MStatus::kSuccess; }
	MStatus			deregisterCommand(	const MString& commandName )
					{ commandName; return MStatus::kSuccess; }
	MStringArray	addMenuItem(
							const MString& menuItemName,
							const MString& parentName,
							const MString& commandName,
							const MString& commandParams,
							bool needOptionBox = false,
							const MString *optBoxFunction = NULL,
							MStatus *retStatus = NULL,
                            const MString *extraMenuItemParams = NULL
							)
					{ menuItemName; parentName; commandName; commandParams;
					  needOptionBox; optBoxFunction; retStatus; extraMenuItemParams; return MStringArray(); }
};

class MFnMesh : public MFnDagNode
{
public:
	MFnMesh() {}
	MFnMesh( MObject & object, MStatus * ret = NULL )
				{ object; ret; }
    MObject     create( int numVertices, int numPolygons,
						const MFloatPointArray &vertexArray,
                        const MIntArray &polygonCounts,
                        const MIntArray &polygonConnects,
						MObject parentOrOwner = MObject::kNullObj,
						MStatus * ReturnStatus = NULL )
				{ numVertices; numPolygons; vertexArray; polygonCounts; polygonConnects;
				  parentOrOwner; ReturnStatus; return MObject(); }
	MStatus		getUVSetNames(MStringArray &setNames) const
				{ setNames; return MStatus::kSuccess; }
	MStatus 	setUVs( const MFloatArray& uArray, const MFloatArray& vArray,
						const MString * uvSet = NULL )
				{ uArray; vArray; uvSet; return MStatus::kSuccess; }
	MStatus		assignUVs( const MIntArray& uvCounts, const MIntArray& uvIds,
						   const MString * uvSet = NULL )
				{ uvCounts; uvIds; uvSet; return MStatus::kSuccess;}
};

class MFnTransform : public MFnDagNode
{
public:
	MFnTransform() {}
	MFnTransform( MObject & object, MStatus * ret = NULL ) { object; ret; }
	MStatus 	setTranslation( const MVector & vec, MSpace::Space space ) { vec; space; return MStatus::kSuccess; }
};

class MFnLambertShader : public MFnDependencyNode
{
public:
	MFnLambertShader() {}
	MFnLambertShader( MObject & object, MStatus * ret = NULL ) { object; ret; }
	MObject     create( bool UIvisible = true, MStatus * ReturnStatus = NULL )
				{ UIvisible; ReturnStatus; return MObject();}
	MPlug		findPlug( const MString & attrName, bool wantNetworkedPlug,
								MStatus* ReturnStatus=NULL) const
				{ attrName; wantNetworkedPlug; ReturnStatus; return MPlug(); }
};

class MGlobal
{
public:
    static MStatus		getSelectionListByName( const MString& name,
												MSelectionList &list ) { name; list; return MStatus::kSuccess; }
	static MStatus		executeCommand( const MString& command,
										MStringArray& result,
										bool displayEnabled = false,
										bool undoEnabled = false )
						{ command; result; displayEnabled; undoEnabled; return MStatus::kSuccess; }
	static MString		executeCommandStringResult( const MString& command,
													bool displayEnabled = false,
													bool undoEnabled = false,
													MStatus * ResultStatus = NULL)
						{ command; displayEnabled; undoEnabled; ResultStatus; return MString(""); }
	static MStatus      deleteNode( MObject& ) {return MStatus::kSuccess;}
	static void			displayInfo( const MString & theMessage ) { theMessage; }
	static void			displayWarning( const MString & theWarning ) { theWarning; }
	static void			displayError( const MString & theError ) { theError; }
	static MString		optionVarStringValue(const MString& name, bool *exists = NULL)
						{ name; exists; return MString(""); }
	static bool			setOptionVarValue(const MString& name, MString value) { name; value; return true; }
	//getSelectionListByName
};

namespace MHWRender
{

enum MTextureType {
	kImage2D = 2,		//!< 2D image
};
enum MRasterFormat {
	kR8G8B8A8_UNORM = 49, //!< RGBA : 8-bits unsigned each
};
class MTexture
{
public:
};
class MTextureDescription
{
public:
	void setToDefault2DTexture() {}
	unsigned int fWidth;		//!< Width in pixels
	unsigned int fHeight;		//!< Height in pixels
	unsigned int fDepth;		//!< Depth in pixels. A 2D texture has depth of 1.
	unsigned int fBytesPerRow;	//!< Number of bytes in a row of pixels
	unsigned int fBytesPerSlice;//!< Number of bytes in a slice (if an array)
	unsigned int fMipmaps;		//!< Number of mipmap levels. 0 means the entire mipmap chain.
	unsigned int fArraySlices;	//!< Number of array slices. e.g. 6 would be required for a cube-map
	MRasterFormat fFormat;		//!< Pixel / raster format
	MTextureType	fTextureType;  //!< Type of texture
};
class MTextureManager
{
public:
	MTexture* acquireTexture(const MString& textureName, const MHWRender::MTextureDescription& textureDesc,
		const void* pixelData, bool generateMipMaps = true)
		{ textureName; textureDesc; pixelData; generateMipMaps; return nullptr; } // From memory
	void releaseTexture(MTexture* texture) const { texture; }
	MStatus saveTexture(MTexture* texture, const MString& filePath)
		{ texture; filePath; return MStatus::kSuccess;} // To disk
};
class MRenderer
{
public:
	static MRenderer *theRenderer(bool initializeRenderer = true) { initializeRenderer; return nullptr; }
	MTextureManager* getTextureManager() const {return nullptr;}
};

}; //namespace MHWRender

#endif // MAYASTUB_H
