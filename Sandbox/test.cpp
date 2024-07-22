// tECS.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include <string>

#include "tECS.h"

void solve(long a, long b)

{
    std::cout << "a\tb\tq\tr\tx\ty\n";

    long x = 0, y = 1, lastx = 1, lasty = 0, temp;

    while (b != 0)

    {

        std::cout << std::to_string(a) << "\t" << std::to_string(b) << "\t";

        long q = a / b;

        long r = a % b;

        std::cout << std::to_string(q) << "\t" << std::to_string(r) << "\t";


        a = b;

        b = r;



        temp = x;

        x = lastx - q * x;

        lastx = temp;



        temp = y;

        y = lasty - q * y;

        lasty = temp;

        std::cout << std::to_string(lastx) << "\t" << std::to_string(lasty) << "\n";


    }

    std::cout << ("Roots  x : " + std::to_string(lastx) + " y :" + std::to_string(lasty));

}

struct PositionComponent
{
    double x, y, z;
};

int main()
{
    tECS::ECS& ecs = *tECS::ECS::Get();
    tECS::Entity entt = ecs.CreateEntity();
    ecs.AddComponent(entt, PositionComponent({ 1, 1, 0 }));
    PositionComponent* pos = ecs.GetComponent<PositionComponent>(entt);
    std::cout << "Position: " << pos->x << " | " << pos->y << " | " << pos->z << "\n";
    solve(3457391, 2345786);
}

// Programm ausführen: STRG+F5 oder Menüeintrag "Debuggen" > "Starten ohne Debuggen starten"
// Programm debuggen: F5 oder "Debuggen" > Menü "Debuggen starten"

// Tipps für den Einstieg: 
//   1. Verwenden Sie das Projektmappen-Explorer-Fenster zum Hinzufügen/Verwalten von Dateien.
//   2. Verwenden Sie das Team Explorer-Fenster zum Herstellen einer Verbindung mit der Quellcodeverwaltung.
//   3. Verwenden Sie das Ausgabefenster, um die Buildausgabe und andere Nachrichten anzuzeigen.
//   4. Verwenden Sie das Fenster "Fehlerliste", um Fehler anzuzeigen.
//   5. Wechseln Sie zu "Projekt" > "Neues Element hinzufügen", um neue Codedateien zu erstellen, bzw. zu "Projekt" > "Vorhandenes Element hinzufügen", um dem Projekt vorhandene Codedateien hinzuzufügen.
//   6. Um dieses Projekt später erneut zu öffnen, wechseln Sie zu "Datei" > "Öffnen" > "Projekt", und wählen Sie die SLN-Datei aus.
