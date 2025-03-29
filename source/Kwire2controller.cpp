#include <sstream>
#include "base/source/fstreamer.h"
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
	IBStreamer streamer(state, kLittleEndian);
	std::string paramTitle;

	while (true)
	{
		auto identifier = streamer.readStr8();

		if (!identifier)
			break;

		paramTitle = std::string(identifier);

		CustomParameter* parameter = parameterWithTitle(paramTitle);

		if (!parameter)
			continue;

		double value;

		if (!streamer.readDouble(value))
			continue;

		setParamNormalized(parameter->id, parameter->plainToNormalised(value));
	}

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Kwire2Controller::getState (IBStream* state)
{
	IBStreamer streamer(state, kLittleEndian);

	for (CustomParameter& parameter : customParameters)
	{
		if (!streamer.writeStr8(parameter.title.c_str())) return kResultFalse;
		if (!streamer.writeDouble(parameter.normalisedToPlain(parameter.normalisedValue))) return kResultFalse;
	}

	return kResultOk;
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
	return EditControllerEx1::setParamNormalized(tag, value);;
}

Steinberg::Vst::ParamValue Kwire2Controller::getParamNormalized(Steinberg::Vst::ParamID tag)
{
	return EditControllerEx1::getParamNormalized(tag);
}

//------------------------------------------------------------------------
tresult PLUGIN_API Kwire2Controller::getParamStringByValue(Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
	CustomParameter* param = &customParameters[tag];

	if (param)
	{
		std::stringstream display;
		display << std::fixed << std::setprecision(param->stepCount == 0 ? 2 : 0) << param->normalisedToPlain(valueNormalized) << " " << param->units;

		bool convert = Steinberg::Vst::StringConvert::convert(display.str(), string);
		return convert ? kResultTrue : kResultFalse;
	}

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Kwire2Controller::getParamValueByString(Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
	// called by host to get a normalized value from a string representation of a specific parameter
	// (without having to set the value!)
	CustomParameter* param = &customParameters[tag];

	if (param)
	{
		std::string str;
		bool convert = Steinberg::Vst::StringConvert::convert(str, string);

		if (convert)
		{
			valueNormalized = param->plainToNormalised(std::stod(str));

			return kResultTrue;
		}
	}

	return EditControllerEx1::getParamValueByString(tag, string, valueNormalized);
}

Steinberg::Vst::ParamValue Kwire2Controller::normalizedParamToPlain(Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue valueNormalized)
{
	return EditControllerEx1::normalizedParamToPlain(tag, valueNormalized);
}

Steinberg::Vst::ParamValue Kwire2Controller::plainParamToNormalized(Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue plainValue)
{
	return EditControllerEx1::plainParamToNormalized(tag, plainValue);
}

// Fix defaults being warped with modifier.
void Kwire2Controller::makeParameters()
{
	for (int i = 0; i < nParams; ++i) 
	{
		CustomParameter* param = &customParameters[i];

		// Convert strings to UTF-16 encoded std::u16string
		const std::u16string utf16Title = toU16String(param->title);
		const std::u16string uft16Units = toU16String(" ");
		const std::u16string utf16ShortTitle = toU16String(param->shortTitle);

		Vst::RangeParameter* p = new Vst::RangeParameter(utf16Title.c_str(), i, uft16Units.c_str(),
			0, 1, param->plainToNormalised(param->defaultPlain), param->stepCount, param->flags, 0, utf16ShortTitle.c_str());

		p->setPrecision(param->stepCount == 0 ? 2 : 0);
		parameters.addParameter(p);
	}
}

//------------------------------------------------------------------------
} // namespace Kwire2
