#include "plugin.hpp"

struct LP {
  float y = 0.f;
  float alpha = 0.f;

  void setCutoff(float freq, float sampleRate) {
    float omega0 = 2.f * M_PI * freq;
    alpha = omega0 / (omega0 + sampleRate);
  }

  void reset() {
    y = 0.f;
  }

  float process(float x) {
    y += alpha * (x - y);
    return y;
  }
};

struct FourPoleLPF : Module {
	enum ParamId {
		CUT_PARAM,
		RES_PARAM,
		ATN_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CV_INPUT,
		MAIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		MAIN_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	FourPoleLPF() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(CUT_PARAM, 0.f, 5.f, 2.f, "");
		configParam(RES_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ATN_PARAM, 0.f, 1.f, 0.f, "");
		configInput(CV_INPUT, "");
		configInput(MAIN_INPUT, "");
		configOutput(MAIN_OUTPUT, "");
	}


	LP filter[4];

	void process(const ProcessArgs& args) override {
		float in = inputs[MAIN_INPUT].getVoltage();
		float cut = params[CUT_PARAM].getValue();
		float res = params[RES_PARAM].getValue();
		float cv = inputs[CV_INPUT].isConnected() ? inputs[CV_INPUT].getVoltage() : 0.f;
		float freq = cut + cv;

		float cutoffHz = dsp::FREQ_C4 * std::pow(2.f, freq);
		cutoffHz = clamp(cutoffHz, 1.f, args.sampleRate * 0.45f);

		for (int i = 0; i < 4; ++i) {
			filter[i].setCutoff(cutoffHz, args.sampleRate);
		}

		res = res * res * 4.f;
		float x = in - res * filter[3].y;

		float y = x;
		for (int i = 0; i < 4; ++i) {
			y = filter[i].process(y);
		}

		outputs[MAIN_OUTPUT].setVoltage(y);
		
	}
};


struct FourPoleLPFWidget : ModuleWidget {
	FourPoleLPFWidget(FourPoleLPF* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/FourPoleLPF.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(19.749, 25.999)), module, FourPoleLPF::CUT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(19.712, 47.834)), module, FourPoleLPF::RES_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(32.72, 67.803)), module, FourPoleLPF::ATN_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.763, 68.088)), module, FourPoleLPF::CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.302, 93.126)), module, FourPoleLPF::MAIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.29, 110.028)), module, FourPoleLPF::MAIN_OUTPUT));
	}
};


Model* modelFourPoleLPF = createModel<FourPoleLPF, FourPoleLPFWidget>("FourPoleLPF");