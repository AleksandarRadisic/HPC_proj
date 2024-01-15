#include "expression.h"

#include <chrono>
#include <iostream>
#include <algorithm>
#include <cmath>

Expression::Expression(Expression* exp1, Expression* exp2, Operation operation)
{
	left = exp1;
	right = exp2;
	this -> operation = operation;
	form_value();
	num = left->num + right->num;
	set_indexes(exp1, exp2);
	expression_string = form_expression_string();
}

Expression::Expression(int n, int i)
{
	value = static_cast<double>(n);
	priority = 0;
	num = 1;
	indexes.push_back(i);
	operation = none;
	expression_string = form_expression_string();
	valid = true;
}

void Expression::form_value()
{
	switch(operation)
	{
		case plus:
		{
			value = left->value + right->value;
			valid = std::ceil(value) == std::floor(value);
			priority = 1;
			break;
		}
		case minus:
		{
			value = left->value - right->value;
			valid = std::ceil(value) == std::floor(value);
			priority = 1;
			break;
		}
		case multiply:
		{
			value = left->value * right->value;
			valid = std::ceil(value) == std::floor(value);
			priority = 2;
			break;
		}
		case divide:
		{
			value = left->value / right->value;
			valid = std::ceil(value) == std::floor(value);
			priority = 2;
			break;
		}
		default:
		{
			break;
		}
	}
}

std::string Expression::form_expression_string()
{
	std::string str_expression;
	switch(operation)
	{
		case plus:
		{
			str_expression = left->expression_string + " + " + right->expression_string;
			break;
		}
		case minus:
		{
			str_expression = left->expression_string + " - ";
			if(right -> priority == 1)
			{
				str_expression += "( " + right->expression_string + " )";
				break;
			}
			str_expression += right->expression_string;
			break;
		}
		case multiply:
		{
			if(left -> priority == 1)
			{
				str_expression = "( " + left->expression_string + " )";
			}
			else
			{
				str_expression = left->expression_string;
			}
			str_expression += " * ";
			if(right -> priority == 1)
			{
				str_expression += "( " + right->expression_string + " )";
				break;
			}
			str_expression += right->expression_string;
			break;
		}
		case divide:
		{
			if (left->priority == 1)
			{
				str_expression = "( " + left->expression_string + " )";
			}
			else
			{
				str_expression = left->expression_string;
			}
			str_expression += " / ";
			if (right->priority != 0)
			{
				str_expression += "( " + right->expression_string + " )";
				break;
			}
			str_expression += right->expression_string;
			break;
		}
		case none:
		{
			str_expression = std::to_string((int)value);
			break;
		}
		default:
		{
			break;
		}
	}
	expression_string = str_expression;
	return str_expression;
}

void Expression::set_indexes(Expression* exp1, Expression* exp2)
{
	//auto start_time = std::chrono::high_resolution_clock::now();
	for(int n : exp1 -> indexes)
	{
		indexes.push_back(n);
	}
	for (int n : exp2 -> indexes) {
		if (std::find(indexes.begin(), indexes.end(), n) == indexes.end()) {
			indexes.push_back(n);
		}
	}
	//auto end_time = std::chrono::high_resolution_clock::now();
	//auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
	//std::cout << duration.count() << std::endl;
}

void Expression::print_indexes()
{
	for (auto it = indexes.begin(); it != indexes.end(); ++it)
	{
		std::cout << *it << std::endl;
	}
	std::cout << std::endl;
}

int Expression::compare(Expression* exp)
{
	constexpr int before = 1, equal = 0, after = -1;
	if (this == exp)
	{
		return equal;
	}
	if(expression_string == exp ->expression_string)
	{
		return equal;
	}
	if(value > exp -> value)
	{
		return before;
	}
	if(value < exp -> value)
	{
		return after;
	}
	if(num > exp -> num)
	{
		return before;
	}
	if(num < exp -> num)
	{
		return after;
	}
	return left->compare(exp->left);
}

void Expression::print_parents()
{
	std::cout << "LEFT: " << left->to_string() << " RIGHT: " << right->to_string() << std::endl;
}

std::string Expression::to_string()
{
	return expression_string + " = " + std::to_string((int)value);
}

void Expression::free_memory(Expression* root)
{
	if (root)
	{
		free_memory(root->left);
		free_memory(root->right);
		delete root;
	}
}







