# För att köra
Skriv in gcc main.c -o my_game.exe -Iinclude -Llib -lraylib -lopengl32 -lgdi32 -lwinmm
och sen ./my_game.exe

# Vad vi ska göra
Vi ska göra en enkel Doom Clone. Målet är att låta spelaren gå runt i en bana som är visad med en texturerad raycaster. Det ska finns fiender som kan skjuta och bli skjutna av spelaren. Det ska även finnas en enkel HUD som visar spelarens hälsa och ammunition.

Om det finns tid planerar vi att implementera pickups för hälsa och ammo, ljudeffekter, egna texturer, en startmeny samt en culling-funktion för att förbättra prestanda.

Ifall vi blir klara med allt detta planerar vi att implementera förmågan att åka upp och ner samt en map creator.

# Språk och verktyg
Vi skriver i C, med grafiksbiblioteket raylib. Det ska vara kompatibelt med åtminstone Linux.


# Naming conventions

Allt skrivs på engelska. Vi använder camelcase för funktioner, små bokstäver för variabler och stora bokstäver för konstanter. Typedefs (egna datatyper) är PascalCase. Filnamn är lowercase.


Branches döps efter funktionen som ska implementeras. Issues skrivs med instruktion först och problemet efter, till exempel: “Fix: Walls won’t load”. Commits skrivs som till exempel: “Fixed: Walls won’t load” eller “Added: Raycaster”

# Workflow

Branches bör godkännas av åtminstone en annan gruppmedlem innan de mergas. Projektet börjar med att all jobbar på grunderna. Sedan delar vi upp arbetet mer ordetligt.

# Uppdelning av arbete (so far)
Ray caster 
* Raylib - Ludvig (Gick vidare till v.2)
* Logic - Dante
* Level design - Dante
* Texture -
  
Player 
* Movement - Linus
* Weapon -
  
Enemies
* Movement/Sight - 
* Attack -
  
Hud - Linus
