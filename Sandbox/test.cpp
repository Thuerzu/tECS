// tECS.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.

#include <iostream>
#include <string>

#include "tECS.h"

struct PositionComponent
{
    double x, y, z;
};

struct VelocityComponent
{
    double dx, dy, dz;
};

struct HealthComponent
{
    uint32_t value;
};

int main()
{
    tECS::ECS ecs;
    tECS::Entity entt = ecs.CreateEntity();
    tECS::Entity entt2 = ecs.CreateEntity();
    tECS::Entity entt3 = ecs.CreateEntity();
    ecs.Emplace<PositionComponent>(entt, 1., 1., 0.);
    ecs.Emplace<PositionComponent>(entt2, -1., 0., 0.);
    ecs.Emplace<PositionComponent>(entt3, 0., 0., 1.);
    ecs.Emplace<VelocityComponent>(entt, 0.1, 0., -0.1);
    ecs.Emplace<VelocityComponent>(entt2, 0.0, 0.5, 0.0);
    ecs.Emplace<HealthComponent>(entt, 35u);

    std::cout << ecs.HasAny<PositionComponent, VelocityComponent, HealthComponent>(entt2) << "\n";

    PositionComponent* pos = ecs.Get<PositionComponent>(entt);
	HealthComponent* health = ecs.Get<HealthComponent>(entt);
    //std::cout << "Position: " << pos->x << " | " << pos->y << " | " << pos->z << "\n";
    auto filterMovement = ecs.Where<VelocityComponent, PositionComponent>();

    double dt = 2.5;
    
    filterMovement.ForEach([dt](tECS::Entity e, VelocityComponent& vel, PositionComponent& pos) { pos.x += vel.dx * dt; pos.y += vel.dy * dt; pos.z += vel.dz * dt; std::cout << "Calculating movement of entity [" << e << "] ...\n"; });

	std::cout << "GetOr<HealthComponent>(entt): " << ecs.GetOr<HealthComponent>(entt, HealthComponent{ 0 }).value << "\n";
    std::cout << "GetOr<HealthComponent>(entt2): " << ecs.GetOr<HealthComponent>(entt2, HealthComponent{0}).value << "\n";
    std::cout << "GetOrEmplace<HealthComponent>(entt3): " << ecs.GetOrEmplace<HealthComponent>(entt3, 100u)->value << "\n";

    for (auto e : filterMovement)
    {
        pos = ecs.Get<PositionComponent>(e);
        std::cout << "Position: " << pos->x << " | " << pos->y << " | " << pos->z << "\n";
    }

	auto filterHealth = ecs.Where<HealthComponent>();

    for (auto e : filterHealth)
    {
        health = ecs.Get<HealthComponent>(e);
        std::cout << "Health of Entity [" << e << "]: " << health->value << "\n";
    }

    //solve(3457391, 2345786);
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
