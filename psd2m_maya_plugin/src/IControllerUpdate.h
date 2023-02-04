//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file IControllerUpdate.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef ICONTROLLERUPDATE_H
#define ICONTROLLERUPDATE_H

namespace maya_plugin
{
	class IControllerUpdate
	{
	public:
		virtual ~IControllerUpdate() = default;
		virtual void Update() = 0;
	};
}
#endif // ICONTROLLERUPDATE_H
