from typing import Dict, Set

from llm_sdk import Small_LLM_Model


def build_digit_tokens(model: Small_LLM_Model, vocab_size: int) -> Dict[str, int]:
    chars = "0123456789.-+"
    mapping: Dict[str, int] = {}
    for tid in range(vocab_size):
        s = model.decode([tid])
        if s in chars:
            mapping[s] = tid
    return mapping


def build_quote_tokens(model: Small_LLM_Model, vocab_size: int) -> Set[int]:
    quote_ids: Set[int] = set()
    for tid in range(vocab_size):
        if '"' in model.decode([tid]):
            quote_ids.add(tid)
    return quote_ids
