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
static inline double a_inf(double V, double thetaa, double sigmaa)
{
    //ainf(v)=1/(1+exp(-(v-thetaa)/sigmaa))
    return 1.0/(1.0+exp(-(V + 50.0)/20.0));
}

static inline double b_inf(double V, double thetab, double sigmab)
{
    //binf(v)=1/(1+exp(-(v-thetab)/sigmab))
    return 1.0/(1.0+exp(-(V + 70.0)/-6.0));
}

static inline double
tau_b(double V, double taub1)
{
    //taub(v)=taub0 + (taub1-taub0)/(1+exp(-(v-tb)/sb))
    return 10 + (taub1-10)/(1.0+exp(-(V-(-80))/10));
}

extern "C" Plugin::Object *createRTXIPlugin(void)
{
	return new GA_Calc();
}


static DefaultGUIModel::variable_t vars[] = 
{
	{"Input Voltage (V)", "Input Voltage (V)", DefaultGUIModel::INPUT,}, //read in in volts, need to multiply by 1e3 to get mV
	{"GA", "Calculated GA value to M_Neuron", DefaultGUIModel::OUTPUT,},
	{"IA", "Calculated IA value to Live", DefaultGUIModel::OUTPUT,},
	{"V0 (mV)", "Initial membrane potential (mV)", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
	{"GA_MAX (uS)", "Conductance", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE},
	{"EK (mV)", "K+ Reversal Potential", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE},
	{"Rate (Hz)", "Rate of integration (Hz)", DefaultGUIModel::PARAMETER | DefaultGUIModel::UINTEGER, },
	{"On/Off", "1 = on, 0 = off", DefaultGUIModel::PARAMETER | DefaultGUIModel::UINTEGER, },
    	{"taua", "",DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    	{"taub1", "",DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    	{"thetaa", "",DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    	{"thetab", "",DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    	{"sigmaa", "",DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    	{"sigmab", "",DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    	{"cm (nF)", "Specific membrane capacitance",DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
	{"a", "A-type Potassium Activation", DefaultGUIModel::STATE, },
	{"b", "A-type Potassium Inactivation", DefaultGUIModel::STATE, },
	{"GA", "Conductance of A-type Potassium Current", DefaultGUIModel::STATE, },
	{"IA", "Calculated IA value as a state", DefaultGUIModel::STATE, },
	{"IACell", "IA value that the cell receives", DefaultGUIModel::STATE, },
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
	    if(onToggle == 1){
		output(0) = GA;
		output(1) = IA;
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
			setParameter("GA_MAX (uS)", QString::number(GA_MAX)); // initialized in mS/mm^2, display in mS/cm^2
			setParameter("EK (mV)", QString::number(EA)); // initialized in mV, display in mV
			setParameter("Rate (Hz)", rate);
			setParameter("On/Off", onToggle);
            		setParameter("taua", QString::number(taua));
	    		setParameter("taub1", QString::number(taub1));
			setParameter("thetaa", QString::number(thetaa));
	    		setParameter("thetab", QString::number(thetab));
		        setParameter("sigmaa", QString::number(sigmaa));
	    		setParameter("sigmab", QString::number(sigmab));
            		setParameter("cm (nF)", QString::number(cm));
			setState("a",a);
			setState("b",b);
			setState("GA",GA);
			setState("IA",IA);
			setState("IACell",IACell);
			break;

		case MODIFY:
			V0 = getParameter("V0 (mV)").toDouble();
			GA_MAX = getParameter("GA_MAX (uS)").toDouble();
			EA = getParameter("EK (mV)").toDouble();
			rate = getParameter("Rate (Hz)").toDouble();
			onToggle = getParameter("On/Off").toInt();
			steps = static_cast<int> (ceil(period * rate));
	            	taua = getParameter("taua").toDouble();
           		taub1 = getParameter("taub1").toDouble();
	            	thetaa = getParameter("thetaa").toDouble();
           		thetab = getParameter("thetab").toDouble();
	            	sigmaa = getParameter("sigmaa").toDouble();
           		sigmab = getParameter("sigmab").toDouble();
			cm = getParameter("cm (nF)").toDouble();
			a = a_inf(V0, thetaa, sigmaa);
			b = b_inf(V0, thetab, sigmab);
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
	V0 = 0; // mV
	GA_MAX = .00055;
	EA = -90;
	rate = 400;
	onToggle = 0;
    	taua = 2.0;
    	taub1 = 200.0;
	thetaa = 50.0;
        thetab = 70.0;
        sigmaa = 20.0;
        sigmab = -6.0;		
   	cm = 0.0187;
	a = a_inf(V0, thetaa, sigmaa);
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
	GA = GACalc/cm;	
	IA = -GACalc*(V - EA)/cm;
	IACell = IA * 2e-9;
	da = (a_inf(V, thetaa, sigmaa) - a) / taua;
	db = (b_inf(V, thetab, sigmab) - b) / tau_b(V,taub1);
}
