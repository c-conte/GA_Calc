#include <default_gui_model.h>

class GA_Calc : public DefaultGUIModel
{
	
	public:
	
		GA_Calc(void);
		virtual~GA_Calc(void);
	
		void execute(void);
	
	protected:
	
		void update(DefaultGUIModel::update_flags_t);
	
	private:
	
		void derivs(double *, double *, double);
		void solve(double, double *, double);
		void initParameters();
	
		double y[2];
		double period;
		int steps;
		double a;
		double V;
		double V0;
		double Cm;
		double GA_MAX;
		double EA;
		double GA;
		double rate;
		double taua;
		double taub1;
		double cm;
		double IA;
		int onToggle;
};
