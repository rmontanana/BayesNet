SHELL := /bin/bash
.DEFAULT_GOAL := help
.PHONY: coverage setup help build test

setup: ## Install dependencies for tests and coverage
	@if [ "$(shell uname)" = "Darwin" ]; then \
		brew install gcovr; \
		brew install lcov; \
	fi
	@if [ "$(shell uname)" = "Linux" ]; then \
		pip install gcovr; \
	fi

dependency: ## Create a dependency graph diagram of the project (build/dependency.png)
	cd build && cmake .. --graphviz=dependency.dot && dot -Tpng dependency.dot -o dependency.png

debug: ## Build the project
	@echo ">>> Building Debug BayesNet ...";
	@if [ -d ./build ]; then rm -rf ./build; fi
	@mkdir build; 
	cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug -D ENABLE_TESTING=ON -D CODE_COVERAGE=ON; \
	cmake --build build -j 32;
	@echo ">>> Done";

release:
	@echo ">>> Building Release BayesNet ...";
	@if [ -d ./build ]; then rm -rf ./build; fi
	@mkdir build; 
	cmake -S . -B build -D CMAKE_BUILD_TYPE=Release; \
	cmake --build build -t main -j 32;
	@echo ">>> Done";	

test: ## Run tests
	@echo "* Running tests...";
	find . -name "*.gcda" -print0 | xargs -0 rm
	@cd build; \
	cmake --build . --target unit_tests ;
	@cd build/tests; \
	./unit_tests;

coverage: ## Run tests and generate coverage report (build/index.html)
	@echo "*Building tests...";
	find . -name "*.gcda" -print0 | xargs -0 rm
	@cd build; \
	cmake --build . --target unit_tests ;
	@cd build/tests; \
	./unit_tests;
	gcovr ;

help: ## Show help message
	@IFS=$$'\n' ; \
	help_lines=(`fgrep -h "##" $(MAKEFILE_LIST) | fgrep -v fgrep | sed -e 's/\\$$//' | sed -e 's/##/:/'`); \
	printf "%s\n\n" "Usage: make [task]"; \
	printf "%-20s %s\n" "task" "help" ; \
	printf "%-20s %s\n" "------" "----" ; \
	for help_line in $${help_lines[@]}; do \
		IFS=$$':' ; \
		help_split=($$help_line) ; \
		help_command=`echo $${help_split[0]} | sed -e 's/^ *//' -e 's/ *$$//'` ; \
		help_info=`echo $${help_split[2]} | sed -e 's/^ *//' -e 's/ *$$//'` ; \
		printf '\033[36m'; \
		printf "%-20s %s" $$help_command ; \
		printf '\033[0m'; \
		printf "%s\n" $$help_info; \
	done
