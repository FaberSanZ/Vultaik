

<h1 align="center">
   <img src="Logo/vultaik-logo 2.PNG" width=410>

  
  ## Vultaik (Architecture Axiom1) is a 2D Physics  Engine implemented in DX12®
  
</h1>


## Overview

## Low-level rendering backend
The rendering backend focuses entirely on DX12, so it reuses DX12 enums and data structures where appropriate.


# Axiom Architecture

Axiom is the architectural model used in the Xultaik and Vultaik engines.  
It defines how systems evolve over time while maintaining coherence, scalability, and clarity.

---

## Core Principles

### 1. Independent System Generations
Each system evolves independently through generations.

Examples:
- RenderSystem Gen 1 → Gen 2 → Gen 3
- PhysicSystem Gen 1 → Gen 2 → Gen 3

A generation represents a meaningful evolution of the system, not minor changes.

---

### 2. No Double Advancement per Architecture
A system cannot advance more than one generation within the same Axiom architecture.

❌ Invalid:
- RenderSystem Gen 1 → Gen 3 (within Axiom 1)

✅ Valid:
- RenderSystem Gen 1 → Gen 2 (Axiom 1)
- RenderSystem Gen 2 → Gen 3 (Axiom 2)

This enforces balanced development across all systems.

---

### 3. Architecture Coherence Rule
A new Axiom architecture is defined only when all core systems reach the same generation level.

Example:

Axiom 1:
- RenderSystem Gen 1
- PhysicSystem Gen 1

Axiom 2:
- RenderSystem Gen 2
- PhysicSystem Gen 2

Partial upgrades do not define a new architecture.

---

### 4. Meaningful Generational Changes
A new generation must represent a significant improvement, such as:

- API redesign
- Paradigm shift
- Performance improvements
- Internal system refactor

Minor changes do not qualify as a new generation.

---

## Purpose

The Axiom model ensures:

- Balanced evolution across systems
- Avoidance of over-engineering in isolated components
- Clear architectural milestones
- Strong technical identity
- Structured learning and teaching

---

## Philosophy

Axiom treats engine development as a coordinated evolution, not a collection of independent upgrades.

> "An architecture is not defined by change, but by coherence."
