// tECS.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.

#include <iostream>
#include <string>
#include <chrono>
#include <print>
#include <tECS.h>
#include <tuple>
#include <Profiler.hpp>

struct PositionComponent {
    double x, y, z;
};

struct VelocityComponent {
    double dx, dy, dz;
};

struct HealthComponent {
    uint32_t value;
};

template <size_t N>
void test() {
    std::cout << "================TESTING WITH " << N << " ENTITIES===============================\n";
    auto start = std::chrono::high_resolution_clock::now();

    using namespace tECS;
    ECS ecs;
    std::array<Entity, N> entities;
    for (int i = 0; i < entities.size(); i++)
        entities[i] = ecs.create_entity();
    std::chrono::duration<double, std::milli> entity_creation_data_point = std::chrono::high_resolution_clock::now() - start;
    
    //=============COMPONENT CREATION==========
    auto tp = std::chrono::high_resolution_clock::now();
    THLIB_SET_MARKER("EMPLACING POSITIONS");
    for (int i = 0; i < entities.size(); i++)
        ecs.emplace<PositionComponent>(entities[i], (double)i, (double)i, (double)i);
    THLIB_SET_MARKER("EMPLACING VELOCITIES");
    for (int i = 0; i < entities.size(); i += 2)
        ecs.emplace<VelocityComponent>(entities[i], -(double)i, (double)i, -(double)i);
    THLIB_SET_MARKER("EMPLACING HEALTH DATA");
    for (int i = 0; i < entities.size(); i += 4)
        ecs.emplace<HealthComponent>(entities[i], (uint32_t)i * 2 + 100);
    
    THLIB_SET_MARKER("<Position> SELECTION");
    ecs.where<PositionComponent>().for_each([](PositionComponent& pos) {
        pos.x += 2;
    });

    THLIB_SET_MARKER("<Position, Velocity>\\<Health> SELECTION");
    auto movementSelection = ecs.where<PositionComponent, VelocityComponent>(Exclude<HealthComponent>{});
    for (auto[e, pos, vel] : movementSelection) {
        pos.x += vel.dx; pos.y += vel.dy; pos.z += vel.dz;
        vel.dx -= 1;
    };

    //============OUTPUT=================
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Entity creation: " << entity_creation_data_point.count() << "\n";
    entity_creation_data_point = end - tp;
    std::cout << "Component creation: " << entity_creation_data_point.count() << "\n";
    entity_creation_data_point = end - start;
    std::cout << "Test time: " << entity_creation_data_point.count() << "\n";
}


int main() {
    std::println("Current logging directory: {}", LOGGING_DIRECTORY);

    tECS::ECS ecs;
    tECS::Entity entt = ecs.create_entity();
    tECS::Entity entt2 = ecs.create_entity();
    tECS::Entity entt3 = ecs.create_entity();
    
    ecs.emplace<PositionComponent>(entt, 1., 1., 0.);
    ecs.emplace<PositionComponent>(entt2, -1., 0., 0.);
    ecs.emplace<PositionComponent>(entt3, 0., 0., 1.);
    ecs.emplace<VelocityComponent>(entt, 0.1, 0., -0.1);
    ecs.emplace<VelocityComponent>(entt2, 0.0, 0.5, 0.0);
    ecs.emplace<HealthComponent>(entt, 35u);

    std::cout << ecs.has_any<PositionComponent, VelocityComponent, HealthComponent>(entt2) << "\n";

    PositionComponent* pos = ecs.get<PositionComponent>(entt);
	HealthComponent* health = ecs.get<HealthComponent>(entt);
    
    auto filterMovement = ecs.where<VelocityComponent, PositionComponent>();

    double dt = 2.5;
    
    filterMovement.for_each([dt](VelocityComponent& vel, tECS::Entity e, PositionComponent& pos) { pos.x += vel.dx * dt; pos.y += vel.dy * dt; pos.z += vel.dz * dt; std::cout << "Calculating movement of entity [" << e << "] ...\n"; });

    for (auto[e, vel, pos] : filterMovement) {
        std::cout << "Position: " << pos.x << " | " << pos.y << " | " << pos.z << "\n";
    }

	auto filterHealth = ecs.where<HealthComponent>();

    for (auto[e, health] : filterHealth) {
        std::cout << "Health of Entity [" << e << "]: " << health.value << "\n";
    }

    tBenchmark::Profiler::get().new_profile("TestEntities", std::string(LOGGING_DIRECTORY) + "/ECS.json");
    test<100>();
    test<1000>();
    test<10000>();
    tBenchmark::Profiler::get().end_profile();
}