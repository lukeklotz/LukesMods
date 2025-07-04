#include "plugin.hpp"
#include <cmath>


struct CascadeLFO : Module {
	enum ParamId {
		DIVS_PARAM,
		SPEED_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CLOCK_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUT4_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

    dsp::SchmittTrigger clockTrigger;
    float phase = 0.f;

    float phase1 = 0.f;
    float phase2 = 0.f;
    float phase3 = 0.f;
    float phase4 = 0.f;

	CascadeLFO() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(DIVS_PARAM, 0.1f, 10.f, 1.f, "");
		configParam(SPEED_PARAM, 0.1f, 2.f, 1.f, "");
		configInput(CLOCK_INPUT, "");
		configOutput(OUT1_OUTPUT, "");
		configOutput(OUT2_OUTPUT, "");
		configOutput(OUT3_OUTPUT, "");
		configOutput(OUT4_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
        float speed = params[SPEED_PARAM].getValue(); // volts/octave
        float freq = std::pow(5.f, speed);            // 5^pitch, in Hz
        float div = params[DIVS_PARAM].getValue();


        // compute phase for each output
        float freqs[4];
        float* phases[4] = { &phase1, &phase2, &phase3, &phase4 };

        for (int i = 0; i < 4; i++) {
          freqs[i] = freq / std::pow(div, i);
          *phases[i] += freqs[i] * args.sampleTime;
          if (*phases[i] >= 1.f) *phases[i] -= 1.f;
        }

        float amp = 5.f;

        outputs[OUT1_OUTPUT].setVoltage(amp * sinf(2.f * M_PI * phase1));
        outputs[OUT2_OUTPUT].setVoltage(amp * sinf(2.f * M_PI * phase2));
        outputs[OUT3_OUTPUT].setVoltage(amp * sinf(2.f * M_PI * phase3));
        outputs[OUT4_OUTPUT].setVoltage(amp * sinf(2.f * M_PI * phase4));
    }
};


struct CascadeLFOWidget : ModuleWidget {
	CascadeLFOWidget(CascadeLFO* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/cascadeLFO.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(5.111, 16.743)), module, CascadeLFO::DIVS_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(5.157, 33.269)), module, CascadeLFO::SPEED_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.094, 47.489)), module, CascadeLFO::CLOCK_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.196, 65.59)), module, CascadeLFO::OUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.174, 80.59)), module, CascadeLFO::OUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.272, 95.435)), module, CascadeLFO::OUT3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.365, 111.428)), module, CascadeLFO::OUT4_OUTPUT));
	}
};


Model* modelCascadeLFO = createModel<CascadeLFO, CascadeLFOWidget>("cascadeLFO");
