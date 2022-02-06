# purple-whatsmeow

A libpurple/Pidgin plugin for WhatsApp. Being developed on Ubuntu 20.04. 

This is a re-write of [purple-gowhatsapp](https://github.com/hoehermann/purple-gowhatsapp/tree/gowhatsapp), switching back-ends from [go-whatsapp](https://github.com/Rhymen/go-whatsapp) to [whatsmeow](https://github.com/tulir/whatsmeow). whatsmeow is written by Tulir Asokan. It has multi-device support.

![Instant Message](/instant_message.png?raw=true "Instant Message Screenshot")

### Features

Standard features:

* Connecting to existing account via QR-code.
* Receiving messages, sending messages.
* Receiving files (images, videos, voice, document, stickers).
* Received images are displayed in the conversation window (optional).
* Sending images.
* Sending audio files as voice messages.
* Sending other files as documents.
* Fetching all contacts from account, showing friendly names in buddy list, downloading profile pictures (for [Markus "nihilus" Gothe](https://github.com/nihilus)).
* Sending receipts (configurable).
* Reasonable support for group chats by [yourealwaysbe](https://github.com/yourealwaysbe).
* Under the hood: Reasonable callback mechanism thanks to [Eion Robb](https://github.com/EionRobb).

Major differences from the go-whatsapp vesion:

* Messages are sent asynchronously.
* Incoming messages are not filtered (formerly by Daniele Rogora, now whatsmeow keeps track of already received messages internally).
* Note: Under the hood, gowhatsapp and whatsmeow use completely different prototocls. For this reason, one must establish a new session (scan QR-code) when switching. All old (non-multi-device) sessions will be invalidated. This is a technical requirement.
* Note: This is not a perfect drop-in replacement for the plug-in with the gowhatsapp back-end. For this reason, it has a different ID: `prpl-hehoe-whatsmeow`

Other improvements:

* Contact presence is regarded (buddies are online and offline).
* Typing notifications are handled.
* Logging happens via purple.
* There is an "away" state.
  * WhatsApp does not send contact presence updates while being "away".
  * Other devices (i.e. the main phone) display notifications while plug-in connection is "away".
  * Caveat emptor: Other side-effects may occur while using "away" state.

Features which are present in the go-whatsapp version but missing here:

* Geting list of participants in group chat.
* Support for proxy servers (whatsmeow limitation).

Known issues:

* Group Chats:
  * Topic (friendly name) is not set.
  * Participant lists are not populated.
  * Attachments are downloaded, but link is not shown in group conversation window (not a Purple limitation, tdlib can do it).
  * Cannot send files of any kind to groups (Purple limitation? tdlib can embed images).
  * Group chats are not listed in spectrum.
  * No notification when being added to a group directly.

Other planned features:

* Display receipts in conversation window.
* Join group chat via link.
* Sending proper video messages.
* Option to terminate session explicitly.

These features will not be worked on:

* Accessing microphone and camera for recording voice or video messages.

### Building

#### Pre-Built Binaries

* [Nightly Build](https://buildbot.hehoe.de/purple-whatsmeow/builds/) (Windows).

#### Instructions

Dependencies: 

* pidgin (libpurple glib gtk)
* pkg-config
* cmake (3.8 or later)
* make
* go (1.17 or later)
* gcc (6.3.0 or later)

This project uses CMake.

    mkdir build
    cd build
    cmake ..
    cmake --build .
    sudo make install/strip

#### Windows Specific

CMake will try to set-up a development environment automatically. 

Additional dependencies:

* [go 1.17 or newer (32 bit)](https://go.dev/dl/go1.17.5.windows-386.msi)
* [gcc (32 bit)](https://osdn.net/projects/mingw/)

go and gcc must be in `%PATH%`.

MSYS make and CMake generator "MSYS Makefiles" are recommended.  
The project can be opened using Microsoft Visual Studio 2022.  
Compiling with MSVC results in an unusable binary. NOT recommended.  

### Installation

* Place the binary in your Pidgin's plugin directory (`~/.purple/plugins` on Linux).

#### Set-Up

* Create a new account  
  You can enter an arbitrary username. 
  It is recommended to use your own internationalized number, followed by `@s.whatsapp.net`.  
  Example: `123456789` from Germany would use `49123456789@s.whatsapp.net`. This way, Pidgin's logs look sane.  

* Upon login, a QR code is shown in a Pidgin request window.  
  Using your phone's camera, scan the code within 20 seconds – just like you would do with WhatsApp Web.

#### Purple Settings

* `qrcode-size`  
  The size of the QR code shown for login purposes, in pixels (default: 256).
  
* `plain-text-login`  
  If set to true (default: false), QR code will be delivered as a text message (for text-only clients).
  
* `fetch-contacts`  
  If set to true (default), buddy list will be populated with contacts sent by server. 
  This is useful for the first login, especially.
  
* `fake-online`  
  If set to true (default), contacts currently not online will be regarded as "away" (so they still appear in the buddy list).
  If set to false, offline contacts will be regarded as "offline" (no messages can be sent).

* `send-receipt`  
  Selects when to send receipts "double blue tick" notifications:
    * "immediately": immediately upon message receival
    * "on-interact": as the user interacts with the conversation window (currently buggy)
    * "on-answer": as soon as the user sends an answer (default)
    * "never": never
    
* `inline-images`:
  If set to true (default), images will automatically be downloaded and embedded in the conversation window.
  
* `get-icons`  
  If set to true (default: false), profile pictures are updated every time the plug-in connects.
  
* `spectrum-compatibility`  
  Setting this to true (default: false) will have the following effects:
    * System messages will be treated just like normal messages, allowing them to be logged and forwarded. This only affects soft errors regarding a specific conversation, e.g. "message could not be sent".
    * After a message has been sent successfully, it is not explicitly echoed locally.
  
* `database-address`  
  whatsmeow stores all session information in a SQL database. This string will be passed to [database/sql.Open](https://pkg.go.dev/database/sql#Open) as `dataSourceName`.
  
  This setting can have place-holders:
  
  * `$purple_user_dir` – will be replaced by the user directory, e.g. `~/.purple`.
  * `$username` – will be replaced by the username as entered in the account details.
  
  default: `file:$purple_user_dir/whatsmeow.db?_foreign_keys=on&_busy_timeout=3000`  
  Folder must exist, `whatsmeow.db` is created automatically.
  
  If the address starts with `file:`, the driver will be `sqlite3` for a file-backed SQLite database. This is not recommended for multi-account-applications (e.g. spectrum or bitlbee) due to a [limitation in the driver](https://github.com/mattn/go-sqlite3/issues/209). The file-system (see addess option) must support locking and be responsive. Network shares (especially SMB) **do not work**.
  
  Other [SQLDrivers](https://github.com/golang/go/wiki/SQLDrivers) may be added upon request. As of writing, `pgx` for PostgreSQL is the only other option [supported by whatsmeow](https://github.com/tulir/whatsmeow/blob/b078a9e/store/sqlstore/container.go#L34).

### Notes

#### Attachment Handling and Memory Consumption

Attachments (images, videos, voice messages, stickers, document) are *always* downloaded as *soon as the message is processed*. The user is then asked where they want the file to be written. During this time, the file data is residing in memory multiple times:

* in the input buffer
* in the decryption buffer
* in the go → C message buffer
* in the output buffer

On systems with many concurrent connection, this could exhaust memory.

As of writing, whatsmeow does not offer an interface to read the file in chunks.
