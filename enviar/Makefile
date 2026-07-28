export UV_CACHE_DIR := /sgoinfre/daniviei/my_callmemaybe/.uv_cache
export HF_HOME :="/sgoinfre/daniviei/my_callmemaybe/.hf_cache"
export TMPDIR := /sgoinfre/daniviei/.tmp

.PHONY: install run debug clean lint lint-strict

INPUT ?= data/input/function_calling_tests.json
FUNCTIONS ?= data/input/functions_definition.json
OUTPUT ?= data/output/function_calling_results.json

install:
	uv sync

run:
	uv run python -m src --functions_definition $(FUNCTIONS) --input $(INPUT) --output $(OUTPUT)

debug:
	uv run python -m pdb -m src --functions_definition $(FUNCTIONS) --input $(INPUT) --output $(OUTPUT)

clean:
	rm -rf __pycache__ */__pycache__ .mypy_cache .pytest_cache .hf_cache .uv_cache

lint:
	uv run flake8 .
	uv run mypy . --follow-imports=silent --warn-return-any --warn-unused-ignores --ignore-missing-imports --disallow-untyped-defs --check-untyped-defs

lint-strict:
	uv run flake8 .
	uv run mypy src --strict