#pragma once
#include <omp.h>
#include "expression.h"
#include <cstdint>

class Expression_builder
{
public:
	int target;
	std::vector<int> numbers;
	std::vector<Expression*> exp1;
	std::vector<Expression*> exp2;
	std::vector<Expression*> exp3;
	std::vector<Expression*> exp4;
	std::vector<Expression*> exp5;
	std::vector<Expression*> exp6;
	std::vector<Expression*> exp7;
	std::vector<Expression*> solutions;
	int min_dif;
	int total;
	bool found;
	bool find_all;
	omp_lock_t is_new_solution_lock;
	omp_lock_t exp2_lock;
	omp_lock_t exp3_lock;
	omp_lock_t exp4_lock;
	omp_lock_t exp5_lock;
	omp_lock_t exp6_lock;
	omp_lock_t exp7_lock;


	Expression_builder()
	{
		omp_init_lock(&is_new_solution_lock);
		omp_init_lock(&exp2_lock);
		omp_init_lock(&exp3_lock);
		omp_init_lock(&exp4_lock);
		omp_init_lock(&exp5_lock);
		omp_init_lock(&exp6_lock);
		omp_init_lock(&exp7_lock);
	}

	Expression_builder(int target, std::vector<int> numbers, bool find_all = true)
	{
		omp_init_lock(&is_new_solution_lock);
		omp_init_lock(&exp2_lock);
		omp_init_lock(&exp3_lock);
		omp_init_lock(&exp4_lock);
		omp_init_lock(&exp5_lock);
		omp_init_lock(&exp6_lock);
		omp_init_lock(&exp7_lock);
		this->target = target;
		this->numbers = numbers;
		min_dif = INT32_MAX;
		total = 0;
		this->find_all = find_all;
		found = false;
	}

	~Expression_builder();

	void start_build_seq();
	static bool can_merge(Expression* exp1, Expression* exp2);
	void print_all_solutions();
	void remove_duplicates();
	bool is_associative(Expression* exp1, Expression* exp2, Operation operation);
	bool is_new_solution(Expression* exp);
	void build1();
	void build_expression(std::vector<Expression*>& exp1, std::vector<Expression*>& exp2, std::vector<Expression*>& exp3);

	void start_build_parallel();
	void build_expression_parallel(std::vector<Expression*>& exp1, std::vector<Expression*>& exp2, std::vector<Expression*>& exp3, omp_lock_t* exp3_lock, int offset);

};
