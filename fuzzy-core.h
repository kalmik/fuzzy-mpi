#ifndef fuzzy_CORE
#define fuzzy_CORE
	
	typedef struct pertinence pertinence;
	struct pertinence
	{
		float* data;
		int size;
	};

	typedef struct rule rule;
	struct rule
	{
		float value;
		pertinence* then;
	};

	float max(float _a, float _b);

	float min(float _a, float _b);

	float orOp(float _a, float _b);

	float andOp(float _a, float _b);

	float trimf(float _value, float _points[]);

	float trapmf(float _value, float _points[]);

	float fuzzify(float _value, float* _points, int size);

	void defuzzify( double _value, float* _rules, int _size, double* uX, double* u);

#endif
