//------------------------------------------------------------------------
// Copyright(c) 2025 Laser Brain.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "public.sdk/source/vst/utility/stringconvert.h"

#include "CustomParameter.h"
#include "parameters.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace Kwire2 {

//------------------------------------------------------------------------
//  Kwire2Controller
//------------------------------------------------------------------------
class Kwire2Controller : public Steinberg::Vst::EditControllerEx1
{
public:
//------------------------------------------------------------------------
	Kwire2Controller () = default;
	~Kwire2Controller () SMTG_OVERRIDE = default;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/)
	{
		return (Steinberg::Vst::IEditController*)new Kwire2Controller;
	}

	//--- from IPluginBase -----------------------------------------------
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;

	//--- from EditController --------------------------------------------
	Steinberg::tresult PLUGIN_API setComponentState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setParamNormalized(Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue value) SMTG_OVERRIDE;
	Steinberg::Vst::ParamValue PLUGIN_API getParamNormalized(Steinberg::Vst::ParamID tag) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getParamStringByValue(Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue valueNormalized, Steinberg::Vst::String128 string) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getParamValueByString(Steinberg::Vst::ParamID tag, Steinberg::Vst::TChar* string, Steinberg::Vst::ParamValue& valueNormalized) SMTG_OVERRIDE;
	Steinberg::Vst::ParamValue PLUGIN_API normalizedParamToPlain(Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue valueNormalized) SMTG_OVERRIDE;
	Steinberg::Vst::ParamValue PLUGIN_API plainParamToNormalized(Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue plainValue) SMTG_OVERRIDE;

 	//---Interface---------
	DEFINE_INTERFACES
		// Here you can add more supported VST3 interfaces
		// DEF_INTERFACE (Vst::IXXX)
	END_DEFINE_INTERFACES (EditController)
    DELEGATE_REFCOUNT (EditController)

//------------------------------------------------------------------------
protected:
	CustomParameter* parameterWithTitle(const std::string name)
	{
		for (ParamID id = 0; id < nParams; ++id)
		{
			if (customParameters[id].title == name)
				return &customParameters[id];
		}

		return nullptr;
	}
};

//------------------------------------------------------------------------
} // namespace Kwire2
