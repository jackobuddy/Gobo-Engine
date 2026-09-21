# Gobo Engine Documentation - Standing Instructions

Last updated: 2026-09-21 CDT

## Authority and boundaries

- The documentation team may edit only the suite-root `docs/Documentation` Engineering Wiki directory, suite-root `Documentation` Designer Guide directory, and suite-root `README.md`.
- Never edit Engine source, build files, dependencies, assets, generated output, the Game Template, test games, or another department's files.
- Treat `Projects/GoboGameTemplate` as read-only reference material. It shows the supported designer-facing integration path.

## Two documentation audiences

- Maintain `docs/Documentation` as the Engineering wiki. It documents Engine internals for programmers maintaining the Engine.
- Maintain this `Documentation` site as the Designer wiki. It documents only the public, game-author-facing surface: creating a game from the template, `IGame`, `GameConfig`, `GameContext`, and supported gameplay-facing Engine types.
- Use clear everyday language. Explain technical words when needed. Include short, working-context examples whenever they help a designer get started.
- Do not expose internal ownership, raw SDL setup, implementation-only system mechanics, or unfinished APIs as normal designer workflows.

## Every review

1. Inspect current `Engine` source, the suite-root README, and suite CMake/scripts.
2. Inspect `Projects/GoboGameTemplate` source, CMake files, and setup script to verify the intended user workflow.
3. Refresh both wikis and the suite README for changed public behavior, lifecycle order, templates, configuration, object callbacks, and examples.
4. Clearly label prototype, smoke-test, or unavailable features and never present them as stable designer tools.
5. Keep the site directly usable from `Documentation/index.html`: no server, build, or package installation.
6. Keep markup, navigation, search, links, and examples synchronized and validate the site data before completing a review.

## Site organization

- `index.html` is the browser entry point.
- `assets/` contains only site styles and behavior.
- `content/` contains searchable article data.
- `department/` contains these instructions and any documentation-team records.
- Organize content into focused wiki pages rather than placing many files in the root Documentation directory.
