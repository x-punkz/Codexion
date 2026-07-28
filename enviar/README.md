*This project has been created as part of the 42 curriculum by daniviei.*

# Call Me Maybe

## Description

**Call Me Maybe** is a function-calling tool. Given a request written in
natural language (for example, *"What is the sum of 2 and 3?"*), it does not
answer the question. Instead, it decides **which function to call** and **with
which arguments**, and produces a structured JSON object describing that call:

```json
{
  "prompt": "What is the sum of 2 and 3?",
  "name": "fn_add_numbers",
  "parameters": {"a": 2.0, "b": 3.0}
}
```

The project uses the small **Qwen/Qwen3-0.6B** model (600 million parameters).
Small models are notoriously unreliable at producing valid JSON on their own.
The goal of this project is to reach near-perfect reliability anyway, using
**constrained decoding**: instead of trusting the model to format its output,
we steer its generation one token at a time so the result is, by construction,
always valid JSON that follows the requested schema.

## Algorithm explanation

The core idea is to never rely on the model spontaneously producing correct
JSON. The model generates text one token at a time; at each step it produces a
logit (a raw score) for every token in its vocabulary. Constrained decoding
intervenes at this point:

1. Get the raw logits from the model for the next token.
2. Determine which tokens are valid at this step (they keep both the JSON
   structure and the schema valid).
3. Set the logits of all invalid tokens to negative infinity.
4. Pick the highest-scoring remaining token (an argmax). Because the invalid
   tokens are at minus infinity, the chosen token is always a valid one.

This guarantees structurally valid output regardless of how weak the model is.

The pipeline has three stages:

**1. Reading and validating input.** Two files are read: the prompts and the
function definitions. They are modelled with **pydantic** (`PromptItem`,
`FunctionItem`, `Parameter`). Malformed JSON, a missing file, or an input that
is not a JSON array are all handled gracefully with a clear message and never
crash the program.

**2. Function selection.** A context is built that lists every available
function together with its description, followed by the user request. The model
then generates a function name restricted to the set of valid names. Internally,
a list of candidate names is kept; at each step the valid next tokens are
collected from the candidates, the logits are masked accordingly, the best token
is chosen, and candidates that no longer match are pruned. Including the function
**descriptions** in the context is what makes the selection accurate, because the
model can reason about what each function does rather than guessing from the name
alone.

**3. Argument extraction.** The JSON object is built as a growing string
(`{"a": ` then the value, then `, "b": ` then the value, and so on). For each
parameter, a dedicated value generator is used depending on its type:

- `generate_number` generates the number character by character (digits, an
  optional sign, a single dot). A small state machine decides which characters
  are valid at each step (digits are always allowed; a dot only if there is no
  dot yet; a sign only at the start). It stops when the model, left free, would
  pick a non-numeric token, signalling that the number is finished.
- `generate_string` generates free content until the model would pick a closing
  quote. The opening quote is structural (we add it), the content comes from the
  model, and the closing quote signals the end. Quote tokens are masked out of
  the content so a stray quote can never break the JSON.

The text that precedes generation is essential: the same machinery produces `2`
or `5` depending on the context. By ending the context exactly where the value
should appear (for instance at `{"name": "`), the model is steered to *extract*
the right value from the prompt rather than to *answer* the request.

## Instructions

The project is managed with `uv` and run as a Python package.

Install dependencies (the reviewer only needs this):

```
uv sync
```

Run the program:

```
uv run python -m src
```

By default it reads from `data/input/` and writes to `data/output/`. Custom
paths can be given:

```
uv run python -m src \
    --functions_definition data/input/functions_definition.json \
    --input data/input/function_calling_tests.json \
    --output data/output/function_calling_results.json
```

A `Makefile` automates the common tasks: `make install`, `make run`,
`make debug`, `make clean`, `make lint`.

> **Note on heavy dependencies.** The `llm_sdk` pulls in `torch`, `transformers`
> and `huggingface-hub`, and the Qwen model is downloaded on first use. In
> environments with a small disk quota, point the caches to a volume with space,
> for example with `UV_CACHE_DIR` and `HF_HOME`.

## Example usage

```
$ uv run python -m src
{'prompt': 'What is the sum of 2 and 3?', 'name': 'fn_add_numbers', 'parameters': {'a': 2.0, 'b': 3.0}}
{'prompt': 'Greet shrek', 'name': 'fn_greet', 'parameters': {'name': 'shrek'}}
{'prompt': "Reverse the string 'hello'", 'name': 'fn_reverse_string', 'parameters': {'s': 'hello'}}
Wrote 3 results to data/output/function_calling_results.json
```

## Design decisions

- **Constrained decoding instead of prompting.** The whole project rests on
  masking logits, not on asking the model nicely for JSON. This is the skill the
  project exists to develop, and the only way to guarantee valid output from a
  small model.
- **Value-by-value generation** rather than generating the whole JSON in one
  pass. Each value generator is small, testable in isolation and easy to reason
  about; the orchestrator stitches the structure around them.
- **A growing context string.** The partial JSON is built incrementally, so when
  a later parameter is generated the model already sees the earlier values and
  does not repeat them.
- **Stopping rule based on the model's free choice.** A value ends when the
  model, with no constraint applied, would leave the value type. This keeps the
  generators decoupled from the surrounding structure.
- **Single numeric characters only.** The numeric token set is restricted to
  single characters, so numbers are generated digit by digit. This trades a
  little speed for trivially correct validation.
- **`returns` is optional in the model.** The program does not use the return
  type, so a function definition without it is still usable. This favours
  robustness, since the subject warns that the input files may change.
- **Model independence.** The vocabulary size is discovered at runtime and token
  ids are rediscovered via `decode`, so the project is not hard-wired to a single
  model.
- **mypy is run on `src`.** The provided `llm_sdk` is an external dependency we
  do not own, so type-checking is scoped to our own code (with
  `--follow-imports=silent`).

## Performance analysis

- **JSON validity: 100%.** Every output is parseable and schema-compliant by
  construction, which is the central guarantee of constrained decoding.
- **Accuracy.** Function selection and argument extraction are correct on the
  large majority of the provided tests, meeting the expected bar. Including the
  function descriptions in the selection context was the change that lifted
  accuracy the most.
- **Semantic quality depends on the model.** Structure is always valid, but the
  *meaning* of a value relies on a 0.6B model and degrades on ambiguous or
  degenerate prompts (for example, a complex regex may come out slightly
  malformed, or an empty name may be invented). This is an inherent limit of the
  model size, not of the decoding.
- **Speed.** All test prompts are processed well under the five-minute limit. The
  model is loaded once and the numeric/quote token sets are computed once and
  reused across prompts.

## Challenges faced

- **Disk quota on the campus (the biggest one).** The dependency stack (`torch`,
  `transformers`, the Qwen weights) is large, and `uv sync` repeatedly failed
  with *"No space left on device"* because the cache was landing in the home
  directory. The fix was to move the project to `sgoinfre` and redirect
  `UV_CACHE_DIR` and `HF_HOME` to an absolute path there. A subtle trap was that
  `~/sgoinfre` resolved to the home directory, while the real space was at
  `/sgoinfre/<login>`.
- **Byte-level BPE and the space token.** The tokenizer embeds a leading space in
  the token, so a function name after `"is: "` uses a different token id than the
  same name in isolation. Encoding names with a leading space (`" " + name`) was
  needed for the mask to allow the token the model actually wants.
- **The missing opening quote.** String values first came out as `1,` because the
  context did not yet contain the opening quote; the model saw `{"name": ` and
  generated a number. Adding the structural opening quote to the context before
  generating the content fixed it.
- **Unstable selection without descriptions.** Passing only the function names to
  the model made it choose wrong on non-obvious prompts; adding the descriptions
  to the selection context resolved this.

## Testing strategy

Testing was done by running the program against crafted inputs and checking that
the output is always valid JSON and that the program never crashes. The cases
covered include: a normal run, missing input files, malformed JSON in either
input file, an input that is valid JSON but not an array, an empty array, a
nonsense prompt that matches no function, and edge cases such as very large
numbers and degenerate strings. The exact commands are kept in `tests.txt`.

## Resources

- Qwen3 model card and documentation (Hugging Face).
- The project subject and the provided `llm_sdk`.
- General references on byte-pair encoding (BPE) tokenization and on constrained
  decoding for structured generation.

**Use of AI.** AI (Claude) was used throughout this project as a learning and
development aid: to explain concepts (constrained decoding, byte-level BPE, the
generation pipeline), to review code and point out bugs.
