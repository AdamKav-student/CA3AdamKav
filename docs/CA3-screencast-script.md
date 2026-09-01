# CA3 Screencast Script — Armoured Warfare (RoboCat / UDP)

Working notes for the screencast. Each section is: **what to have on screen**, **what to say**,
and the **code to point at**. Every number below is taken from the code, not estimated by feel —
the file and line references are there so you can jump straight to them on camera.

Suggested total: 12–15 minutes. Sections 1–3 are the demo, 4–11 are the walkthrough,
12 is the CA2/CA3 comparison.

---

## 0. Before you hit record

Build both projects (`RoboCatSFMLServer`, `RoboCatSFMLClient`) in the same configuration.

Launch order and arguments (`Server.cpp:45`, `Client.cpp:54`):

| Executable | Arg 1 | Arg 2 |
|---|---|---|
| `RoboCatSFMLServer.exe` | port, e.g. `45000` | optional simulated latency in seconds, e.g. `0.1` |
| `RoboCatSFMLClient.exe` | `ip:port`, e.g. `127.0.0.1:45000` | player name, e.g. `Adam` |

Run one server and **two or three clients** side by side (two windows visible at once is enough
to prove replication; three is better for the scoreboard).

Two things to be aware of on camera:

- The window and view are fixed at 1280×720 (`WindowManager.cpp:7`, `RenderManager.cpp:7`) but
  the world walls are at 2500×1500 (`RoboCat.cpp:3`). There is no camera follow, so keep the
  tanks in the left/upper part of the map or they drive off the visible area.
- The server closes itself when the last client disconnects
  (`NetworkManagerServer.cpp:282`), so close the clients last.

---

## 1. Game Demo — showing it working in a multiplayer environment

**On screen:** server console, then two client windows tiled.

**Say:**

> This is a client–server game. The server is a headless console application; every player runs a
> separate client process and they only ever talk to the server, never to each other. I'm running
> both clients on this machine against `127.0.0.1`, but the only thing that changes on a LAN is the
> address on the command line — there's nothing loopback-specific in the code.

Demo beats, in this order:

1. **Join** — start client 1, watch the server log `Server Welcoming, new client 'Adam' as player 1`
   (`NetworkManagerServer.cpp:115`). Start client 2 — **point out that player 1's window immediately
   shows the second tank appear**. That is replication doing its job.
2. **Different models** — player 1 drives a Sherman, player 2 a Panzer IV. That's picked from the
   join-order player id, not chosen locally (`TankType.hpp:13`, `RoboCatClient.cpp:99`).
3. **Movement sync** — drive tank 1, show it moving in tank 2's window at the same time. Rotate the
   turret with the arrow keys independently of the chassis — turret angle is replicated too.
4. **Shooting and damage** — fire on player 2, show health dropping in their HUD, four hits to kill,
   the death, the 3-second respawn, and the kill counter going up on the shooter's screen.
5. **Pickups** — drive over an oil barrel, show health going back up on both screens and the barrel
   disappearing on both screens.
6. **Scoreboard** — both clients show the same names and kill counts in the top left.
7. **Network panel** — hold **Tab**. This is the money shot for a networking module: live tick rate,
   round-trip time, packets sent/delivered/dropped, and bytes in/out per second
   (`HUD.cpp:35`). Leave it up for a few seconds.
8. **Latency** — press **`+`** two or three times to add 100 ms of simulated latency per press
   (`InputManager.cpp:111`), and show the round-trip number climbing in the Tab panel and the
   remote tank starting to rubber-band. Press **`-`** to bring it back. This proves the game is
   genuinely networked and not sharing memory.
9. **Disconnect** — Alt-F4 one client, show the other client's scoreboard entry disappear and the
   tank vanish within about 3 seconds (`NetworkManagerServer.cpp:259`, timeout is `3.f` at `:10`).
10. **Win condition** — if you have time, get to 10 kills and show the victory screen on the winner
    and the defeat screen on the loser (`ScoreBoardManager.hpp:6`, `GameStateManager.cpp:60`).

---

## 2. Design Changes — how the design changed for a multiplayer environment

**Say:**

> The single biggest change is that the client no longer owns the truth about anything. In a
> single-player build, pressing W moves your tank and that's the end of it. Here, pressing W
> produces a *move* — an input sample with a timestamp — that goes to the server, and the server
> decides what actually happened.

The concrete design changes:

- **The game was split into three projects.** `RoboCatSFML` is shared code — game objects,
  serialisation, the socket layer, the scoreboard. `RoboCatSFMLServer` and `RoboCatSFMLClient`
  each subclass the shared types (`RoboCat` → `RoboCatServer` / `RoboCatClient`) so simulation
  and presentation are separated. The server has no SFML window at all.
- **Every object that matters got a network identity.** `GameObject` carries a network id
  (`GameObject.hpp:50`) and a four-character class id (`CLASS_IDENTIFICATION`), so the server can
  say "object 7, which is an `RCAT`" and the client can build the right type from the registry.
- **Objects can only be created by the server.** The client's `GameObjectRegistry` creates a
  `RoboCatClient` or a `YarnClient` only in response to a replicated create
  (`ReplicationManagerClient.cpp:40`). A client cannot spawn a shell for itself.
- **Serialisation was designed into the objects.** Every replicated class implements
  `Write(stream, dirtyState)` and `Read(stream)` (`RoboCat.cpp:203`, `RoboCatClient.cpp:80`),
  and the state is split into a bitmask of properties so only what changed is sent.
- **Authority calls moved server-side.** Damage, scoring, respawns and the fire cooldown all live
  in `RoboCatServer` (`RoboCatServer.cpp:65`, `:79`, `:99`). The client has a copy of the cooldown
  constant, but only so it doesn't play a firing sound for a shot the server is going to refuse.
- **The client gained prediction and smoothing.** Your own tank is simulated locally straight away
  and then reconciled against the server (`RoboCatClient.cpp:179`); other players' tanks are
  extrapolated forward and interpolated in (`RoboCatClient.cpp:286`).
- **Gameplay tuning changed because of the network.** The tanks were slowed down
  (`RoboCat.cpp:12` — max rotation went from 100 to 70, linear speed from 5000 to 2000) because
  fast objects make prediction errors much more visible, and a one-second fire delay was added so
  a held fire key doesn't create a shell per frame per client.

---

## 3. Client Overview — how the client works

**On screen:** `Client.cpp`, then `NetworkManagerClient.cpp`.

The client frame, `Client::DoFrame` (`Client.cpp:92`), in order:

1. `InputManager::Update()` — samples the keyboard into a `Move` every 30 ms
   (`InputManager.cpp:7`, `:181`) and pushes it onto the local `MoveList`.
2. `Engine::DoFrame()` → `World::Update()` — every game object updates. Your own tank consumes the
   pending move and simulates it immediately: **that's the client-side prediction**
   (`RoboCatClient.cpp:46`).
3. `NetworkManagerClient::ProcessIncomingPackets()` — reads packets off the UDP socket, and applies
   the replicated state.
4. `GameStateManager::CheckForGameOver()` — looks at the replicated scoreboard for anyone on 10 kills.
5. `RenderManager::Render()` — draws sprites in draw order, then the HUD, then the win/lose overlay.
6. `NetworkManagerClient::SendOutgoingPackets()` — sends an input packet every 33 ms.

**Say about the connection state machine** (`NetworkManagerClient.hpp:3`):

> The client has three states: uninitialised, saying hello, and welcomed. While it's saying hello it
> re-sends a `HELO` packet once a second until the server answers — that's how a lossy protocol does
> a handshake. Once it gets a `WLCM` with its player id it flips to welcomed and starts sending
> input.

**Say about prediction and reconciliation** (`RoboCatClient.cpp:158`–`:198`):

> When a state packet arrives for my own tank, the server's position is authoritative but it's
> already out of date by half a round trip. So I take the server position, then replay every move
> the server hasn't acknowledged yet on top of it (`DoClientSidePredictionAfterReplicationForLocalCat`).
> If the result still differs from where I had drawn myself, I don't snap — I lerp 10% of the way per
> update (`InterpolateClientSidePrediction`), so a correction looks like drift instead of a teleport.
>
> For *other* players' tanks I do the opposite: their position is a round trip old, so I simulate them
> forward by one RTT in 1/30th-of-a-second steps (`DoClientSidePredictionAfterReplicationForRemoteCat`)
> and then interpolate the correction in over the same RTT.

---

## 4. Server Overview — how the server works

**On screen:** `Server.cpp:83` (`DoFrame`), then `NetworkManagerServer.cpp`.

`Server::DoFrame`:

```cpp
NetworkManagerServer::sInstance->ProcessIncomingPackets();   // read and queue input
NetworkManagerServer::sInstance->CheckForDisconnects();      // 3s silence = gone
NetworkManagerServer::sInstance->RespawnCats();              // 3s after death
Engine::DoFrame();                                           // World::Update() — the simulation
NetworkManagerServer::sInstance->SendOutgoingPackets();      // state out to each client
```

**Say:**

> The server is authoritative and headless. It owns the world, and it keeps one `ClientProxy` per
> connected player (`ClientProxy.hpp`). The proxy is the important structure: it holds that player's
> socket address, their unprocessed move list, their own delivery-notification manager, and — this
> is the part people miss — **their own replication manager**. Replication state is per client, not
> global, because client A might have acknowledged the create of object 7 while client B hasn't.

Point out the per-client work in `RoboCatServer::Update` (`RoboCatServer.cpp:16`):

> The server doesn't simulate tanks off its own clock. It walks that client's unprocessed moves and
> runs `ProcessInput` and `SimulateMovement` once per move, with **the delta time the client
> recorded**. That's what keeps the server's replay and the client's prediction bit-identical, and
> it's also why a laggy player doesn't get slowed down — their moves still all get executed, just
> later.

Then, after the simulation, the server compares before/after and marks state dirty
(`RoboCatServer.cpp:57`):

```cpp
if (!RoboMath::Is2DVectorEqual(oldLocation, GetLocation()) || ... )
{
    NetworkManagerServer::sInstance->SetStateDirty(GetNetworkId(), ECRS_Pose);
}
```

> Nothing is sent because a frame happened. Things are sent because they *changed*.

Two more server behaviours worth naming:

- **New client** (`NetworkManagerServer.cpp:74`): create a `ClientProxy`, register it under both its
  address and its player id, add a scoreboard entry, spawn a tank, send `WLCM`, and then queue a
  create for *every object already in the world* so the joiner catches up.
- **Disconnects** (`NetworkManagerServer.cpp:255`): a client that hasn't sent anything for 3 seconds
  is dropped; and on Windows, a closed client makes the next send return `WSAECONNRESET`, which is
  handled immediately (`NetworkManagerServer.cpp:20`) rather than waiting out the timeout.

---

## 5. Protocol — the client/server communication protocol

**On screen:** `NetworkManager.hpp:6`.

Four packet types, each a four-character code so they're readable in a hex dump:

```cpp
static const uint32_t kHelloCC   = 'HELO';   // client -> server: I want in, here's my name
static const uint32_t kWelcomeCC = 'WLCM';   // server -> client: you are player N
static const uint32_t kStateCC   = 'STAT';   // server -> client: the world changed
static const uint32_t kInputCC   = 'INPT';   // client -> server: here are my last moves
```

**The handshake:**

| Step | Packet | Contents |
|---|---|---|
| 1 | `HELO` (repeated every 1 s until answered) | four-CC + player name string |
| 2 | `WLCM` | four-CC + player id (int) |
| 3 | server-side, no packet | scoreboard entry added, tank spawned, a create queued for every existing object |

**Steady state, both directions at ~30 Hz:**

`INPT` (client → server), built in `SendInputPacket` (`NetworkManagerClient.cpp:220`):

```
[ 'INPT' 32 bits ][ sequence number 16 ][ has-acks 1 (+ ack range) ][ move count 2 bits ][ up to 3 moves ]
```

`STAT` (server → client), built in `SendStatePacketToClient` (`NetworkManagerServer.cpp:158`):

```
[ 'STAT' 32 ][ sequence number 16 ][ timestamp-dirty 1 (+ last processed move timestamp 32) ]
[ scoreboard ][ replication records... ]
```

**Say about reliability** (`DeliveryNotificationManager.cpp`):

> UDP gives me nothing, so the protocol layers its own delivery notification on top. Every packet
> carries a 16-bit sequence number. The receiver acknowledges the sequence numbers it got, as ranges
> (`AckRange.cpp:3`), and the sender keeps a queue of in-flight packets. When an ack comes back it
> reports success; when a packet is skipped over by a later ack, or 500 ms goes by
> (`kDelayBeforeAckTimeout`, `DeliveryNotificationManager.cpp:5`), it reports failure.
>
> The key point is that this is **notification, not retransmission**. It never re-sends the old
> packet, because a 500 ms old tank position is useless. Instead it tells the replication manager
> "that create didn't land", and the replication manager re-dirties the object so the *current*
> state goes out in the next packet (`ReplicationManagerTransmissionData.cpp:59`).
>
> The two ends are asymmetric on purpose: the client acknowledges state packets but doesn't need
> acks itself, the server processes acks but doesn't send any — you can see it in the two
> constructor arguments, `mDeliveryNotificationManager(true, false)` on the client
> (`NetworkManagerClient.cpp:15`) and `(false, true)` on the proxy (`ClientProxy.cpp:12`).
> Input doesn't need reliability because the same move is deliberately sent in three consecutive
> packets — redundancy instead of retransmission.

---

## 6. Your Code — what you wrote on top of the class example

The starting point is the commit "Working Sample" (`45578c2`). Everything after it is this project:
**49 source files changed, ~1,470 lines added**. Show `git log --oneline` on camera if you like.

Worth calling out specifically:

**Gameplay and replication**

- **Independent turret.** New input axis (`InputState.hpp:27` `mDesiredTurretAmount`, encoded in
  `InputState.cpp:36`), rotation applied in `RoboCat::ProcessInput` (`RoboCat.cpp:48`), replicated
  as an extra float inside the pose block (`RoboCat.cpp:235`, read at `RoboCatClient.cpp:129`), and
  shells launched down the turret's forward vector rather than the chassis'
  (`RoboCat.cpp:27` `GetTurretForwardVector`, used by `Yarn::InitFromShooter`).
- **Health as a replicated property.** New `ECRS_Health` bit (`RoboCat.hpp:11`), capped at 4 and
  **written in 4 bits** (`RoboCat.cpp:270`), with the cap chosen so it fits
  (`RoboCat.hpp:24` — "health is replicated in four bits, so this has to stay under 16").
- **Barrels heal instead of scoring.** `MouseServer::HandleCollisionWithCat` calls
  `RoboCatServer::Heal`, and only consumes the barrel if it actually healed someone
  (`MouseServer.cpp:18`, `RoboCatServer.cpp:79`).
- **Server-enforced fire delay.** `kTimeBetweenShots` (`RoboCat.cpp:6`) enforced in
  `RoboCatServer::HandleShooting` (`RoboCatServer.cpp:65`), mirrored on the client purely for the
  firing sound (`InputManager.cpp:150`).
- **Tank model from join order** (`TankType.hpp`, `RoboCatClient.cpp:99`) — replaced an earlier
  local selection screen, because model choice has to be something every client agrees on.
- **Win/lose.** `kKillsToWin = 10` (`ScoreBoardManager.hpp:6`), evaluated on each client from the
  replicated scoreboard in `GameStateManager::CheckForGameOver` (`GameStateManager.cpp:60`).

**Networking diagnostics (this is the bit to linger on)**

- **The Tab network panel**, `HUD::RenderNetworkStats` (`HUD.cpp:35`) — reads the server tick rate,
  RTT, unacknowledged move count, simulated latency, packets dispatched/delivered/dropped and
  bytes/second straight out of the network managers. To build it I added
  `GetServerTickRate`, `GetLastMoveProcessedByServerTimestamp` and
  `GetDeliveryNotificationManager` accessors to `NetworkManagerClient`, and
  `WeightedTimedMovingAverage::UpdatePerSecond` to measure state packets per second.

**Robustness — the "shooting drops the connection" bug hunt (commit `07d0f02`)**

This is a strong thing to talk about because every one of these is a genuine networking failure mode:

- A shell that hits a tank dies *between two writes*, so `ReplicationManagerServer::Write` was asking
  an already-unregistered object to serialise itself (`ReplicationManagerServer.cpp:60`).
- `SetStateDirty` / `ReplicateDestroy` / `HandleCreateAckd` used `operator[]`, which **fabricates** a
  replication command for an id the client has never heard of, and the default-constructed command
  had uninitialised state (`ReplicationCommand.hpp:19`, `ReplicationManagerServer.cpp:24`).
- The client dereferenced a null object on an update for something it had already destroyed — which
  is legitimate, because the destroy and the update can cross on the wire. Since the record length
  isn't known, the fix is to stop reading the packet (`ReplicationManagerClient.cpp:78`).
- **Two malformed-packet hardening fixes**, which are the ones with security flavour:
  `ScoreBoardManager::Read` resized a vector to an unvalidated count read off the wire
  (`ScoreBoardManager.cpp:109`), and the replication reader trusted an unknown action code and an
  unknown class id (`ReplicationManagerClient.cpp:26`, `:58`). All three now bound themselves by
  what the packet could actually contain.
- `DeliveryNotificationManager`'s destructor divided by the dispatched packet count — a client that
  connected and never sent a move took the **whole server** down, and everyone with it
  (`DeliveryNotificationManager.cpp:26`).
- The 2-bit move count wrapped when 4 or 5 moves were sent, so the server read the wrong number and
  everything after it in the packet was misaligned (`NetworkManagerClient.cpp:234`).
- `mTimeOfLastInputPacket` was uninitialised; left as stack garbage it could suppress *every* input
  packet, and the server would then time the client out for going quiet
  (`NetworkManagerClient.cpp:17`).

---

## 7. Data Structures — what carries data between server and client

| Structure | File | Role |
|---|---|---|
| `OutputMemoryBitStream` / `InputMemoryBitStream` | `MemoryBitStream.hpp` | Bit-level reader/writer. Everything on the wire goes through these; `Write(value, bitCount)` is what makes 2-bit and 4-bit fields possible. |
| `InputState` | `InputState.hpp` | One keyboard sample: horizontal, vertical, turret, firing. |
| `Move` | `Move.hpp` | An `InputState` + timestamp + delta time. The unit of client input. |
| `MoveList` | `MoveList.hpp` | Deque of moves. Client: moves not yet acknowledged. Server: moves not yet simulated. `AddMoveIfNew` (`MoveList.cpp:15`) de-duplicates the redundant copies. |
| `ClientProxy` | `ClientProxy.hpp` | The server's view of one player: address, name, id, move list, delivery manager, replication manager, respawn timer. |
| `ReplicationCommand` | `ReplicationCommand.hpp` | Per object, per client: an action (create/update/destroy) plus a dirty-state bitmask. |
| `InFlightPacket` | `InFlightPacket.hpp` | A sent packet's sequence number, dispatch time, and what it was carrying. |
| `ReplicationManagerTransmissionData` | `ReplicationManagerTransmissionData.hpp` | Attached to an in-flight packet: the list of (network id, action, state) in it, so a loss can be undone. |
| `AckRange` | `AckRange.hpp` | A run of consecutive acknowledged sequence numbers — start plus count, so N acks cost 17–25 bits, not 16N. |
| `ScoreBoardManager::Entry` | `ScoreBoardManager.hpp` | Colour, player id, name, score. Serialised whole in every state packet. |
| `unordered_map<int, GameObjectPtr>` | `NetworkManager.hpp:35` | The linking context — network id to local object, on both ends. This is what makes "object 7" mean the same tank in three different processes. |

The replication record itself is the important wire structure:

```
[ network id 32 ][ action 2 bits ]
  create  -> [ class id 32 ][ object state ]
  update  -> [ object state ]
  destroy -> nothing
```

and "object state" for a tank is five presence bits, each followed by its block if set
(`RoboCat.cpp:203`): player id (32), pose (velocity 64 + location 64 + rotation 32 + turret 32),
thrust direction (1 extra bit), colour (96), health (4).

---

## 8. Compression / Limiting Data — and the size estimate

**Say — what's actually done:**

1. **Bit packing, not byte packing.** `OutputMemoryBitStream` writes arbitrary bit counts, so the
   replication action is 2 bits (`ReplicationManagerServer.cpp:69`), the move count is 2 bits
   (`NetworkManagerClient.cpp:242`), health is 4 bits, and a `Yarn`'s player id is 8
   (`Yarn.cpp:57`).
2. **Dirty-state replication.** Only properties that changed are written, marked by
   `SetStateDirty` after the simulation (`RoboCatServer.cpp:57`). A tank driving in a straight line
   sends pose only — no colour, no player id, no health.
3. **Presence bits.** Each optional block costs 1 bit when it's absent, so a skipped 96-bit colour
   costs one bit rather than being sent as zeros.
4. **Input is encoded as direction, not magnitude.** `WriteSignedBinaryValue` (`InputState.cpp:5`)
   writes 1 bit for "zero", or 2 bits for "non-zero and this sign" — three analogue-looking axes in
   6 bits instead of 96.
5. **Send on change, not on tick.** The server only sends a state packet to a client whose last move
   timestamp is dirty (`NetworkManagerServer.cpp:140`), so state output is driven by input arrival.
6. **Redundancy instead of retransmission.** Only the last 3 moves are sent
   (`NetworkManagerClient.cpp:234`), and only when there are moves at all.
7. **Ack ranges** rather than one ack per packet (`AckRange.cpp:3`).

**The size estimate — do this arithmetic on camera, it's the part the rubric wants:**

Input packet (client → server):

```
'INPT'              32 bits
sequence number     16
ack range           17
move count           2
3 moves            ~114   (each: 4-7 bits of input + 32-bit timestamp)
                  -----
                  ~181 bits = 23 bytes payload, 51 bytes on the wire with UDP+IP headers
                  at 30 packets/s  ->  ~1.5 KB/s per client, upstream
```

State packet (server → client), 2 players with 6-character names, both moving, one shell in flight:

```
header + timestamp   81 bits
scoreboard          512   (32 + 2 entries x [96 colour + 32 id + 32 length + 48 name + 32 score])
2 tank pose updates 464   (each 34 bits of record header + 198 bits of state)
1 shell pose update 197
                  -----
                 1,254 bits = 157 bytes payload, 185 bytes on the wire
                 at 30 packets/s  ->  ~5.5 KB/s per client, downstream
```

A full create is the biggest single record: a tank create is 396 bits (~50 bytes), a shell create
333 bits (~42 bytes), and a destroy is only the 34-bit record header.

**Be honest about what isn't compressed** — it makes the answer stronger:

> The one thing I'd fix first is the scoreboard. It's written in full in every single state packet
> (`NetworkManagerServer.cpp:208`), including 96 bits of colour and the whole name string per player,
> none of which ever change after a player joins. At two players that's 512 bits — **about 40% of a
> typical packet**. Sending it only when a score changes, or replicating it as a game object like
> everything else, would cut the packet roughly in half.
>
> Second, position and rotation are full 32-bit floats. The map is 2500×1500, so a 16-bit fixed-point
> quantisation would give me ~4 cm of precision and halve the pose block from 192 bits to about 96.
> The fixed-point helpers are already there in `MemoryBitStream.hpp:7` — they're used for
> quaternions in the sample code and I never applied them to my own state.

---

## 9. Player Capacity — how many players the server supports

**Say:** there are three separate limits, and they land in roughly the same place.

**1. Bandwidth.** Server output is O(N²): every one of the N clients gets a packet describing
roughly N tanks plus N scoreboard entries. Per client per packet ≈ `113 + 472N` bits.

| Players | Packet on the wire | Per client down | Server upstream total |
|---|---|---|---|
| 2 | ~185 B | 5.5 KB/s | 11 KB/s (0.09 Mbit/s) |
| 4 | ~328 B | 9.8 KB/s | 39 KB/s (0.3 Mbit/s) |
| 8 | ~514 B | 15.4 KB/s | 123 KB/s (1.0 Mbit/s) |
| 16 | ~986 B | 29.6 KB/s | 473 KB/s (3.8 Mbit/s) |

**2. MTU — this is the hard wall.** The receive buffer is a fixed `char packetMem[1500]`
(`NetworkManager.cpp:54`), but the output stream grows without limit (`MemoryBitStream.cpp:10`).
At roughly **24 players** a state packet exceeds 1500 bytes; it will fragment on the way out and be
truncated on the way in, and a truncated packet means a misaligned replication stream. Nothing
detects this — it just breaks.

**3. Ingress rate.** `ProcessIncomingPackets` reads at most 10 packets per call
(`NetworkManager.hpp:11`, `NetworkManager.cpp:63`). The server loop is uncapped so in practice it
runs at hundreds of iterations per second and this is not binding — but if the server were ever
frame-limited to 60 Hz, 600 packets/s at 30 packets/s per client is exactly **20 clients**.

**The answer to give:**

> Comfortably, on a LAN, **8 players**. The practical ceiling before the packet exceeds an MTU and
> starts corrupting is **around 16 to 20**. Past that it doesn't degrade gracefully, it breaks,
> because nothing splits a state packet across datagrams. Fixing the scoreboard duplication and
> quantising positions would roughly double that; properly, you'd add relevance filtering so a
> client is only told about objects near it, which turns the O(N²) into something closer to linear.
> For the demo I'm running 2 to 3.

---

## 10. State Synchronisation — players, bullets, "enemies"

**Say:** there is exactly one mechanism, and all three object types go through it.

> The server keeps a map from network id to game object. When an object is created, `RegisterGameObject`
> (`NetworkManagerServer.cpp:288`) hands it an id and queues a **create** command on every connected
> client's replication manager. When it changes, `SetStateDirty` ORs the changed property bits into
> every client's command. When it dies, `UnregisterGameObject` queues a **destroy**. Then once per
> packet, `ReplicationManagerServer::Write` (`ReplicationManagerServer.cpp:45`) walks that client's
> commands and writes a record for each one with dirty state.

Per object type:

- **Players (`RCAT`)** — pose, thrust, colour, player id, health. Dirtied after each simulation step
  by comparing before/after (`RoboCatServer.cpp:57`) and on damage/heal
  (`RoboCatServer.cpp:94`, `:129`). This is the only object with client-side prediction: your own
  tank is simulated ahead and reconciled by move replay, other players' are extrapolated by one RTT.
- **Bullets (`YARN`)** — created server-side only, in `RoboCatServer::HandleShooting`
  (`RoboCatServer.cpp:65`), which is also where the fire cooldown is enforced. They replicate pose,
  colour and player id. Between updates the client dead-reckons them locally
  (`Yarn::Update`, `Yarn.cpp:97`), which is why they look smooth at 30 Hz. They die after 1 second
  (`YarnServer.cpp:7`) or on hitting a tank, and either way that's a destroy record.
- **"Enemies" / pickups (`MOUS`, the oil barrels)** — 10 are spawned by the server at start-up
  (`Server.cpp:77`). They never move, so after the initial create they cost nothing at all until
  someone picks one up and they're destroyed. Be straight about this on camera: **this build has no
  AI enemies.** The `ESCT_AI` control path exists in `RoboCatServer` and would simulate a tank
  server-side with no client proxy, but nothing spawns one; the game mode is player-vs-player
  deathmatch, and the barrels are the only non-player objects.
- **The scoreboard** is synchronised separately, outside the replication system, as a block written
  into every state packet (`NetworkManagerServer.cpp:208`, read at `NetworkManagerClient.cpp:191`).

**The reliability guarantee** (`ReplicationManagerTransmissionData.cpp`) — worth one sentence:

> Because creates and destroys can't be allowed to go missing, each state packet remembers which
> records it carried. If it's declared lost, a lost create is re-queued as a create, a lost destroy
> as a destroy, and a lost update re-dirties **only the bits that aren't already on their way in
> another in-flight packet** (`ReplicationManagerTransmissionData.cpp:87`).

---

## 11. Known Issues — synchronisation problems the game has

Name these before the examiner does. Ordered most-visible first.

1. **Shells have no client-side prediction.** Pressing fire does nothing locally; the shell appears
   only when the server's create record arrives, so it's half a round trip late. Add 300 ms with
   `+` and it's obvious. The firing *sound* plays immediately, which papers over it.
2. **Remote tanks rubber-band under loss.** They're extrapolated forward a whole RTT and then
   corrected (`RoboCatClient.cpp:286`); if a pose update is dropped, the next one arrives as a
   larger correction. Related: `InterpolateClientSidePrediction` (`RoboCatClient.cpp:222`) only
   smooths location and velocity — **rotation and turret rotation are snapped**, so a remote turret
   can visibly jump.
3. **Collisions are resolved independently on every client.** `RoboCat::ProcessCollisions` runs in
   every client's local simulation as well as the server's, so two tanks shoving each other look
   different on each screen until the next pose update reconciles them.
4. **No lag compensation.** The server tests shell hits against present-time positions, with no
   rewind to the shooter's view of the world. At high ping you have to lead your shots.
5. **Out-of-order packets are dropped entirely.** `ProcessSequenceNumber`
   (`DeliveryNotificationManager.cpp:83`) discards anything older than expected, so a reordered
   packet's contents are lost and have to come back via the loss path.
6. **Only one ack range per packet.** `WriteAckData` (`DeliveryNotificationManager.cpp:59`) pops a
   single range, so under sustained loss acknowledgements fall behind and the 500 ms timeout does
   the work instead — state can take half a second to be re-sent.
7. **A client that loses all its input packets stops receiving state**, because state is only sent
   when the last-move timestamp is dirty (`NetworkManagerServer.cpp:140`). It recovers as soon as
   one input packet gets through, and the 3-second timeout catches the terminal case.
8. **Game over is decided per client.** Each client independently reads the scoreboard
   (`GameStateManager.cpp:60`), so two clients can flip to victory/defeat a packet apart, and a
   client that misses that packet sees it late.
9. **No camera.** The view is 1280×720 but the walls are at 2500×1500 (`RoboCat.cpp:3`) — a tank can
   drive out of the visible area while remaining perfectly in sync.
10. **All players spawn on the same spot**, `(600 - playerId, 400)` (`Server.cpp:121`), so respawns
    can overlap and shove each other apart.
11. **Nothing splits an oversized state packet** — see the capacity section; it's a correctness bug
    waiting at ~24 players.

---

## 12. Game Persistence

**Be straight about this one.** There is no persistence in the build:

> Nothing is written to disk. The scoreboard lives in the server's memory as a `vector<Entry>`
> (`ScoreBoardManager.hpp:54`), it's replicated to clients in every state packet, and it's gone when
> the process exits — and the process exits as soon as the last client disconnects
> (`NetworkManagerServer.cpp:282`). Scores also don't survive a player leaving: `HandleLostClient`
> removes their entry entirely (`Server.cpp:130`), so a reconnecting player comes back as a new
> player id with a score of zero.
>
> What persistence *would* look like here is small and well-localised, because the scoreboard is
> already the single source of truth: write `mEntries` out as name/score pairs in
> `HandleClientDisconnected`, and load an all-time high-score table at server start-up so the client
> can render it beside the live board. That's the one rubric item I haven't implemented.

*(If you'd rather show something here than admit a gap, this is a ~40-line change on the server —
say the word and I'll add it before you record.)*

---

## 13. CA2 vs CA3 — TCP versus UDP

**On screen:** `game_server.cpp` / `network_protocol.hpp` from CA2 beside `NetworkManagerServer.cpp`
/ `ReplicationManagerServer.cpp` from CA3.

### High level

| | **CA2 — SFML Book architecture, TCP** | **CA3 — RoboCat architecture, UDP** |
|---|---|---|
| Transport | `sf::TcpListener` / `sf::TcpSocket`, one stream per peer | One `UDPSocket`, one port, all peers |
| Server | A thread inside the client process (`GameServer::ExecutionThread`) | A separate headless executable |
| Reliability | Free, from TCP | Built by hand: sequence numbers, ack ranges, in-flight packets, timeouts |
| Send rate | Fixed 20 Hz tick (`tick_rate = 1/20`) plus event packets | ~30 Hz, driven by input arrival |
| What's sent | The whole world, every tick, every field | Only objects whose state changed, only the changed properties |
| Encoding | `sf::Packet` — byte-aligned `<<` / `>>` | Bit stream — 2-bit, 4-bit, 8-bit fields |
| Authority | Split: clients report their own positions | Server-authoritative: clients send input only |
| Client smoothing | None | Prediction, move replay, interpolation, extrapolation |
| Identity | `uint8_t` aircraft identifier | `int` network id + four-CC class id, in a shared linking context |

### Two code examples worth putting side by side

**(a) Sending state.** CA2 broadcasts the entire world on a fixed tick, to everyone, unconditionally:

```cpp
// CA2 — game_server.cpp, UpdateClientState()
update_client_state_packet << static_cast<uint8_t>(Server::PacketType::kUpdateClientState);
update_client_state_packet << static_cast<float>(m_battlefield_rect.position.y + ...);
update_client_state_packet << static_cast<uint8_t>(m_aircraft_count);
for (const auto& aircraft : m_aircraft_info)
{
    update_client_state_packet << aircraft.first << aircraft.second.m_position.x
        << aircraft.second.m_position.y << aircraft.second.m_hitpoints
        << aircraft.second.m_missile_ammo;
}
SendToAll(update_client_state_packet);
```

CA3 sends a per-client diff, and only for objects flagged dirty:

```cpp
// CA3 — ReplicationManagerServer.cpp, Write()
for (auto& pair : mNetworkIdToReplicationCommand)
{
    ReplicationCommand& replicationCommand = pair.second;
    if (replicationCommand.HasDirtyState())
    {
        inOutputStream.Write(networkId);
        inOutputStream.Write(action, 2);        // 2 bits, not a byte
        ...
        replicationCommand.ClearDirtyState(writtenState);
    }
}
```

> CA2's packet is the same size whether anything moved or not, and every field is byte-aligned —
> hitpoints and ammo are a byte each whether they need one or not. CA3's is proportional to how
> much actually changed, and the fields are sized to their range.

**(b) Who decides where you are.** In CA2 the client tells the server its own position and the
server simply believes it:

```cpp
// CA2 — game_server.cpp, Client::PacketType::kStateUpdate
packet >> aircraft_identifier >> aircraft_position.x >> aircraft_position.y
       >> aircraft_hitpoints >> missile_ammo;
m_aircraft_info[aircraft_identifier].m_position = aircraft_position;
m_aircraft_info[aircraft_identifier].m_hitpoints = aircraft_hitpoints;
```

In CA3 the client sends *input*, and the server derives position by simulating it:

```cpp
// CA3 — RoboCatServer.cpp, Update()
MoveList& moveList = client->GetUnprocessedMoveList();
for (const Move& unprocessedMove : moveList)
{
    const InputState& currentState = unprocessedMove.GetInputState();
    float deltaTime = unprocessedMove.GetDeltaTime();
    ProcessInput(deltaTime, currentState);
    SimulateMovement(deltaTime);
}
moveList.Clear();
```

> That's the single biggest difference between the two submissions. In CA2 a modified client can
> claim any position it likes and every other client will draw it there. In CA3 the only thing a
> client can send is which keys are down; if it lies, it lies about its keyboard.

### What each transport cost me

**Say:**

> TCP in CA2 was much less code. There's no sequence number, no ack, no timeout, no in-flight queue —
> the whole `DeliveryNotificationManager` and `ReplicationManagerTransmissionData` pair,
> about 350 lines, simply doesn't exist in CA2.
>
> What it cost was control. TCP's ordering guarantee means a lost packet blocks every packet behind
> it until it's retransmitted — head-of-line blocking. For a positional update that's the wrong
> trade: by the time the retransmission lands I have a newer position anyway, and I'd rather have
> had the newer one on time. That's exactly what CA3's delivery notification is for — it reports the
> loss and re-dirties the object so the *current* state goes out, instead of re-sending the stale one.
> It also means CA3 degrades under bad conditions instead of stalling, which you can see live by
> holding Tab and pressing `+`.
>
> The other structural difference: CA2's server runs on a thread inside one of the clients, so the
> host has an advantage and there's no such thing as a dedicated server. CA3's server is its own
> executable with no window and no SFML rendering at all — it's the thing you'd actually deploy.
