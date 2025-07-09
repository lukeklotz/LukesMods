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

struct OnePoleLPF : Module {
	enum ParamId {
		CUTOFF_PARAM,
		RES_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CV_INPUT,
		IN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	OnePoleLPF() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(CUTOFF_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RES_PARAM, 0.f, 1.f, 0.f, "");
		configInput(CV_INPUT, "");
		configInput(IN_INPUT, "");
		configOutput(OUT_OUTPUT, "");
	}

    LP filter;

	void process(const ProcessArgs& args) override {
        float input = inputs[IN_INPUT].getVoltage();
    
        float baseFreq = params[CUTOFF_PARAM].getValue();
        float freqCV = inputs[CV_INPUT].isConnected() ? inputs[CV_INPUT].getVoltage() : 0.f; //this might act weird because theres no attenuator
        float finalFreq = baseFreq + freqCV;
        float cutoffHz = dsp::FREQ_C4 * std::pow(2.f, finalFreq);

        // clamp for safety
        cutoffHz = clamp(cutoffHz, 1.f, args.sampleRate * 0.45f);

        filter.setCutoff(cutoffHz, args.sampleRate);
        float filteredOutput = filter.process(input);
        outputs[OUT_OUTPUT].setVoltage(filteredOutput);
	}
};


struct OnePoleLPFWidget : ModuleWidget {
	OnePoleLPFWidget(OnePoleLPF* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/OnePoleLPF.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.256, 25.052)), module, OnePoleLPF::CUTOFF_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.154, 56.101)), module, OnePoleLPF::RES_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.132, 76.418)), module, OnePoleLPF::CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.302, 93.126)), module, OnePoleLPF::IN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.29, 110.028)), module, OnePoleLPF::OUT_OUTPUT));
	}
};


Model* modelOnePoleLPF = createModel<OnePoleLPF, OnePoleLPFWidget>("OnePoleLPF");
