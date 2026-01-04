# Changelog

All notable changes to the MultiplayerSessions plugin will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

---

## [0.2.5] - 2026-01-04

### Added

- **Host Management**

  - `UpdateLobbySettings()` - Modify lobby max players, visibility, and password at runtime
  - `SetLobbyVisibility()` - Quick toggle between public/private with optional password
  - `KickPlayer()` - Remove players from lobby with custom reason
  - `GetLobbyPlayer()` - Query individual player info by UniqueNetId

- **Player Leave Events**
  - Persistent `SessionParticipantLeftDelegate` for reliable player leave detection
  - `OnPlayerLeftLobby` delegate fires for all leave scenarios (kick, disconnect, voluntary)
  - `ELobbyLeaveReason` enum for accurate leave reason tracking

### Fixed

- Menu widget delegate cleanup in `NativeDestruct()` to prevent crashes
- Null pointer dereference in `OnJoinSession`
- Missing `MultiplayerSessionsTypes.h` include in Menu.h

### Deferred

- `TransferHost()` - Requires direct Steam API access, stubbed for future implementation

---

## [0.2.0] - 2026-01-04

### Added

- **Lobby System** - Full lobby implementation with global visibility (no Steam regional restrictions)

- **Data Structures** (`MultiplayerSessionsTypes.h`)

  - `FLobbyInfo` - Lobby metadata (ID, host name, player counts, visibility, ping)
  - `FLobbyPlayerInfo` - Player metadata (ID, name, host status)
  - `FLobbySettings` - Lobby configuration (max players, visibility, password)
  - `ELobbyJoinResult` - Join operation results
  - `ELobbyLeaveReason` - Player leave reasons

- **Lobby Delegates**

  - `MultiplayerOnLobbyCreated` - Lobby creation result
  - `MultiplayerOnLobbyListUpdated` - Lobby search results
  - `MultiplayerOnLobbyJoinComplete` - Join attempt result
  - `MultiplayerOnPlayerJoinedLobby` - Player joined event
  - `MultiplayerOnPlayerLeftLobby` - Player left event
  - `MultiplayerOnKickedFromLobby` - Local player kicked event
  - `MultiplayerOnHostMigration` - Host changed event
  - `MultiplayerOnLobbySettingsUpdated` - Settings modified event

- **Lobby Operations**

  - `CreateLobby()` - Create lobby with global visibility and optional password protection
  - `FindLobbies()` - Search for lobbies globally (not restricted to Steam friends/region)
  - `JoinLobby()` - Join lobby with password validation

- **Lobby Query Methods**

  - `GetCurrentLobbyInfo()` - Get current lobby metadata
  - `GetLobbyPlayers()` - Get all players in lobby
  - `IsLobbyHost()` - Check if local player is host
  - `IsInLobby()` - Check if in a lobby
  - `LeaveLobby()` - Leave current lobby

- **Security**
  - MD5 password hashing for private lobbies
  - Pre-join validation (lobby exists, not full, correct password)

### Technical Notes

- Uses `bUsesPresence = false` for global lobby visibility
- Uses `bUseLobbiesIfAvailable = true` for Steam Lobbies API
- Backward compatible with existing session methods (deprecated but functional)

---

## [0.1.0] - Initial Release

### Added

- **Session System** (Legacy)

  - `CreateSession()` - Create online session
  - `FindSessions()` - Search for sessions
  - `JoinSession()` - Join a session
  - `DestroySession()` - Destroy current session
  - `StartSession()` - Start the session

- **Session Delegates**

  - `MultiplayerOnCreateSessionComplete`
  - `MultiplayerOnFindSessionsComplete`
  - `MultiplayerOnJoinSessionComplete`
  - `MultiplayerOnDestroySessionComplete`
  - `MultiplayerOnStartSessionComplete`

- **Menu Widget** (`UMenu`)
  - Blueprint-callable `MenuSetup()` for easy integration
  - Host and Join button functionality
  - Automatic input mode switching
