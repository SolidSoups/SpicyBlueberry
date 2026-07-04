# 5 Minute Pizza — UE 5.8 Weekend Prototype

## Overview
Competitive multiplayer game. Players deliver pizzas on foot from their restaurant to delivery points for score. Score weighted by delivery order, time, and pizza quality. Players can sabotage each other physically. World generates from a shared seed; delivery points shift during a run.

## Tech Stack
- Unreal Engine 5.8
- PCG Framework — world/terrain generation, delivery point placement
- Iris — network replication (opt-in, `bUseIris=true`)
- C++ primary, Blueprint for iteration only

## Team Roles
- Person A: PCG — seed-driven world generation, restaurant placement (fixed), delivery point placement (dynamic during run)
- Person B (this workspace's user): Iris — pizza object state, player combat, scoring, boss task/ability system

## Learning Goals
This project exists so each person can learn a specific new UE 5.8 system, not just to ship the game. Keep this in mind when explaining things or suggesting approaches.
- Person B (this workspace's user): learning Iris specifically. Already knows standard UE replication (has used it before), so Iris explanations should build on that base rather than starting from replication fundamentals — focus on what's different/new in Iris (push model, replication fragments/RepGraph-style filtering, opt-in via `bUseIris`), not a from-scratch replication tutorial.
- Person A: learning the PCG framework as it exists in 5.8 specifically. Has used PCG before 5.8, so focus on what's changed/new in 5.8's PCG rather than PCG basics.
- Prefer explanations and code that surface the new-system concepts explicitly rather than hiding them behind abstraction — the point is to understand how Iris/5.8 PCG work, not just to get a working result.

## Core Systems

### Delivery scoring
- Score components: delivery order (first/second/etc.), elapsed time, pizza quality
- Deliveries issued in batches of 2-4, player chooses route/priority
- Pizza is a physical actor, carried by hand from restaurant to delivery point

### Pizza quality
- Degrades from: bad assembly, contact with wrong objects, temperature loss over time
- Quality is server-authoritative state on the pizza actor, checked at delivery event

### Boss tasks / abilities
- Boss issues side tasks during active delivery runs, player decides whether to divert
- Completing a task grants bonus money + one ability
- Only one ability held at a time (new ability replaces old)
- Example ability: pizza becomes lightweight, floats

### Combat
- Non-lethal disruption only: shove, throw shovel
- Physics-based, not hitscan/damage-based
- Goal is to interrupt delivery/carry state, not eliminate players

### World generation
- Seed shared at run start, each client generates world locally (matches driving-game precedent: don't replicate geometry, replicate the seed)
- Restaurants: fixed positions, generated once, static for the run
- Delivery points: regenerate/adjust during gameplay, must be replicated as data (position + associated batch), since these change post-generation and all clients need the same values

## Architecture Notes
- Pizza actor: `Replicated` struct for quality state (contributing factors + resulting score float), separate from transform/physics replication
- Delivery point set: replicated array, server-authoritative, updated on batch completion/regeneration — not regenerated locally like static world, since it changes at runtime and desync here breaks scoring fairness
- Combat: shove/throw as server RPCs with client-side physics prediction if time allows, otherwise server-authoritative only for the weekend build
- Ability system: single-slot, replicated ability ID/state on player character

## Scope Constraints (weekend timebox)
- No vehicles (mechanic removed from prior draft)
- Fixed set of pizza-degradation triggers (2-3 max: bad assembly, wrong contact, cooldown)
- One ability implemented for the ability system's first pass (floating pizza)
- 2-4 players, direct join, no matchmaking
- No persistent economy beyond in-run bonus money from boss tasks

## Open Design Questions (unresolved, do not assume answers)
- Whether players must assemble their own pizza before pickup, or pizzas are pre-made
- Exact quality degradation formula/thresholds
- Delivery point regeneration trigger/frequency

## Session Norms
- State current implementation status at start of each new conversation if resuming work
- Flag when a request would break the weekend scope constraints above rather than silently expanding scope
- Code snippets: simple, minimal abstraction, no unrequested inheritance/interfaces
- Do not resolve open design questions unprompted; surface them, do not decide them

## Claude Code Permissions
Write/Edit tools are disabled for this directory (learning project — user wants to write code by hand). Do not attempt file edits; describe changes in chat instead and let the user apply them.
