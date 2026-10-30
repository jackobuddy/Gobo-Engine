# Gobo Engine Documentation - Standing Instructions

Last updated: 2026-10-30 CDT

## Authority and boundaries

- The documentation team may edit only the suite-root `docs/Documentation` Engineering Wiki directory, suite-root `Documentation` Designer Guide directory, and suite-root `README.md`.
- Never edit Engine source, build files, dependencies, assets, generated output, the Game Template, test games, or another department's files.
- Treat `Projects/GoboGameTemplate` as read-only reference material. It shows the supported designer-facing integration path.

## Two documentation audiences

- Maintain `docs/Documentation` as the Engineering wiki. It documents Engine internals for programmers maintaining the Engine.
- Maintain this `Documentation` site as the Designer wiki. It documents only the public, game-author-facing surface: creating a game from the template, `IGame`, `GameConfig`, `GameContext`, Scene workflow, and supported gameplay-facing Engine types.
- Use clear everyday language. Explain technical words when needed. Include short, working-context examples whenever they help a designer get started.
- Do not expose internal ownership, raw SDL setup, implementation-only system mechanics, or unfinished APIs as normal designer workflows.
- Never link from the Designer Guide to the Engineering Wiki. Keep the designer learning path self-contained.
- Explain every EntityObject callback in the designer reference: exact signature, required components/system, when it fires, event fields, practical use, and common mistakes. Distinguish object callbacks, hit-tested interactions, and explicit EventManager subscriptions.
- Teach through complete, placed examples: identify the class/file/callback, setup, required artwork/config, and expected result. Use Character for the introductory player tutorial and the current template as the baseline. ExampleGame was removed; do not send readers to absent source or assume sample assets are bundled.

## Every review

1. Inspect current `Engine` source, the suite-root README, and suite CMake/scripts.
2. Inspect `Projects/GoboGameTemplate` source, CMake files, and setup script to verify the intended user workflow.
3. Refresh both wikis and the suite README for changed public behavior, scene/lifecycle order, templates, configuration, object callbacks, asset staging/import behavior, and examples. Treat current Engine headers and implementations as the source of truth when a template or older article has not yet caught up.
4. Clearly label prototype, smoke-test, or unavailable features and never present them as stable designer tools.
5. Keep the site directly usable from `Documentation/index.html`: no server, build, or package installation.
6. Keep markup, navigation, search, links, and examples synchronized and validate the site data before completing a review.
7. Treat department feedback as a useful lead, not a substitute for source verification; reports can describe an earlier build while the Engine changes rapidly.
8. Recheck source changes reported during a review before keeping a limitation in the guide. Remove completed or no-longer-applicable QA queue entries only after verification.
9. Keep one canonical definition per wiki page in content/wiki-data.js. Do not accumulate historical overrides or leave obsolete instructions searchable. Retain a small compatibility comment in wiki-data-extra.js while existing entry points load it.
10. Verify callback coverage against EntityObject.h, and trace system subscriptions/dispatch instead of assuming a declared hook is connected. Check search against function names inside article text, plus links, navigation, and mobile readability.
11. Cover maps, the optional Map Editor, packed sprite staging, modules, pause policies, alarms, solid movement, and text input as they evolve. Explain editor-only settings separately from runtime behavior. Distinguish implemented modules from placeholders.
12. Maintain the README's MIT license summary and links to the actual Engine and third-party license files. Verify packaging notice paths against the current script; do not modify license texts or claim that Engine licensing covers game-owned assets.

## Site organization

- `index.html` is the browser entry point.
- `assets/` contains only site styles and behavior.
- `content/` contains searchable article data.
- `department/` contains these instructions and any documentation-team records.
- Organize content into focused wiki pages rather than placing many files in the root Documentation directory.
