#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	 p->addModel(modelOnePoleLPF);
     p->addModel(modelCascadeLFO);
	 p->addModel(modelFourPoleLPF);
}
