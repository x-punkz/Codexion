import os
import sys
import json
from typing import List, Union, Dict, Literal
from pydantic import BaseModel, ValidationError


class PromptItem(BaseModel):
    prompt: str


class Parameter(BaseModel):
    type: Literal["number", "string"]


class FunctionItem(BaseModel):
    name: str
    description: str
    parameters: Dict[str, Parameter]
    returns: Parameter | None = None


def load_json_file(path: str) -> List[Union[PromptItem, FunctionItem]]:
    with open(path, "r", encoding="utf-8") as file:
        data = json.load(file)

    if not isinstance(data, list):
        raise ValueError("[Error] Input JSON must be an array.")

    result: List[Union[PromptItem, FunctionItem]] = []
    for item in data:
        if not isinstance(item, dict):
            raise ValueError(f"[Error] Array element is not an object: {item!r}")
        if "prompt" in item:
            result.append(PromptItem(**item))
        elif "name" in item:
            result.append(FunctionItem(**item))
        else:
            raise ValueError(f"[Error] Unknown format: {item}")

    return result


def safe_load_json_file(path: str) -> List[Union[PromptItem, FunctionItem]]:
    try:
        return load_json_file(path)
    except FileNotFoundError:
        print(f"[Error] File not found: {path}", file=sys.stderr)
    except json.JSONDecodeError as je:
        print(f"[Error] Invalid JSON in {path}:\n{je}", file=sys.stderr)
    except ValidationError as vlde:
        print(f"[Error] Schema validation failed in {path}:\n{vlde}", file=sys.stderr)
    except ValueError as ve:
        print(f"[Error] {ve}", file=sys.stderr)
    return []


def write_results(path: str, results: List[dict]) -> bool:
    try:
        directory = os.path.dirname(path)
        if directory:
            os.makedirs(directory, exist_ok=True)
        with open(path, "w", encoding="utf-8") as file:
            json.dump(results, file, indent=2, ensure_ascii=False)
        return True
    except OSError as error:
        print(f"[Error] Error writing output to {path}: {error}", file=sys.stderr)
        return False
