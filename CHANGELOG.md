# Changelog

All notable changes to the MultiplayerSessions plugin will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

---

## [0.3.0] - 2026-01-12

### Added

- **Complete Lobby UI System**

  - `CreateLobbyWidget` - Configurable lobby creation form

    - Max players selection (2-16 via spin box)
    - Public/Private lobby toggle with real-time password field visibility
    - Password input with validation (min 4 characters for private lobbies)
    - Input validation with error feedback
    - Loading state during creation
    - Cancel and Create buttons with proper state management

  - `LobbyListWidget` - Scrollable lobby browser

    - Real-time lobby search and display
    - Refresh button for manual search
    - Loading indicator during search
    - Empty state messaging
    - Status text showing lobby count
    - Back button navigation to main menu

  - `LobbyEntryWidget` - Individual lobby row display

    - Host name, player count (e.g., "3/5"), and ping display
    - Lock icon for private lobbies
    - Hover effects for visual feedback
    - Visual distinction between public and private lobbies
    - Click-to-join functionality

  - `PasswordInputWidget` - Modal password entry dialog
    - Context-aware display (shows target lobby host name)
    - Masked password input field
    - Real-time password validation
    - Dynamic join button state (enabled only when valid)
    - Error display for incorrect passwords
    - Cancel functionality

- **Menu System Integration**

  - WidgetSwitcher-based view management
  - Three distinct views: Main Menu, Create Lobby, Lobby Browser
  - Enum-based view switching for type safety
  - Seamless transitions between views

- **Complete Join Flow**
  - Public lobby direct join (no password required)
  - Private lobby password prompt
  - Comprehensive error handling:
    - Success → Client travel to lobby level
    - Wrong Password → Keep dialog open, show error
    - Lobby Full → Close dialog, display error message
    - Lobby Not Found → Auto-refresh lobby list
    - Connection Failed → Generic error feedback
  - Client travel implementation after successful join

### Changed

- **Menu Widget** - Complete overhaul
  - Migrated from simple button panel to full WidgetSwitcher architecture
  - Added widget lifecycle management (Setup/Construct/Destruct)
  - Integrated all child widgets with delegate-based communication
  - BindWidget pattern for type-safe UI references

### Fixed

- Widget alignment issues in MenuSwitcher
- Blueprint serialization errors during packaging
- Password visibility toggle logic in CreateLobbyWidget
- Proper delegate cleanup in all widget destructors

### Technical

- Architecture: Delegate-based widget communication
- Pattern: BindWidget for type-safe references
- Lifecycle: Proper Setup/NativeConstruct/NativeDestruct flow
- Testing: Verified with NULL OSS (editor) and Steam OSS (packaged builds)

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
