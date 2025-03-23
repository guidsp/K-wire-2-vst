#include <sstream>
#include "vstgui/plugin-bindings/vst3editor.h"
#include "public.sdk/source/vst/utility/stringconvert.h"
#include "Kwire2controller.h"
#include "Kwire2cids.h"
#include "parameters.h"

using namespace Steinberg;

namespace Kwire2 {

//------------------------------------------------------------------------
// Kwire2Controller Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API Kwire2Controller::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

	// Here you could register some parameters
	makeParameters();

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Kwire2Controller::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API Kwire2Controller::setComponentState (IBStream* state)
{
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Kwire2Controller::setState (IBStream* state)
{
	// Here you get the state of the controller

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Kwire2Controller::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API Kwire2Controller::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		// create your editor here and return a IPlugView ptr of it
		auto* view = new VSTGUI::VST3Editor (this, "view", "Kwire2editor.uidesc");
		return view;
	}
	return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Kwire2Controller::setParamNormalized(Vst::ParamID tag, Vst::ParamValue value)
{
	// called by host to update your parameters
	tresult result = EditControllerEx1::setParamNormalized(tag, value);
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Kwire2Controller::getParamStringByValue(Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
	CustomParameter* param = &customParameters[tag];

	if (!param)
		return kResultFalse;

	std::stringstream display;
	display << std::fixed << std::setprecision(param->stepCount == 0 ? 2 : 0) << param->normalisedToPlain(valueNormalized);

	bool convert = VST3::StringConvert::convert(display.str(), string);
	return convert ? kResultTrue : kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Kwire2Controller::getParamValueByString(Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
	// called by host to get a normalized value from a string representation of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamValueByString(tag, string, valueNormalized);
}

Steinberg::Vst::ParamValue Kwire2Controller::normalizedParamToPlain(Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue valueNormalized)
{
	CustomParameter* param = &customParameters[tag];

	if (!param)
		return kResultFalse;

	return param->normalisedToPlain(valueNormalized);
}

void Kwire2Controller::makeParameters()
{
	// Converter for converting strings to utf 16 (accepted by vst3)
	std::wstring_convert<std::codecvt<char16_t, char, std::mbstate_t>, char16_t> converter;

	for (int i = 0; i < nParams; ++i) {
		CustomParameter* param = &customParameters[i];

		Vst::RangeParameter* p = new Vst::RangeParameter(converter.from_bytes(param->title).c_str(), i, converter.from_bytes(param->units).c_str(),
			param->minPlain, param->maxPlain, param->defaultPlain, param->stepCount, param->flags, 0, converter.from_bytes(param->shortTitle).c_str());

		p->setPrecision(param->stepCount == 0 ? 2 : 0);
		parameters.addParameter(p);
	}
}

//------------------------------------------------------------------------
} // namespace Kwire2
