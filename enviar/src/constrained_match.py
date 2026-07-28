from typing import Any, Dict, List, Set

from llm_sdk import Small_LLM_Model

from .json_utils import FunctionItem
from .decoding import generate_number, generate_string


def build_selection_context(prompt: str, functions: List[FunctionItem]) -> str:
    lines = ["Available functions:"]
    for f in functions:
        lines.append(f"- {f.name}: {f.description}")
    lines.append(f'User request: "{prompt}"')
    lines.append("The function to call is:")
    return "\n".join(lines) + " "


def select_function_name(
    model: Small_LLM_Model,
    prompt: str,
    functions: List[FunctionItem],
) -> str:
    NEG_INF = float("-inf")

    context = build_selection_context(prompt, functions)
    context_ids = model.encode(context).tolist()[0]

    function_names = [f.name for f in functions]
    name_ids = [model.encode(" " + n).tolist()[0] for n in function_names]

    generated: List[int] = []
    candidates = name_ids.copy()

    while True:
        next_valid_tokens = {
            seq[len(generated)]
            for seq in candidates
            if len(generated) < len(seq) and seq[:len(generated)] == generated
        }
        if not next_valid_tokens:
            break
        logits = model.get_logits_from_input_ids(context_ids + generated)
        masked = [
            logit if token in next_valid_tokens else NEG_INF
            for token, logit in enumerate(logits)
        ]
        next_token = max(range(len(masked)), key=lambda i: masked[i])
        generated.append(next_token)
        candidates = [
            seq for seq in candidates if seq[:len(generated)] == generated
        ]
        if len(candidates) == 1 and len(generated) == len(candidates[0]):
            break

    name = model.decode(generated).strip()
    return name if name in function_names else function_names[0]


def build_base_context(prompt: str, function: FunctionItem) -> str:
    return (
        f'User request: "{prompt}"\n'
        f'Available function: {function.name} — {function.description}\n'
        f'Extract the arguments as a JSON object:\n'
    )


def extract_arguments(
    model: Small_LLM_Model,
    prompt: str,
    function: FunctionItem,
    digit_tokens: Dict[str, int],
    quote_tokens: Set[int],
) -> Dict[str, Any]:
    args: Dict[str, Any] = {}
    base = build_base_context(prompt, function)
    json_so_far = "{"
    params = list(function.parameters.items())

    for i, (param_name, spec) in enumerate(params):
        json_so_far += f'"{param_name}": '
        context_ids = model.encode(base + json_so_far).tolist()[0]

        if spec.type == "number":
            value: Any = generate_number(model, context_ids, digit_tokens)
            json_so_far += str(value)
        elif spec.type == "string":
            string_context = base + json_so_far + '"'
            context_ids = model.encode(string_context).tolist()[0]
            value = generate_string(model, context_ids, quote_tokens)
            json_so_far += f'"{value}"'
        else:
            value = None
            json_so_far += "null"

        args[param_name] = value
        if i < len(params) - 1:
            json_so_far += ", "

    return args


def constrained_function_call(
    model: Small_LLM_Model,
    prompt: str,
    functions: List[FunctionItem],
    digit_tokens: Dict[str, int],
    quote_tokens: Set[int],
) -> Dict[str, Any]:
    name = select_function_name(model, prompt, functions)
    function = next(f for f in functions if f.name == name)
    parameters = extract_arguments(
        model, prompt, function, digit_tokens, quote_tokens
    )
    return {"prompt": prompt, "name": name, "parameters": parameters}
