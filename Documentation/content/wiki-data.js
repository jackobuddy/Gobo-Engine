window.GOBO_DESIGNER_WIKI={groups:[{title:'Getting started',pages:['home','template','lifecycle','first-playable']},{title:'Your game',pages:['igame','config','context']},{title:'Gameplay tools',pages:['entities','systems','assets','input','collision','camera']},{title:'Presentation',pages:['ui-audio']},{title:'Reference',pages:['limits']}],pages:[
{id:'home',section:'Designer documentation',title:'Gobo Engine Designer Guide',summary:'Build your game from the supplied template.',content:`<div class="notice">This guide is for people making games with Gobo Engine. It describes the game project you create from <code>Projects/GoboGameTemplate</code>, not changing Engine source.</div><h2>Start here</h2><ol><li>Clone the suite, run its setup script, and build it.</li><li>Create your game from <code>Projects/GoboGameTemplate</code> using the project-creation script.</li><li>Put your game code in the new project's <code>src/Game.h</code> and <code>src/Game.cpp</code>.</li><li>Use the lifecycle callbacks to configure and run your game.</li></ol><p>The template main file creates your Game and passes it to Application. You normally do not modify Engine Application code.</p>`},
{id:'template',section:'Getting started',title:'Set up a game from the template',summary:'Create a game project from GoboGameTemplate.',content:`<p>From the suite root, create a project with a valid name:</p><pre><code>.\New-GoboProject.ps1 -Name MyGame
cd Projects\MyGame
.\scripts\bootstrap.ps1
cmake --build --preset windows-x64-debug</code></pre><p>The new-project script copies <code>Projects/GoboGameTemplate</code>, clears its old build output, and replaces the template project name in its CMake and vcpkg files. Its checked-in location is the suite root. The game bootstrap script configures the Windows x64 preset against <code>../../Engine</code>. Visual Studio 2022 with Desktop development with C++ and C++ CMake tools is the intended setup.</p><p>Edit <code>src/Game.h</code> and <code>src/Game.cpp</code>; add new game source files to the copied template's CMakeLists.txt.</p>`},
{id:'lifecycle',section:'Getting started',title:'When your game code runs',summary:'The callback order for starting, frames, and stopping.',content:`<pre><code>Configure → engine starts → Start
each frame: BeginFrame → input → Update → entity changes apply → clear → Render
Stop → engine shuts down</code></pre><p>Configure runs before the window exists. Start runs once when services are ready. BeginFrame runs before input, so subscribe to events there. Update receives seconds since the previous frame. Render runs after clearing. Stop runs once during shutdown. Entity changes become visible after Update.</p>`},
{id:'igame',section:'Your game',title:'IGame: the game you write',summary:'Override lifecycle callbacks in the template Game class.',content:`<p>Your <code>Game</code> derives from <code>IGame</code>. Every callback has an empty default, so override only what you need.</p><pre><code>GameConfig Configure() const override;
void Start(GameContext&amp;) override;
void BeginFrame(GameContext&amp;) override;
void Update(GameContext&amp;, float deltaSeconds) override;
void Render(GameContext&amp;) override;
void Stop(GameContext&amp;) override;</code></pre><p>Use Configure for settings, Start for initial assets/entities, BeginFrame for input subscriptions, Update for game logic, Render for drawing, and Stop for cleanup.</p>`},
{id:'config',section:'Your game',title:'GameConfig: choose game settings',summary:'Return window, camera, color, and frame settings from Configure.',content:`<pre><code>GameConfig Game::Configure() const {
  GameConfig config;
  config.title = "My Gobo Game";
  config.window.windowWidth = 1280;
  config.window.windowHeight = 720;
  config.render.renderWidth = 1280;
  config.render.renderHeight = 720;
  config.clearColor = {30, 45, 70, 255};
  return config;
}</code></pre><p>Defaults: title Untitled Gobo Game; window 1920×1080 windowed; render view 1280×720 letterboxed; camera at (0,0); VSync on; frame cap on at 60 FPS; clear color (21,21,21,255). <code>targetFPS</code> must be greater than zero.</p>`},
{id:'context',section:'Your game',title:'GameContext: Engine services',summary:'Use the context parameter supplied to game callbacks.',content:`<p>GameContext provides <code>GetWorld()</code>, <code>GetAssets()</code>, <code>GetEvents()</code>, <code>GetCamera()</code>, <code>GetRenderer()</code>, <code>GetSDLRenderer()</code>, and <code>RequestQuit()</code>.</p><p>Use the World for entities; Assets for textures; Events for key subscriptions; Camera for the view; and RequestQuit to end cleanly. These references belong to the Engine: never delete them or retain them after Stop.</p>`},
{id:'entities',section:'Gameplay tools',title:'Entities and components',summary:'Create game objects and add their data.',content:`<pre><code>auto&amp; world = context.GetWorld();
Entity item = world.CreateEntity();
item.AddComponent&lt;TransformComponent&gt;(Vector2(200, 120));
item.AddComponent&lt;RigidBodyComponent&gt;(Vector2(80, 0));
item.AddComponent&lt;SpriteComponent&gt;("coin", 16, 16);</code></pre><p>A Transform stores position; a RigidBody stores velocity; a Sprite identifies a texture. Actor and Character are convenience types: <code>world.CreateEntity&lt;Actor&gt;(config)</code> adds transform and movement data; Character also adds sprite, collision, animation, and health data.</p><div class="notice warning">Tags and groups are early tools. Give an entity one stable tag and one stable group at creation; do not rely on retagging, regrouping, or unknown-name lookup.</div>`},
{id:'assets',section:'Gameplay tools',title:'Textures and sprites',summary:'Load once and refer to a texture by ID.',content:`<pre><code>void Game::Start(GameContext&amp; context) {
  context.GetAssets().AddTexture(
    context.GetSDLRenderer(), "hero", "assets/hero.png");
}</code></pre><p>AddTexture returns false when loading fails or an ID is already used. Use the ID on SpriteComponent. Missing texture IDs are skipped and logged. Fixed sprites stay on screen while normal sprites move with the camera. AnimationComponent expects horizontal sprite-sheet frames.</p>`},
{id:'input',section:'Gameplay tools',title:'Keyboard input',summary:'Subscribe to key presses during BeginFrame.',content:`<pre><code>void Game::BeginFrame(GameContext&amp; context) {
  context.GetEvents().SubscribeToEvent&lt;KeyPressedEvent&gt;(
    this, &amp;Game::OnKeyPressed);
}
void Game::OnKeyPressed(KeyPressedEvent&amp; event) {
  if (event.symbol == SDLK_ESCAPE) context-&gt;RequestQuit();
}</code></pre><p>Subscriptions reset each frame before BeginFrame, so register them each frame. The callback owner must remain alive while subscribed.</p>`},
{id:'limits',section:'Reference',title:'Current limits',summary:'Important boundaries of the current designer-facing API.',content:`<ul><li>Windows and the supplied CMake presets are the supported workflow.</li><li>Projectiles, health, tags, groups, and sample behavior are early gameplay tools.</li><li>Direct SDL drawing is available, but is a lower-level C++ option.</li><li>Vector2 is the normal 2D value type. Avoid unchecked indexes and division by zero.</li></ul>`}
]};
