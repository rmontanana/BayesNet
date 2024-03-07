SHELL := /bin/bash
.DEFAULT_GOAL := help
.PHONY: coverage setup help buildr buildd test clean debug release sample

f_release = build_release
f_debug = build_debug
app_targets = BayesNet
test_targets = unit_tests_bayesnet
n_procs = -j 16

define ClearTests
	@for t in $(test_targets); do \
		if [ -f $(f_debug)/tests/$$t ]; then \
			echo ">>> Cleaning $$t..." ; \
			rm -f $(f_debug)/tests/$$t ; \
		fi ; \
	done
	@nfiles="$(find . -name "*.gcda" -print0)" ; \
	if test "${nfiles}" != "" ; then \
		find . -name "*.gcda" -print0 | xargs -0 rm 2>/dev/null ;\
	fi ; 
endef


setup: ## Install dependencies for tests and coverage
	@if [ "$(shell uname)" = "Darwin" ]; then \
		brew install gcovr; \
		brew install lcov; \
	fi
	@if [ "$(shell uname)" = "Linux" ]; then \
		pip install gcovr; \
	fi

dependency: ## Create a dependency graph diagram of the project (build/dependency.png)
	@echo ">>> Creating dependency graph diagram of the project...";
	$(MAKE) debug
	cd $(f_debug) && cmake .. --graphviz=dependency.dot && dot -Tpng dependency.dot -o dependency.png

buildd: ## Build the debug targets
	cmake --build $(f_debug) -t $(app_targets) $(n_procs)

buildr: ## Build the release targets
	cmake --build $(f_release) -t $(app_targets) $(n_procs)

clean: ## Clean the tests info
	@echo ">>> Cleaning Debug BayesNet tests...";
	$(call ClearTests)
	@echo ">>> Done";

install: ## Install library
	@echo ">>> Installing BayesNet...";
	@cmake --install $(f_release)
	@echo ">>> Done";

debug: ## Build a debug version of the project
	@echo ">>> Building Debug BayesNet...";
	@if [ -d ./$(f_debug) ]; then rm -rf ./$(f_debug); fi
	@mkdir $(f_debug); 
	@cmake -S . -B $(f_debug) -D CMAKE_BUILD_TYPE=Debug -D ENABLE_TESTING=ON -D CODE_COVERAGE=ON
	@echo ">>> Done";

release: ## Build a Release version of the project
	@echo ">>> Building Release BayesNet...";
	@if [ -d ./$(f_release) ]; then rm -rf ./$(f_release); fi
	@mkdir $(f_release); 
	@cmake -S . -B $(f_release) -D CMAKE_BUILD_TYPE=Release
	@echo ">>> Done";

fname = "tests/data/iris.arff"
sample: ## Build sample
	@echo ">>> Building Sample...";
	cmake --build $(f_release) -t bayesnet_sample $(n_procs)
	$(f_release)/sample/bayesnet_sample $(fname)
	@echo ">>> Done";	

opt = ""
test: ## Run tests (opt="-s") to verbose output the tests, (opt="-c='Test Maximum Spanning Tree'") to run only that section
	@echo ">>> Running BayesNet & Platform tests...";
	@$(MAKE) clean
	@cmake --build $(f_debug) -t $(test_targets) $(n_procs)
	@for t in $(test_targets); do \
		if [ -f $(f_debug)/tests/$$t ]; then \
			cd $(f_debug)/tests ; \
			./$$t $(opt) ; \
		fi ; \
	done
	@echo ">>> Done";

coverage: ## Run tests and generate coverage report (build/index.html)
	@echo ">>> Building tests with coverage..."
	@$(MAKE) test
	@gcovr $(f_debug)/tests
	@echo ">>> Done";	


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
