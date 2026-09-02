# CA3 Screencast Script — Armoured Warfare

**Adam Kavanagh — D00247069 — Multiplayer Distributed Programming, CA3**

Target running time **~22 minutes**. Each section below maps to a line on the CA3
marking scheme. Timings are a guide — the ordering matters more than the exact minutes.

**Recording setup before you hit record**

- Build Release x64. Have **1 server + 3 clients** already launched and connected, plus a
  terminal ready to launch a 4th mid-recording (you need a live join for the create/replication
  section).
- Have Visual Studio open on a second monitor with these files pre-opened as tabs, in this order:
  `NetworkManager.cpp`, `NetworkManagerServer.cpp`, `NetworkManagerClient.cpp`,
  `MemoryBitStream.hpp`, `RoboCat.cpp`, `RoboCatClient.cpp`, `ReplicationManagerServer.cpp`,
  `DeliveryNotificationManager.cpp`, `ScoreBoardManager.cpp`.
- Zoom Visual Studio's text to ~150% so code is readable in a compressed recording.
- **Hold Tab** in-game to bring up the network panel — you will use it repeatedly as your
  evidence. Learn where the numbers sit on screen before recording.
- `+` / `-` add and remove 100 ms of simulated latency on a client. This is your single most
  valuable demo tool. Test it before recording.

---

## 0. Intro — 0:00–0:45

> "I'm Adam Kavanagh, D00247069. This is my CA3 submission, a UDP client–server tank
> deathmatch called Armoured Warfare, built on the RoboCat SFML framework from class.
> Free-for-all, first to ten kills wins. I'll cover the client, the server and the protocol
> between them, the structures I serialise and how I've cut them down, how state
> synchronisation works and where it breaks, persistence, and finally how this codebase
> compares to the TCP game I wrote for CA2."

Show the main menu, click Play, land in the game. Keep it moving.

---

## 1. Demo the game and the multiplayer design — 0:45–3:30
*(Marking scheme: Gameplay, 10)*

Play across two client windows side by side. Narrate over it:

- **Controls.** WASD drives the chassis, arrow keys traverse the turret independently, Space or
  Enter fires. The split between hull and turret is the core of the design — you can retreat
  while still shooting, so movement and aim are two separate decisions.
- **Fire delay.** `RoboCat::kTimeBetweenShots` is 1.0 s. Hold the fire key and show that you
  still only get one shell per second. Say the important part: **the server enforces this**, in
  `RoboCatServer::HandleShooting` (`RoboCatServer.cpp:65`). The client also knows the number,
  but only so it doesn't play a firing sound for a shot the server is going to refuse
  (`InputManager::PlayFireSoundIfOffCooldown`). A modified client cannot fire faster.
- **Health and the barrels.** Four hit points, shown top-right. Drive over an oil drum and show
  the heal. `MouseServer::HandleCollisionWithCat` only consumes the barrel if the heal actually
  landed, so driving over one at full health doesn't waste it.
- **How players tell each other apart.** Join order picks the tank model —
  `GetTankTypeForPlayerId` in `TankType.hpp` — player 1 is a Sherman, player 2 a Panzer IV, and
  it alternates from there. Colour comes from the scoreboard palette keyed off player id.
  There is no lobby or selection screen; identity is derived from the player id the server
  assigns in the welcome packet, so it can never disagree between clients.
- **Changes made for a multiplayer environment.** Be explicit here, this is a marked point:
  - The world was scaled up to 2500×1500 so fifteen tanks aren't stacked on top of each other.
  - Tanks were slowed down (`mMaxLinearSpeed` 5000 → 2000, `mMaxRotationSpeed` 100 → 70) —
    at the original speed a tank crossed the screen faster than a state packet could describe
    it, and the prediction correction was visible as constant snapping.
  - Barrels heal instead of scoring, so the only way to score is to shoot another player. That
    makes the scoreboard mean something in a free-for-all.
  - Kills, not survival, end the game — ten kills, replicated to everyone, so all fifteen
    clients agree on the winner without a separate authoritative message.

Press Tab and leave the network panel up for a beat: *"That panel is the whole talk in one
screen, and I'll come back to it."*

---

## 2. Overview of the client — 3:30–6:00
*(Marking scheme: Overview of Client, 10)*

Switch to `Client.cpp`. Walk `Client::DoFrame` (`Client.cpp:92`) top to bottom — it is the
clearest statement of the client's job:

```
InputManager::Update()            // sample input into a Move, every 0.03s
Engine::DoFrame()                 // update + predict every game object
ProcessIncomingPackets()          // read state from the server, correct ourselves
CheckForGameOver()                // decide from the replicated scoreboard
RenderManager::Render()
SendOutgoingPackets()             // ship the last 3 moves back up
```

Points to hit:

- **The client owns nothing.** It renders, samples input, and predicts. Every authoritative
  decision — who was hit, who died, who scored, when you respawn — happens on the server. The
  client only *guesses ahead* so the game feels responsive.
- **Input sampling is decoupled from frame rate.** `kTimeBetweenInputSamples` is 0.03 s
  (`InputManager.cpp:7`). Moves are sampled on a fixed clock regardless of render rate, because
  the server replays those exact moves with those exact delta times. If sampling were tied to
  frame rate, a client at 144 fps and a client at 30 fps would simulate differently from the same
  input and prediction would never converge.
- **The manager layout.** Say you split the client into single-responsibility managers:
  `WindowManager`, `TextureManager`, `FontManager`, `AudioManager`, `RenderManager`,
  `MenuManager`, `HUD`, `GameStateManager`, `InputManager`, `NetworkManagerClient`. Point at the
  file list. This is worth saying out loud for the code-structure marks.
- **The pieces I wrote.** Be honest and specific — the framework is from class, so name what's
  yours:
  - `TurretSpriteComponent` and the whole hull/turret split, including replicating
    `mTurretRotation` as part of the pose.
  - `GameStateManager` — the victory/defeat end state driven off the replicated scoreboard.
  - `HUD::RenderNetworkStats` — the Tab panel.
  - `TankType.hpp` — deriving the tank model from player id instead of a selection screen.
  - `AudioManager` and the input-driven sound layer.
  - The robustness fixes: the length clamp in `InputMemoryBitStream::Read(std::string&)`, the
    scoreboard sanity check in `ScoreBoardManager::Read`, the `nullptr` guards in
    `ReplicationManagerClient`, and the shared-pointer fix on `mTurretComponent`. These come up
    again in section 5, they're worth flagging now.

---

## 3. Overview of the server and the protocol — 6:00–9:30
*(Marking scheme: Overview of Server including discussion of protocol, 10)*

Switch to `Server.cpp`, then `NetworkManagerServer.cpp`.

**The server loop** — `Server::DoFrame` (`Server.cpp:83`):

```
ProcessIncomingPackets()   // drain the socket, route by address
CheckForDisconnects()      // 3s silence = gone
RespawnCats()              // 3s after death
Engine::DoFrame()          // authoritative simulation
SendOutgoingPackets()      // one tailored state packet per client
```

It's a separate executable, `RoboCatSFMLServer`, headless — no window, no rendering. Say that
plainly: **the server is not a player**.

**The protocol.** Draw it out as four message types, defined as FourCCs in
`NetworkManager.hpp`:

| Packet | Direction | Purpose |
|---|---|---|
| `'HELO'` | client → server | Join request, carries the player's name. Resent every 1 s until welcomed. |
| `'WLCM'` | server → client | Assigns the player id. This id is the client's identity for the whole session. |
| `'INPT'` | client → server | Sequence number, ack range, and the last 3 moves. ~30/s. |
| `'STAT'` | server → client | Sequence number, last-processed-move timestamp, scoreboard, replication records. |

Walk the handshake in code:

1. `NetworkManagerClient::SendHelloPacket` (`NetworkManagerClient.cpp:88`) — fires every second
   while `mState == NCS_SayingHello`. UDP has no connection, so the retry *is* the connection.
2. `NetworkManagerServer::ProcessPacket` (`NetworkManagerServer.cpp:30`) — looks the sender up in
   `mAddressToClientMap`. Unknown address goes to `HandlePacketFromNewClient`. **This is the
   security boundary:** anything that isn't a `'HELO'` from an unknown address is logged and
   dropped, not parsed.
3. `HandlePacketFromNewClient` (`:74`) — creates a `ClientProxy`, which is the server's entire
   model of that player: their address, their delivery notification manager, their *own*
   replication manager, their unprocessed move list, their respawn timer.
4. Then the loop at `:99` — for every object already in the world, queue a `ReplicateCreate` on
   *this new client's* replication manager. That's how a player joining ten minutes in gets a
   correct world.

**Do this live.** Launch the fourth client on camera. Show the other three windows: the new tank
appears, and the new client's window fills in with every tank and barrel already in play. Say:
*"Nothing global happened there — that catch-up is entirely per-client state on that one
`ClientProxy`."*

**Reliability on top of UDP.** Open `DeliveryNotificationManager.cpp`. The key idea: UDP gives
no delivery guarantee, so the game builds exactly the guarantee it needs and no more.

- Every packet carries a 16-bit sequence number (`WriteSequenceNumber`, `:39`).
- The receiver acks ranges, not individual packets — `AckRange::Write` writes a start plus one
  bit for "is this range longer than one", and only then 8 bits of count. A run of 40 delivered
  packets is acked in 25 bits.
- The sender keeps `mInFlightPackets` and gets a **callback** on success or failure —
  `ReplicationManagerTransmissionData::HandleDeliveryFailure` (`:16`). Show it. When an update is
  lost, the state that was in it gets marked dirty again and goes into the *next* packet.
- The critical line to say out loud: **"I don't retransmit the lost packet. I retransmit the
  current value of the state that was in it."** A stale position is worthless; a position from
  33 ms ago is what the client wants anyway. That is why this is UDP and not TCP.
- Point at `HandleUpdateStateDeliveryFailure` (`:74`) — before re-dirtying, it scans other
  in-flight packets and clears any state already resent, so a burst of loss doesn't send the same
  field five times.
- Creates and destroys, though, *are* fully reliable — `ReplicationCommand` stays in `RA_Create`
  until the ack arrives (`HandleCreateAckd`), because a missed create means a permanently invisible
  tank.

---

## 4. Structures, serialisation and compression — 9:30–13:30
*(Marking scheme: Structures and compression used to achieve state synchronisation, serialization, 15)*

This is the highest-value section. Have the numbers ready.

**The foundation: bit-level streams.** Open `MemoryBitStream.hpp`. `OutputMemoryBitStream::Write`
takes a bit count, not a byte count. Nothing is padded to a byte boundary. Say why that matters:
in CA2 with `sf::Packet`, a bool cost 8 bits. Here it costs 1.

**Client → server: the input packet.** Open `NetworkManagerClient::SendInputPacket` (`:220`) and
`InputState::Write` (`InputState.cpp:32`).

- `WriteSignedBinaryValue` is the trick worth pointing at. The input axes are only ever -1, 0 or
  +1, so instead of a 32-bit float it writes **1 bit if the axis is zero, 2 bits otherwise**.
  Three axes plus a fire bit: **4 to 7 bits of actual input**.
- The move count is written in 2 bits (`:242`) because it's only ever 0–3.
- **Only the last 3 moves are sent**, and they're sent *again* in the next packet. That is the
  redundancy strategy: rather than acking input, just resend it. Losing three input packets in a
  row is what it takes to lose a move.

| Input packet field | Bits |
|---|---|
| `'INPT'` FourCC | 32 |
| Sequence number | 16 |
| Ack range | 1–26 |
| Move count | 2 |
| 3 × Move (input 4–7 + timestamp 32) | 108–117 |
| **Total** | **~185 bits = 24 bytes** |

Plus 28 bytes of IP + UDP header = **~52 bytes on the wire, 30 times a second = ~1.5 KB/s per
client**. Fifteen clients is about 23 KB/s inbound at the server. Upstream is a non-issue.

**Server → client: the state packet.** Open `NetworkManagerServer::SendStatePacketToClient`
(`:158`), then `RoboCat::Write` (`RoboCat.cpp:203`).

The pattern in `RoboCat::Write` is the thing to explain: **one presence bit per field group**,
and the field only goes in the packet if the dirty mask says it changed. A tank that is driving
sends its pose and nothing else — no player id, no colour, no health.

Compression choices to name individually:

- **Health in 4 bits** (`RoboCat.cpp:270`). `kMaxHealth` is 4, so 4 bits is generous. Mention the
  bug this caused and how you fixed it — two shells landing in the same frame drove health
  negative, and a negative number written into 4 bits wrapped around to full health. The guard is
  the `DoesWantToDie()` early-out at the top of `TakeDamage` (`RoboCatServer.cpp:99`). This is a
  genuinely good story: it shows you understand that compression has consequences.
- **Yarn player id in 8 bits** (`Yarn.cpp:57`) — 255 players is plenty, 32 bits was waste.
- **Replication action in 2 bits** (`ReplicationManagerServer.cpp:69`) — Create/Update/Destroy.
- **Thrust direction in 1–2 bits** rather than a float.
- **Destroy records carry no payload at all** — the 2-bit action is the whole message.

| State packet, 15 players, ~5 shells in flight | Bytes |
|---|---|
| `'STAT'` + sequence + move timestamp | 11 |
| Scoreboard (15 entries × 32 B) | 484 |
| 15 tank pose updates (29 B each) | 435 |
| 5 shell pose updates (25 B each) | 123 |
| **Total** | **~1053 bytes** |

At 30 packets/s that is **~32 KB/s down per client, ~250 kbit/s** — and **~3.8 Mbit/s upstream at
the server** for fifteen clients. Verify this live: hold Tab, read the "in" and "out" B/s figures
off the panel on camera.

**How many players can the server support — and be honest about the ceiling.**

The hard limit isn't CPU, it's the datagram. `NetworkManager::ReadIncomingPacketsIntoQueue`
(`NetworkManager.cpp:54`) receives into a **1500-byte buffer**, one Ethernet MTU. A state packet
bigger than that gets truncated, not fragmented and reassembled. At 15 players I'm at ~1053
bytes — about **70% of the budget**. Each extra player adds roughly 61 bytes (32 scoreboard + 29
tank), so the layout as it stands runs out at **roughly 21 or 22 players**. The riskiest moment is
a mass simultaneous respawn, where every tank sends a full ~50-byte create at once.

**Where the next win is — and be specific, this is the "attempts to limit data" mark.**

The single worst offender is visible right in the table: **the scoreboard is 46% of the packet and
it is resent in full, 30 times a second, even though it only changes on a kill, a join or a
leave.** Open `ScoreBoardManager::Write` (`:84`) and `Entry::Write` (`:128`) and show it — colour
as three 32-bit floats, a 32-bit player id, the player's **name with a 32-bit length prefix**, and
a 32-bit score. Every entry, every packet.

Say what you would do, in order of value:

1. **A single dirty bit on the scoreboard.** One bit saying "unchanged since last packet" would
   take the 15-player packet from ~1053 bytes to about **570** — a 46% cut for a few lines of
   code. Names should be sent once at join, not thirty times a second.
2. **Quantise positions.** The world is 2500×1500. At 0.1-unit precision that is 15 bits of x and
   14 of y instead of 64. Rotation and turret rotation at 1° precision are 9 bits each instead of
   32. `ConvertToFixed`/`ConvertFromFixed` are already sitting at the top of `MemoryBitStream.hpp`
   — the framework provides this and I haven't used it. A tank pose would drop from 192 bits to
   about 47.
3. **Stop sending velocity.** `AdjustVelocityByThrust` is deterministic given rotation and thrust
   direction, both of which are already in the packet. That's 64 bits per tank per packet the
   client could derive instead of receive.
4. **Derive colour from player id.** It's picked from a 4-entry palette keyed off player id
   (`ScoreBoardManager.cpp:70`) and the client *already* derives the tank model from player id the
   same way. 96 bits of float RGB, sent twice — once in the object state and again in the
   scoreboard entry — for something worth 2 bits.
5. **Shrink the network id.** 32 bits per replication record for ids allocated sequentially from 1.
   12 bits covers 4096 live objects.

Together those would put a 15-player packet somewhere near **200 bytes** and take the ceiling well
past thirty players. Frame it exactly like that: *"I know where the fat is and I can cost it."*

---

## 5. Game state synchronisation, and where it breaks — 13:30–17:30
*(Marking scheme: Game state synchronization, 15 — including the "known issues" requirement)*

**The three mechanisms.** Say these are three separate things people conflate:

**(a) Object lifetime — the replication manager.** Each `ClientProxy` owns a
`ReplicationManagerServer` holding a `networkId → ReplicationCommand` map. A command is a dirty
mask plus an action. `NetworkManagerServer::SetStateDirty` (`:318`) fans a change out to every
client's map. `ReplicationManagerServer::Write` (`:45`) walks the map and writes only commands
with dirty state. Two guards worth pointing at, both bugs I hit:
- `SetStateDirty` uses `find`, not `operator[]` — otherwise it would create a command for an
  object the client has never heard of.
- `Write` skips create/update for an object no longer registered (`:60`) — a shell can die between
  two writes, and there's nothing to say about it until the destroy goes out.

**(b) Client-side prediction and rollback.** This is the important one. Open
`RoboCatClient::Update` (`:46`) and `DoClientSidePredictionAfterReplicationForLocalCat` (`:200`).

The sequence, say it slowly:

1. Client samples a move, applies it locally **immediately**, keeps it in `mMoveList`.
2. Move goes to the server; the server replays it in `RoboCatServer::Update` (`:16`) with the same
   delta time and the same `ProcessInput`/`SimulateMovement` code — **that shared code lives in
   `RoboCat.cpp`, compiled into both projects.** That's why prediction converges.
3. The state packet comes back carrying the timestamp of the last move the server processed.
   `ReadLastMoveProcessedOnServerTimestamp` (`NetworkManagerClient.cpp:133`) drops every acked
   move from the list — and computes RTT off it as a free by-product.
4. The client snaps to the authoritative pose, then **replays every remaining unacked move** on
   top of it. That is rollback-and-replay.
5. If the replay lands somewhere different, `InterpolateClientSidePrediction` (`:222`) blends
   toward the correction rather than teleporting — 10% a frame for your own tank.

**(c) Remote tanks.** `DoClientSidePredictionAfterReplicationForRemoteCat` (`:286`) extrapolates
another player's tank forward by a **full RTT** in 1/30 s chunks, because the pose you just
received is already one trip old.

**Demo this properly — this is the money shot of the whole recording.** On one client, press `+`
three times to add 300 ms of simulated latency. Hold Tab and show round-trip climb on the panel.
Then:
- Drive **your own** tank. It stays responsive — that's prediction. There's no input lag even at
  300 ms.
- Watch the **other** tank from the laggy client. It overshoots when the other player stops, then
  eases back. Say exactly why: extrapolation runs forward a full RTT, and the client only learns
  about the stop when the zero-velocity update arrives.
- Press `-` back down and show it settle.

**Known synchronisation issues.** Be candid — the brief explicitly asks for this and it reads as
competence, not weakness:

1. **Remote tanks overshoot and rubber-band under latency.** Cause above. The fix is interpolating
   between two buffered past states instead of extrapolating from one — you play a fixed delay
   behind, but you never show a position the server never had.
2. **Hits look late.** Shells move on both sides but collisions are resolved **server-only**
   (`YarnServer::HandleCollisionWithCat`). At 300 ms you see your shell visibly pass through a
   tank before the health drop and the destroy arrive. The real fix is lag compensation —
   rewinding tank positions to the shooter's view of the world — which is out of scope here, but
   name it.
3. **Out-of-order packets are dropped, not reordered.** `ProcessSequenceNumber` (`:83`) discards
   anything below the expected sequence number. On a link that reorders, that shows up as extra
   effective packet loss. A small reorder buffer would recover it.
4. **State is only sent when the client's input arrives.** `SendOutgoingPackets` (`:131`) only
   sends if `IsLastMoveTimestampDirty`. A client whose input packets are all being lost stops
   receiving state entirely and then times out at 3 s. `UpdateAllClients` exists and would fix
   this — it isn't wired into the frame loop.
5. **Prediction runs collisions against predicted positions.** The local replay collides against
   where the client *thinks* other tanks are. Tank-on-tank contact is where prediction most often
   diverges — that's the `"Move replay ended with incorrect rotation"` log at
   `RoboCatClient.cpp:226`.
6. **Game over is decided independently on each client** from the replicated scoreboard
   (`GameStateManager::CheckForGameOver`). No authoritative end message, so clients can end a frame
   or two apart. It's self-correcting only because the whole scoreboard is resent every packet —
   which is the one place that 484-byte cost buys something.

**Malformed-packet hardening.** Worth 30 seconds because the causes are all sync-related. A
misaligned stream hands you a nonsense value that gets read as a length or a count, and the crash
happens far from the actual bug. Show:
- `InputMemoryBitStream::Read(std::string&)` clamping the element count to the bits actually left
  in the packet.
- `ScoreBoardManager::Read` (`:101`) rejecting an entry count the remaining packet couldn't
  possibly hold.
- `ReplicationManagerClient` returning `false` on an unknown class id, an unknown action, or an
  update for an object it doesn't have — because once you don't know a record's size, **the rest
  of the packet is unreadable and the only safe move is to stop**.

Say the general principle: *"A UDP client has to treat every packet as hostile input. It's one
`recvfrom` away from anything on the network."*

---

## 6. Game persistence — 17:30–18:30
*(Marking scheme: Game persistence, 10)*

> ⚠️ **Read the note at the end of this document before recording this section.** There is no
> persistence code in the repository as it stands. Do not claim otherwise on camera.

**If you add persistence before recording** (strongly recommended — it's 10 marks for roughly
40 lines), cover:
- What is saved: the high score — top kill counts with player names — written server-side.
- Where: a file next to the server executable, written on game over and on clean shutdown.
- When it's read: at server start-up, so the record survives a restart.
- Show the file on disk with a real score in it, then restart the server and show the record
  persisting.

**In-game state that already survives events**, and worth mentioning either way:
- Score and identity live on the **server**, in `ScoreBoardManager` and the `ClientProxy`, not on
  the client. A player who dies keeps their score — `Server::HandleLostClient` is what removes an
  entry, not death — and respawns after 3 s via `ClientProxy::RespawnCatIfNecessary`, keeping
  their id, colour, tank model and kill count.
- A client that reconnects gets a fresh player id and a fresh entry. Session identity is the
  socket address, so persistence across reconnects would need a name-keyed lookup in
  `HandlePacketFromNewClient`.

---

## 7. CA2 versus CA3 — 18:30–21:30
*(Marking scheme: Comparison of the CA2 and CA3 codebases, 10)*

Have both solutions open. Lead with the high-level point, then show code — the brief asks for
exactly that order.

**The high-level difference: who is allowed to be right.**

In CA2 the **client was authoritative over its own aircraft**. Open
`multiplayer_gamestate.cpp:241` — every 1/20 s each client packs `kStateUpdate` with its own
aircraft's *position, health and ammo* and sends it up. The server stores that in
`m_aircraft_info` (`game_server.cpp:314`) and rebroadcasts it. The server was a **relay** that
happened to also spawn enemies. Any client could have claimed any position or any health value.

In CA3 the client sends **intent only** — three axes and a fire bit, 4 to 7 bits. It never sends a
position. The server simulates, and the client's own view of itself is a prediction it has to
reconcile. That single change is what forced everything else: prediction, rollback, replay,
per-object dirty state, delivery notification.

**Then the concrete comparisons:**

| | CA2 (TCP) | CA3 (UDP) |
|---|---|---|
| Transport | `sf::TcpListener` + one `sf::TcpSocket` per peer | one non-blocking `UDPSocket`, clients keyed by `SocketAddress` |
| Server process | a `std::thread` inside the client (`GameServer::ExecutionThread`) | separate headless executable |
| Serialisation | `sf::Packet` `<<`/`>>`, byte-aligned | `OutputMemoryBitStream`, bit-aligned |
| Authority | client-authoritative position | server-authoritative, client predicts |
| State sent | full state of every aircraft to everyone, 20 Hz | per-client dirty fields only, ~30 Hz |
| Reliability | TCP's, all-or-nothing | `DeliveryNotificationManager` — reliable creates/destroys, latest-value updates |
| Loss handling | none needed, none possible to tune | explicit: re-dirty and resend the *current* value |
| Timeout | 1 s (`m_client_timeout`) | 3 s (`mClientDisconnectTimeout`) |
| Message tag | `uint8_t` enum, 1 byte | 32-bit FourCC — readable in a hex dump, but 4× the cost |

**Code examples to put on screen, side by side:**

1. **Serialisation cost.** CA2 `game_server.cpp:481` —
   `packet << id << pos.x << pos.y << hitpoints << missile_ammo` — 11 bytes per aircraft, every
   aircraft, every tick, whether it moved or not. Next to `RoboCat::Write` with its presence bits
   and 4-bit health. Then be fair about it: CA2's `kUpdateClientState` for 15 aircraft is about
   **171 bytes**; mine is about **1053**. CA2 sent *less*. But it sent less because it *knew* less
   — no velocity, no rotation, no turret, no create/destroy protocol, no delivery guarantees, and
   a client that could lie about all of it. Making that point yourself is worth more than pretending
   CA3 won on raw bytes.

2. **Head-of-line blocking.** In CA2 a dropped segment stalls **everything** behind it —
   TCP won't deliver packet N+1 until N is retransmitted, so one lost position update freezes
   every later update too. In CA3 a lost packet costs exactly what was in it, and the next packet
   33 ms later carries fresher values. Point at `HandleUpdateStateDeliveryFailure` again: I'm not
   resending the lost data, I'm resending the *current* data. TCP structurally cannot do that.

3. **Per-client state.** CA2's `SendToAll` (`game_server.cpp:461`) builds one packet and sends the
   identical bytes to everyone. CA3 builds a **different packet per client** from that client's own
   `ReplicationManagerServer` and its own ack history. More server work, but it means a client that
   just joined and a client that's been playing ten minutes each get exactly what they need.

4. **What TCP gave me for free that I had to write.** Sequence numbers, acks, timeouts,
   connection setup and teardown — `DeliveryNotificationManager`, `AckRange`, `InFlightPacket` and
   the hello/welcome handshake are all code that `sf::TcpSocket` made unnecessary in CA2. That's
   the honest cost of UDP: roughly 400 lines to rebuild a *weaker* guarantee — deliberately
   weaker, because that's the one the game actually wants.

5. **Structure.** CA2 was a scene-graph game with networking bolted onto a `MultiplayerGameState`;
   the same `World` ran on both sides and the netcode leaked into game states. CA3 splits into
   three projects — shared, client, server — and shares the simulation itself
   (`RoboCat::ProcessInput`, `SimulateMovement`) rather than sharing the scene. That's not stylistic:
   **prediction only works because the identical function runs on both machines.**

---

## 8. Version control and wrap-up — 21:30–22:30
*(Marking scheme: Screencast, version control, code structure, 10)*

- Show the GitHub repository and `git log`. Point at the incremental history — lobby state,
  disconnect fix, tank models, fire delay and kill counter, the network panel, barrels healing —
  each a self-contained change with a message describing *why*, not just what.
- Show the three-project solution structure in Solution Explorer and say the one-line rationale:
  shared code that must run identically on both machines lives in `RoboCatSFML`; anything that
  renders lives in the client; anything authoritative lives in the server. Nothing in
  `RoboCatSFML` includes SFML graphics — that's what makes the headless server possible.
- Acknowledge the AI use, as the brief requires: the framework is the in-class RoboCat example,
  and Claude was used as a development aid throughout. It's already stated in the README — say it
  on camera too.
- Close on the honest summary: *"Fifteen players at about 32 KB/s each, a server-authoritative
  simulation with prediction and rollback, reliable object lifetime over unreliable transport,
  and I can tell you exactly which 484 bytes I'd cut first."*

---

## Pre-flight checklist

- [ ] Name and student ID at the top of every source file (submission requirement 1 — **not
      currently done in this repo**)
- [ ] Persistence implemented, or honestly addressed on camera (see note below)
- [ ] DkIT cover sheet completed (critical element — work is not marked without it)
- [ ] Server + 4 clients tested together at the recording resolution
- [ ] `+`/`-` latency simulation tested
- [ ] Tab network panel legible at recording resolution
- [ ] Audio levels checked — game audio ducked under voice, or muted with `M`
- [ ] Repo link and screencast link in the submission zip

---

## ⚠️ Two gaps to close before you record

**1. There is no persistence in the codebase.** A search for file I/O across all three projects
returns nothing. The brief says *"it is enough to save the high score"* and the marking scheme
allocates **10 marks** to game persistence. As it stands there is nothing to demonstrate. This is
a small job — write the top scores from `ScoreBoardManager` to a file server-side on game over,
read it back at start-up — and it is the highest marks-per-line change available. Interestingly,
CA2 *did* persist something: `multiplayer_gamestate.cpp:17` reads and writes the last server IP to
`ip.txt`.

**2. No source file carries a name and student ID.** Submission requirement 1 states every source
file must have them at the top. Zero files currently do. This is mechanical to fix across the
files you wrote or substantially modified.
