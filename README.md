# Resultat
## Hur man kör spelet:
Öppna schlem.exe

## Vad som finns:
Spelet är nu klart och inkluderar en texturerad raycaster med olika egengjorda texturer för fiender och vägar. Det finns även tre olika fiende arketyper för spelaren att ta an, närstrid, medeldistans och långdistans. Spelaren har också tillgång till tre olika vapen, knyttnäve, k-pist och paj. Det finns även pickups som spelaren kan plocka upp för hälsa eller ammunition. Informationen om spelaren situation finns tillgänglig i en HUD, som visar hälsa, ammo och hur många fiender som finns kvar på leveln. När spelaren besegrat alla fiender så rör man sig vidare till nästa nivå, det finns totalt 4 nivåer med växande svårighetsgrad. 

Det finns även menyer för när man startar, pausar, dör eller vinner. 
- Startmenyn låter spelaren gå in i första nivån eller stänga ner spelet. 
- Pausmenyn låter spelaren stänga ner spelet, gå tillbaka till startmenyn, eller återvända till spelet. 
- Dödsmenyn låter spelaren gå tillbaka till startmenyn eller börja om.
- Vinstmenyn låter spelaren gå tillbaka till menyn eller gå vidare till nästa nivå.

## Förklaring av våra filer:
- Main: Huvudspelloopen, ritandet, menyer
- Map: Maploading, enemies, pickups
- Movement: Spelarlogik, Vapenlogik
- Raycast: Vectorlogik/matte, Raylogik
- Raylib: Ritarlogik, inputs, Spelframework (Kommer från https://www.raylib.com/index.html) 

## Hur man kompilerar spelet:
Skriv in i terminalen:
gcc main.c raycast.c map.c movement.c -o schlem.exe -Iinclude -Llib -lraylib -lopengl32 -lgdi32 -lwinmm 

(Se till att ha rätt compiler: https://winlibs.com )

---
# Planen

## Vad vi ska göra
Vi ska göra en enkel Doom Clone. Målet är att låta spelaren gå runt i en bana som är visad med en texturerad raycaster. Det ska finns fiender som kan skjuta och bli skjutna av spelaren. Det ska även finnas en enkel HUD som visar spelarens hälsa och ammunition.

Om det finns tid planerar vi att implementera pickups för hälsa och ammo, ljudeffekter, egna texturer, en startmeny samt en culling-funktion för att förbättra prestanda.

Ifall vi blir klara med allt detta planerar vi att implementera förmågan att åka upp och ner samt en map creator.

## Språk och verktyg
Vi skriver i C, med grafiksbiblioteket raylib. Det ska vara kompatibelt med åtminstone Linux.


## Naming conventions

Allt skrivs på engelska. Vi använder camelcase för funktioner, små bokstäver för variabler och stora bokstäver för konstanter. Typedefs (egna datatyper) är PascalCase. Filnamn är lowercase.


Branches döps efter funktionen som ska implementeras. Issues skrivs med instruktion först och problemet efter, till exempel: “Fix: Walls won’t load”. Commits skrivs som till exempel: “Fixed: Walls won’t load” eller “Added: Raycaster”

## Workflow

Branches bör godkännas av åtminstone en annan gruppmedlem innan de mergas. Projektet börjar med att all jobbar på grunderna. Sedan delar vi upp arbetet mer ordetligt.

## Uppdelning av arbete 
Se github Issues. 
