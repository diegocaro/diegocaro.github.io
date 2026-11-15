# diegocaro.cl

Personal static site on GitHub Pages; pushing to `main` publishes it. Content is mostly in Spanish.

## Build

- `make` renders `.md` → `.html` (pandoc, `templates/mypage.html5`) and `.shtml` → `.html` (`scripts/ssi.py`).
- An `.html` with a same-named `.md` or `.shtml` next to it is generated: never edit it, edit the source and rebuild.
- Run `make` before every commit; commit the regenerated `.html` with its source.
- New folder with `.md` files? Add it to `SRCS` in the `Makefile`.

## Content

- Any `.html` without a source file is hand-made (e.g. `ladob/diego_by_caro.html`): don't edit it unless asked.
- `ladob/` (Lado B) holds personal pieces; list each one in `ladob/index.md`.
- Keep link text and descriptions short.
