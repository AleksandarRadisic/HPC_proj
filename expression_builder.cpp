#include <algorithm>

#include "expression_builder.h"

#include <iostream>
#include <cmath>
#include <omp.h>
#include <cstdint>
#include <chrono>


Expression_builder::~Expression_builder()
{
	for (int i = 0; i < exp1.size(); i++)delete exp1[i];
	for (int i = 0; i < exp2.size(); i++)delete exp2[i];
	for (int i = 0; i < exp3.size(); i++)delete exp3[i];
	for (int i = 0; i < exp4.size(); i++)delete exp4[i];
	for (int i = 0; i < exp5.size(); i++)delete exp5[i];
	for (int i = 0; i < exp6.size(); i++)delete exp6[i];
	for (int i = 0; i < exp7.size(); i++)delete exp7[i];
}


void Expression_builder::start_build_seq()
{
	std::sort(numbers.begin(), numbers.end());

	auto start_time = std::chrono::high_resolution_clock::now();

	build1();

	build_expression(exp1, exp1, exp2);
	//std::cout<<"\nEXP2 SIZE: "<<exp2.size() <<std::endl;

	build_expression(exp2, exp1, exp3);
	//std::cout<<"\nEXP3 SIZE: "<<exp3.size() <<std::endl;

	build_expression(exp3, exp1, exp4);
	//std::cout<<"\nEXP4 SIZE: "<<exp4.size() <<std::endl;
	build_expression(exp2, exp2, exp4);
	//std::cout<<"\nEXP4 SIZE: "<<exp4.size() <<std::endl;

	build_expression(exp4, exp1, exp5);
	//std::cout<<"\nEXP5 SIZE: "<<exp5.size() <<std::endl;

	build_expression(exp3, exp2, exp5);
	//std::cout<<"\nEXP5 SIZE: "<<exp5.size() <<std::endl;

	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << "Expressions up to 5 numbers calculated in (SEQUENTIAL): " << duration.count() << " milliseconds" << std::endl;


	start_time = std::chrono::high_resolution_clock::now();

	build_expression(exp5, exp1, exp6);
	//std::cout<<"\nEXP6 SIZE: "<<exp6.size() <<std::endl;
	build_expression(exp4, exp2, exp6);
	//std::cout<<"\nEXP6 SIZE: "<<exp6.size() <<std::endl;
	build_expression(exp3, exp3, exp6);
	//std::cout<<"\nEXP6 SIZE: "<<exp6.size() <<std::endl;

	build_expression(exp6, exp1, exp7);
	//std::cout<<"\nEXP7 SIZE: "<<exp7.size() <<std::endl;
	build_expression(exp5, exp2, exp7);
	//std::cout<<"\nEXP7 SIZE: "<<exp7.size() <<std::endl;
	build_expression(exp4, exp3, exp7);
	//std::cout<<"\nEXP7 SIZE: "<<exp7.size() <<std::endl;

	end_time = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << "Expressions with 6 and 7 numbers calculated in (SEQUENTIAL): " << duration.count() << " milliseconds" << std::endl;

	total = exp1.size() + exp2.size() + exp3.size() + exp4.size() + exp5.size() + exp6.size() + exp7.size();

	remove_duplicates();
}

void Expression_builder::start_build_parallel()
{
	//omp_set_num_threads(8);
	auto start_time = std::chrono::high_resolution_clock::now();

	#pragma omp parallel
	#pragma omp single
	{
		//std::cout<<omp_get_num_threads()<<std::endl;
		std::sort(numbers.begin(), numbers.end());

		build1();

		#pragma omp task
		{
			build_expression_parallel(exp1, exp1, exp2, &exp2_lock, -1);//TASK1 -- independent
			//std::cout<<"\nEXP2 SIZE: "<<exp2.size() <<std::endl;
			build_expression_parallel(exp2, exp1, exp3, &exp3_lock, -1);//TASK2 -- depends on task1
			//std::cout<<"\nEXP3 SIZE: "<<exp3.size() <<std::endl;
		}

		#pragma omp taskwait

		#pragma omp task
		{
			#pragma omp task
			{
				build_expression_parallel(exp3, exp2, exp5, &exp5_lock, -1);//TASK9 -- depends on task1 and task2
				//std::cout<<"\nEXP5 SIZE: "<<exp5.size() <<std::endl;
			}
			build_expression_parallel(exp2, exp2, exp4, &exp4_lock, -1);//TASK4 -- depends on task1
			//std::cout<<"\nEXP4 SIZE: "<<exp4.size() <<std::endl;
		}

		#pragma omp task
		{
			build_expression_parallel(exp3, exp1, exp4, &exp4_lock, -1);//TASK5 -- depends on task2
			//std::cout<<"\nEXP4 SIZE: "<<exp4.size() <<std::endl;
		}
		#pragma omp taskwait
		#pragma omp task
		{
			build_expression_parallel(exp4, exp1, exp5, &exp5_lock, -1);//TASK8 -- depends on task4 and task5
			//std::cout<<"\nEXP5 SIZE: "<<exp5.size() <<std::endl;
		}

	}

	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << "Expressions up to 5 numbers calculated in (PARALLEL - TASK BASED): " << duration.count() << " milliseconds" << std::endl;

	start_time = std::chrono::high_resolution_clock::now();

	int offset = 0;
	exp6 = std::vector<Expression*>(exp5.size() * exp1.size() * 8, nullptr);
	build_expression_parallel(exp5, exp1, exp6, &exp6_lock, 0);//TASK6 -- depends on task1 and task4 and task5
	exp6.erase(std::remove_if(exp6.begin(), exp6.end(), [](Expression* ptr) { return ptr == nullptr; }), exp6.end());
	offset = exp6.size();
	//std::cout<<"\nEXP6 SIZE: "<<exp6.size() <<std::endl;

	exp6.insert(exp6.end(), exp4.size() * exp2.size() * 8, nullptr);
	build_expression_parallel(exp4, exp2, exp6, &exp6_lock, offset);//TASK10 -- depends on task8 and task9 
	exp6.erase(std::remove_if(exp6.begin(), exp6.end(), [](Expression* ptr) { return ptr == nullptr; }), exp6.end());
	offset = exp6.size();
	//std::cout<<"\nEXP6 SIZE: "<<exp6.size() <<std::endl;

	exp6.insert(exp6.end(), exp3.size() * exp3.size() * 8, nullptr);
	build_expression_parallel(exp3, exp3, exp6, &exp6_lock, offset);//TASK3 -- depends on task2
	exp6.erase(std::remove_if(exp6.begin(), exp6.end(), [](Expression* ptr) { return ptr == nullptr; }), exp6.end());
	//std::cout<<"\nEXP6 SIZE: "<<exp6.size() <<std::endl;

	exp7 = std::vector<Expression*>(exp6.size() * exp1.size() * 8, nullptr);
	offset = 0;
	build_expression_parallel(exp6, exp1, exp7, &exp7_lock, offset);//TASK7 -- depends on task2 and task4 and task5
	exp7.erase(std::remove_if(exp7.begin(), exp7.end(), [](Expression* ptr) { return ptr == nullptr; }), exp7.end());
	//std::cout<<"\nEXP7 SIZE: "<<exp7.size() <<std::endl;
	offset = exp7.size();

	exp7.insert(exp7.end(), exp5.size() * exp2.size() * 8, nullptr);
	build_expression_parallel(exp5, exp2, exp7, &exp7_lock, offset);//TASK11 -- depends on task1 and task8 and task9
	exp7.erase(std::remove_if(exp7.begin(), exp7.end(), [](Expression* ptr) { return ptr == nullptr; }), exp7.end());
	//std::cout<<"\nEXP7 SIZE: "<<exp7.size() <<std::endl;
	offset = exp7.size();

	exp7.insert(exp7.end(), exp4.size() * exp3.size() * 8, nullptr);
	build_expression_parallel(exp4, exp3, exp7, &exp7_lock, offset);//TASK12 -- depends on task3 and task6 and task10
	exp7.erase(std::remove_if(exp7.begin(), exp7.end(), [](Expression* ptr) { return ptr == nullptr; }), exp7.end());
	//std::cout<<"\nEXP7 SIZE: "<<exp7.size() <<std::endl;

	end_time = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << "Expressions with 6 and 7 numbers calculated in (PARALLEL): " << duration.count() << " milliseconds" << std::endl;

	total = exp1.size() + exp2.size() + exp3.size() + exp4.size() + exp5.size() + exp6.size() + exp7.size();

	remove_duplicates();
}


bool Expression_builder::can_merge(Expression* exp1, Expression* exp2)
{
	for(auto it : exp1 -> indexes)
	{
		for (auto it2 : exp2->indexes)
		{
			if (it == it2)
			{
				return false;
			}
		}
	}
	return true;
}

void Expression_builder::print_all_solutions()
{
	std::cout << "Solutions:\n";
	for(auto it = solutions.begin(); it != solutions.end(); ++it)
	{
		std::cout << (*it)->to_string()  << std::endl;
	}
	std::cout << "Total " << solutions.size() << std::endl;
}

void Expression_builder::remove_duplicates()
{
	for (int i =0;i<solutions.size();++i)
	{
		for (int j = i+1; j < solutions.size(); ++j)
		{
			if(solutions[j]->expression_string == solutions[i] ->expression_string)
			{
				solutions.erase(solutions.begin() + j);
				--j;
			}
		}
	}
}

bool Expression_builder::is_associative(Expression* exp1, Expression* exp2, Operation operation)
{
	if(operation == plus || operation == multiply)
	{
		if(exp1 -> operation == operation)
		{
			return !(exp1->right->compare(exp2) >= 0);
		}
		return !(exp1->compare(exp2) >= 0);

	}
	if(exp1->num == 1)
	{
		return false;
	}
	if(exp1->operation != operation)
	{
		return false;
	}
	return !(exp1->right->compare(exp2) >= 0);
}

bool Expression_builder::is_new_solution(Expression* exp)
{
	
	if(exp -> value == target)
	{
		if(min_dif != 0)
		{
			omp_set_lock(&is_new_solution_lock);
			solutions.clear();
			min_dif = 0;
			omp_unset_lock(&is_new_solution_lock);
		}
		found = true;
		if(!find_all)
		{
			omp_set_lock(&is_new_solution_lock);
			solutions.push_back(exp);
			omp_unset_lock(&is_new_solution_lock);
			return true;
		}
		omp_set_lock(&is_new_solution_lock);
		solutions.push_back(exp);
		omp_unset_lock(&is_new_solution_lock);
	}
	else if(std::abs(exp -> value - target) < min_dif)
	{
		if(exp -> valid)
		{
			omp_set_lock(&is_new_solution_lock);
			min_dif = (int) std::abs(exp->value - target);
			solutions.clear();
			solutions.push_back(exp);
			omp_unset_lock(&is_new_solution_lock);
		}
	}
	else if(std::abs(exp -> value - target) == min_dif)
	{
		omp_set_lock(&is_new_solution_lock);
		solutions.push_back(exp);
		omp_unset_lock(&is_new_solution_lock);
	}
	return false;
}

void Expression_builder::build1()
{
	for(int i =0;i<numbers.size();i++)
	{
		auto exp = new Expression(numbers[i], i);
		exp1.push_back(exp);
		if(is_new_solution(exp))
		{
			return;
		}
	}
}

void Expression_builder::build_expression(std::vector<Expression*>& exp1, std::vector<Expression*>& exp2, std::vector<Expression*>& exp3)
{
	auto start_time = std::chrono::high_resolution_clock::now();
	if(found && !find_all)
	{
		return;
	}
	int num1 = exp1[0]->num;
	int num2 = exp2[0]->num;
	for (Expression* i : exp1) 
	{
		for(auto j : exp2)
		{
			auto exxp1 = i;
			auto exxp2 = j;
			if(!can_merge(exxp1, exxp2))
			{
				continue;
			}
			if(num1 == num2 && i >= j)
			{
				continue;
			}

			//A+B
			if(exxp1 -> operation != minus && exxp2->priority != 1)
			{
				if(!is_associative(exxp1, exxp2, plus))
				{
					auto e1 = new Expression(exxp1, exxp2, plus);
					exp3.push_back(e1);
					if(is_new_solution(e1))
					{
						return;
					}
				}
			}
			//B+A
			if(exxp2 -> operation != minus && exxp1 -> priority != 1)
			{
				if(!is_associative(exxp2, exxp1, plus))
				{
					auto e1 = new Expression(exxp2, exxp1, plus);
					exp3.push_back(e1);
					if(is_new_solution(e1))
					{
						return;
					}
				}
			}

			//A*B
			if(exxp1 -> value != 1 && exxp2 -> value != 1 && 
				exxp1 -> operation != divide && exxp2 -> priority != 2)
			{
				if (!is_associative(exxp1, exxp2, multiply))
				{
					auto e2 = new Expression(exxp1, exxp2, multiply);
					exp3.push_back(e2);
					if(is_new_solution(e2))
					{
						return;
					}
				}
			}

			//B*A
			if (exxp2->value != 1 && exxp1->value != 1 &&
				exxp2->operation != divide && exxp1->priority != 2)
			{
				if (!is_associative(exxp2, exxp1, multiply))
				{
					auto e2 = new Expression(exxp2, exxp1, multiply);
					exp3.push_back(e2);
					if (is_new_solution(e2))
					{
						return;
					}
				}
			}

			//A-B
			if(exxp1->value > exxp2->value)
			{
				if(exxp2->priority != 1)
				{
					if(!is_associative(exxp1, exxp2, minus))
					{
						auto e3 = new Expression(exxp1, exxp2, minus);
						exp3.push_back(e3);
						if(is_new_solution(e3))
						{
							return;
						}
					}
				}
			}
			//B-A, ako je B>A
			else if(exxp2 -> value > exxp1 -> value)
			{
				if(exxp1 -> priority != 1)
				{
					if(!is_associative(exxp2, exxp1, minus))
					{
						auto e3 = new Expression(exxp2, exxp1, minus);
						exp3.push_back(e3);
						if (is_new_solution(e3))
						{
							return;
						}
					}
				}
			}

			//A/B
			if(exxp2 -> value != 1 && exxp2 -> priority != 2)
			{
				if(!is_associative(exxp1, exxp2, divide))
				{
					auto e4 = new Expression(exxp1, exxp2, divide);
					exp3.push_back(e4);
					if(is_new_solution(e4))
					{
						return;
					}
				}
			}
			//B/A
			if(exxp1 -> value != 1 && exxp1 -> priority != 2)
			{
				if(!is_associative(exxp2, exxp1, divide))
				{
					auto e4 = new Expression(exxp2, exxp1, divide);
					exp3.push_back(e4);
					if(is_new_solution(e4))
					{
						return;
					}
				}
			}
		}
	}
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	//std::cout << "Time taken by code SEQUENTIAL: " << duration.count() << " milliseconds" << "-- SIZE " << exp3.size() << "-- " << std::endl;
}

void Expression_builder::build_expression_parallel(std::vector<Expression*>& exp1, std::vector<Expression*>& exp2, std::vector<Expression*>& exp3, omp_lock_t* exp3_lock, int offset)
{
	std::vector<int> results;
	std::vector<std::string> strings;
	auto start_time = std::chrono::high_resolution_clock::now();
	if (found && !find_all)
	{
		return;
	}
	int num1 = exp1[0]->num;
	int num2 = exp2[0]->num;
	bool should_exit = false;
	#pragma omp parallel for
	for (int i =0;i<exp1.size();i++)
	{
		if (should_exit)continue;
		for (int j = 0; j < exp2.size(); j++)
		{
			auto exxp1 = exp1[i];
			auto exxp2 = exp2[j];
			if (!can_merge(exxp1, exxp2))
			{
				continue;
			}
			if (num1 == num2 && i >= j)
			{
				continue;
			}

			//A+B
			if (exxp1->operation != minus && exxp2->priority != 1)
			{
				if (!is_associative(exxp1, exxp2, plus))
				{
					auto e1 = new Expression(exxp1, exxp2, plus);
					if(offset == -1){
						omp_set_lock(exp3_lock);
						exp3.push_back(e1);
						omp_unset_lock(exp3_lock);
					}
					else{
						size_t pos = (size_t)offset + (size_t)i * (size_t)exp2.size() * 8 + (size_t)j * 8;
						exp3[pos] = e1;
					}
					if (is_new_solution(e1))
					{
						should_exit = true;
						break;
					}
				}
			}
			//B+A
			if (exxp2->operation != minus && exxp1->priority != 1)
			{
				if (!is_associative(exxp2, exxp1, plus))
				{
					auto e1 = new Expression(exxp2, exxp1, plus);
					if(offset == -1){
						omp_set_lock(exp3_lock);
						exp3.push_back(e1);
						omp_unset_lock(exp3_lock);
					}
					else{
						size_t pos = (size_t)offset + (size_t)i * (size_t)exp2.size() * 8 + (size_t)j * 8 + 1;
						exp3[pos] = e1;
					}
					if (is_new_solution(e1))
					{
						should_exit = true;
						break;
					}
				}
			}

			//A*B
			if (exxp1->value != 1 && exxp2->value != 1 &&
				exxp1->operation != divide && exxp2->priority != 2)
			{
				if (!is_associative(exxp1, exxp2, multiply))
				{
					auto e2 = new Expression(exxp1, exxp2, multiply);
					if(offset == -1){
						omp_set_lock(exp3_lock);
						exp3.push_back(e2);
						omp_unset_lock(exp3_lock);
					}
					else{
						size_t pos = (size_t)offset + (size_t)i * (size_t)exp2.size() * 8 + (size_t)j * 8 + 2;
						exp3[pos] = e2;
					}
					if (is_new_solution(e2))
					{
						should_exit = true;
						break;
					}
				}
			}

			//B*A
			if (exxp2->value != 1 && exxp1->value != 1 &&
				exxp2->operation != divide && exxp1->priority != 2)
			{
				if (!is_associative(exxp2, exxp1, multiply))
				{
					auto e2 = new Expression(exxp2, exxp1, multiply);
					if(offset == -1){
						omp_set_lock(exp3_lock);
						exp3.push_back(e2);
						omp_unset_lock(exp3_lock);
					}
					else{
						size_t pos = (size_t)offset + (size_t)i * (size_t)exp2.size() * 8 + (size_t)j * 8 + 3;
						exp3[pos] = e2;
					}
					if (is_new_solution(e2))
					{
						should_exit = true;
						break;
					}
				}
			}

			//A-B
			if (exxp1->value > exxp2->value)
			{
				if (exxp2->priority != 1)
				{
					if (!is_associative(exxp1, exxp2, minus))
					{
						auto e3 = new Expression(exxp1, exxp2, minus);
						if(offset == -1){
							omp_set_lock(exp3_lock);
							exp3.push_back(e3);
							omp_unset_lock(exp3_lock);
						}
						else{
							size_t pos = (size_t)offset + (size_t)i * (size_t)exp2.size() * 8 + (size_t)j * 8 + 4;
							exp3[pos] = e3;
						}
						if (is_new_solution(e3))
						{
							should_exit = true;
							break;
						}
					}
				}
			}
			//B-A, ako je B>A
			else if (exxp2->value > exxp1->value)
			{
				if (exxp1->priority != 1)
				{
					if (!is_associative(exxp2, exxp1, minus))
					{
						auto e3 = new Expression(exxp2, exxp1, minus);
						if(offset == -1){
							omp_set_lock(exp3_lock);
							exp3.push_back(e3);
							omp_unset_lock(exp3_lock);
						}
						else{
							size_t pos = (size_t)offset + (size_t)i * (size_t)exp2.size() * 8 + (size_t)j * 8 + 5;
							exp3[pos] = e3;
						}
						if (is_new_solution(e3))
						{
							should_exit = true;
							break;
						}
					}
				}
			}

			//A/B
			if (exxp2->value != 1 && exxp2->priority != 2)
			{
				if (!is_associative(exxp1, exxp2, divide))
				{
					auto e4 = new Expression(exxp1, exxp2, divide);
					if(offset == -1){
						omp_set_lock(exp3_lock);
						exp3.push_back(e4);
						omp_unset_lock(exp3_lock);
					}
					else{
						size_t pos = (size_t)offset + (size_t)i * (size_t)exp2.size() * 8 + (size_t)j * 8 + 6;
						exp3[pos] = e4;
					}
					if (is_new_solution(e4))
					{
						should_exit = true;
						break;
					}
				}
			}
			//B/A
			if (exxp1->value != 1 && exxp1->priority != 2)
			{
				if (!is_associative(exxp2, exxp1, divide))
				{
					auto e4 = new Expression(exxp2, exxp1, divide);
					if(offset == -1){
						omp_set_lock(exp3_lock);
						exp3.push_back(e4);
						omp_unset_lock(exp3_lock);
					}
					else{
						size_t pos = (size_t)offset + (size_t)i * (size_t)exp2.size() * 8 + (size_t)j * 8 + 7;
						exp3[pos] = e4;
					}
					
					if (is_new_solution(e4))
					{
						should_exit = true;
						break;
					}
				}
			}
		}
	}
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
}
