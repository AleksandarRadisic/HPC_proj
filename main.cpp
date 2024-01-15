#include "expression_builder.h"

#include <chrono>
#include <iostream>
#include <ostream>

int main()
{
	{
		std::cout << std::endl << "---------PARALLEL----------" << std::endl;
		Expression_builder builder_parallel(82, std::vector<int>{4, 5, 6, 8, 10, 25, 5});
		builder_parallel.find_all = true;
		auto start_time_parallel = std::chrono::high_resolution_clock::now();
		builder_parallel.start_build_parallel();
		auto end_time_parallel = std::chrono::high_resolution_clock::now();
		auto duration_parallel = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_parallel - start_time_parallel);
		//builder_parallel.print_all_solutions();
		std::cout << "Total solutions: " << builder_parallel.solutions.size() << std::endl;
		std::cout << "Time taken by code PARALLEL: " << duration_parallel.count() << " milliseconds" << std::endl;
		std::cout << "Equations calculated PARALLEL: " << builder_parallel.total <<std::endl;
	}
	

	{
		std::cout << "---------SEQUENTIAL----------" << std::endl;
		Expression_builder builder(82, std::vector<int>{4, 5, 6, 8, 10, 25, 5});
		builder.find_all = true;
		auto start_time = std::chrono::high_resolution_clock::now();
		builder.start_build_seq();
		auto end_time = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
		//builder.print_all_solutions();
		std::cout << "Total solutions: " << builder.solutions.size() << std::endl;
		std::cout << "Time taken by code SEQUENTIAL: " << duration.count() << " milliseconds" << std::endl;
		std::cout << "Equations calculated SEQUENTIAL: " << builder.total;
	}

	
}

//(374, std::vector<int>{3, 5, 6, 9, 10, 25, 5});
//(735, std::vector<int>{2, 6, 7, 8, 10, 50, 5});
//(654, std::vector<int>{2, 5, 7, 9, 10, 75, 5});
//(82, std::vector<int>{4, 5, 6, 8, 10, 25, 5});