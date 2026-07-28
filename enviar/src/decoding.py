from typing import Dict, List, Set

from llm_sdk import Small_LLM_Model

NEG_INF = float("-inf")


def _allowed_number_chars(generated: List[str]) -> Set[str]:
    digits = set("0123456789")
    if not generated:
        return digits | {".", "-", "+"}
    allowed = set(digits)
    if "." not in generated:
        allowed.add(".")
    return allowed


def generate_number(
    model: Small_LLM_Model,
    context_ids: List[int],
    digit_tokens: Dict[str, int],
    max_len: int = 20,
) -> float:
    generated: List[str] = []
    context = context_ids.copy()
    id_to_char = {tid: ch for ch, tid in digit_tokens.items()}

    while len(generated) < max_len:
        logits = model.get_logits_from_input_ids(context)
        free_choice = max(range(len(logits)), key=lambda i: logits[i])
        has_digit = any(c.isdigit() for c in generated)
        if free_choice not in id_to_char and has_digit:
            break
        allowed_chars = _allowed_number_chars(generated)
        allowed_ids = {digit_tokens[c] for c in allowed_chars}
        masked = [
            logit if tid in allowed_ids else NEG_INF
            for tid, logit in enumerate(logits)
        ]
        next_token = max(range(len(masked)), key=lambda i: masked[i])
        generated.append(id_to_char[next_token])
        context.append(next_token)

    text = "".join(generated)
    try:
        return float(text)
    except ValueError:
        return 0.0


def generate_string(
    model: Small_LLM_Model,
    context_ids: List[int],
    quote_tokens: Set[int],
    max_len: int = 30,
) -> str:
    generated_ids: List[int] = []
    context = context_ids.copy()

    while len(generated_ids) < max_len:
        logits = model.get_logits_from_input_ids(context)
        free_choice = max(range(len(logits)), key=lambda i: logits[i])
        if free_choice in quote_tokens and generated_ids:
            break
        masked = [
            logit if tid not in quote_tokens else NEG_INF
            for tid, logit in enumerate(logits)
        ]
        next_token = max(range(len(masked)), key=lambda i: masked[i])
        generated_ids.append(next_token)
        context.append(next_token)

    return model.decode(generated_ids).strip()
