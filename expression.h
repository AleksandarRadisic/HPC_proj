#pragma once
#include <string>
#include <vector>

enum Operation
{
	plus,
	minus,
	multiply,
	divide,
	none
};

class Expression
{
public:
	double value;
	int num;
	int priority;
	bool valid;
	std::vector<int> indexes;
	Expression* left;
	Expression* right;
	Operation operation;
	std::string expression_string;

	Expression(Expression* exp1, Expression* exp2, Operation operation);
	Expression(int n, int i);
	std::string form_expression_string();
	void form_value();
	void set_indexes(Expression* exp1, Expression* exp2);
	void print_indexes();
	void print_parents();
	int compare(Expression* exp);
	std::string to_string();
	static void free_memory(Expression* root);
};
