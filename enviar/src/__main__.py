import argparse

from llm_sdk import Small_LLM_Model

from .json_utils import safe_load_json_file, write_results, PromptItem, FunctionItem
from .tokens import build_digit_tokens, build_quote_tokens
from .constrained_match import constrained_function_call


def main() -> None:
    parser = argparse.ArgumentParser(
        prog="python -m src",
        description="Translate natural language prompts into structured function calls.",
    )
    parser.add_argument(
        "--functions_definition",
        type=str,
        default="data/input/functions_definition.json",
        help="Path to the functions definition file.",
    )
    parser.add_argument(
        "--input",
        type=str,
        default="data/input/function_calling_tests.json",
        help="Path to the input prompts file.",
    )
    parser.add_argument(
        "--output",
        type=str,
        default="data/output/function_calling_results.json",
        help="Path to the output file.",
    )
    args = parser.parse_args()

    prompts = [
        item for item in safe_load_json_file(args.input)
        if isinstance(item, PromptItem)
    ]
    functions = [
        item for item in safe_load_json_file(args.functions_definition)
        if isinstance(item, FunctionItem)
    ]

    if not prompts or not functions:
        print("No prompts or no function definitions to process.")
        return

    model = Small_LLM_Model()
    vocab_size = len(model.get_logits_from_input_ids(model.encode("a").tolist()[0]))
    digit_tokens = build_digit_tokens(model, vocab_size)
    quote_tokens = build_quote_tokens(model, vocab_size)

    results = []
    for prompt_item in prompts:
        result = constrained_function_call(
            model, prompt_item.prompt, functions, digit_tokens, quote_tokens
        )
        results.append(result)

    if write_results(args.output, results):
        print(f"Wrote {len(results)} results to {args.output}")


if __name__ == "__main__":
    main()
