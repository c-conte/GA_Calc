#include <GA_Calc.h>
#include <math.h>

/*
static inline double a_inf(double V)
{
	return pow(0.0761 * exp(0.0314 * (V + 94.22)) / (1 + exp(0.0346 * (V + 1.17))),(1 / 3.0));
}

static inline double tau_a(double V)
{
	return 0.3632e-3 + 1.158e-3 / (1 + exp(0.0497 * (V + 55.96)));
}

static inline double b_inf(double V)
{
	return pow(1 / (1 + exp(0.0688 * (V + 53.3))), 4.0);
}

static inline double tau_b(double V)
{
	return 1.24e-3 + 2.678e-3 / (1 + exp(0.0624 * (V + 50.0)));
}
*/
static inline double a_inf(double V)
{
    //ainf(v)=1/(1+exp(-(v-thetaa)/sigmaa))
    return 1.0/(1.0+exp(-(V + 50.0)/20.0));
}

static inline double b_inf(double V)
{
    //binf(v)=1/(1+exp(-(v-thetab)/sigmab))
    return 1.0/(1.0+exp(-(V + 70.0)/-6.0));
}

static inline double
tau_b(double V)
{
    //taub(v)=taub0 + (taub1-taub0)/(1+exp(-(v-tb)/sb))
    return 10 + (200-10)/(1.0+exp(-(V-(-80))/10));
}

extern "C" Plugin::Object *createRTXIPlugin(void)
{
	return new GA_Calc();
}


static DefaultGUIModel::variable_t vars[] = 
{
	{"Input Voltage (V)", "Input Voltage (V)", DefaultGUIModel::INPUT,}, //read in in volts, need to multiply by 1e3 to get mV
	{"GA", "Calculated GA value", DefaultGUIModel::OUTPUT,},
	{"V0 (mV)", "Initial membrane potential (mV)", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
	{"GA_MAX", "Conductance", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE},
	{"EA", "A-type K+ Reversal Potential", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE},
	{"Rate (Hz)", "Rate of integration (Hz)", DefaultGUIModel::PARAMETER | DefaultGUIModel::UINTEGER, },
	{"Toggle Block", "1 = block, 0 = off", DefaultGUIModel::PARAMETER | DefaultGUIModel::UINTEGER, },
	{"a", "A-type Potassium Activation", DefaultGUIModel::STATE, },
	{"b", "A-type Potassium Inactivation", DefaultGUIModel::STATE, },
	{"GA", "Conductance of A-type Potassium Current", DefaultGUIModel::STATE, },
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);
/*
#define a (y[0])
#define b (y[1])
#define da (dydt[0])
#define db (dydt[1])
*/
#define a (y[0])
#define b (y[1])
#define da (dydt[0])
#define db (dydt[1])
#define GACalc (GA_MAX*a*a*a*b)

GA_Calc::GA_Calc(void) : DefaultGUIModel("GA_Calc", ::vars, ::num_vars)
{
	createGUI(vars, num_vars);
	initParameters();
	update( INIT );
	refresh();
	resizeMe();
}

GA_Calc::~GA_Calc(void){}

void GA_Calc::execute(void)
{
	
	for (int i = 0; i < steps; ++i){
	    V = input(0)*1e3; //converts to mV
	    solve(period / steps, y,V);
	    if(blockToggle == 1){
		output(0) = GA;
	    }
	    else{
		output(0) = 0;
	    } 	
	}

	return;
}

void GA_Calc::update(DefaultGUIModel::update_flags_t flag){
	switch(flag){
		case INIT:
			setParameter("V0 (mV)", QString::number(V0)); // initialized in mV, display in mV
			setParameter("GA_MAX", QString::number(GA_MAX)); // initialized in mS/mm^2, display in mS/cm^2
			setParameter("EA", QString::number(EA)); // initialized in mV, display in mV
			setParameter("Rate (Hz)", rate);
			setParameter("Toggle Block", blockToggle);
			setState("a",a);
			setState("b",b);
			setState("GA",GA);
			break;

		case MODIFY:
			V0 = getParameter("V0 (mV)").toDouble();
			GA_MAX = getParameter("GA_MAX").toDouble();
			EA = getParameter("EA").toDouble();
			rate = getParameter("Rate (Hz)").toDouble();
			blockToggle = getParameter("Toggle Block").toInt();
			steps = static_cast<int> (ceil(period * rate));
			a = a_inf(V0);
			b = b_inf(V0);
			break;

		case PERIOD:
			period = RT::System::getInstance()->getPeriod() * 1e-6; // time in seconds
			steps = static_cast<int> (ceil(period * rate));
			break;

		default:
			break;
	}
}

void GA_Calc::initParameters() {
	V0 = -60.0; // mV
	GA_MAX = .1;
	EA = -90;
	rate = 400;
	blockToggle = 0;
	a = a_inf(V0);
	b = .47;
	period = RT::System::getInstance()->getPeriod() * 1e-6; // s
	steps = static_cast<int> (ceil(period * rate)); // calculate how many integrations to perform per execution step
}


void GA_Calc::solve(double dt, double *y, double V){
	double dydt[2];
	derivs(y, dydt,V);
	for (size_t i = 0; i < 2; ++i)
		y[i] += dt * dydt[i];
}

void GA_Calc::derivs(double *y, double *dydt, double V){
	GA = GACalc;	
	da = (a_inf(V) - a) / 2.0;
	db = (b_inf(V) - b) / tau_b(V);
}
