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
	
		void derivs(double *, double *);
		void solve(double, double *);
		void initParameters();
	
		double y[2];
		double period;
		int steps;
	
		double V0;
		double Cm;
		double GA_max;
		double EA;
		double IA;
		double rate;
};