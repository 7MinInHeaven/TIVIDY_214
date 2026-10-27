#include "Composite.h"

WorkComponent::WorkComponent(std::string name) : state(nullptr), item(nullptr), name(name){}

WorkComponent::WorkComponent(const WorkComponent& other) 
    : state(other.state), item(other.item), name(other.name) {}

WorkComponent::~WorkComponent() {
    // Clean up if needed, or leave empty if managed elsewhere
}