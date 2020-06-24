# Applications

## DIDSession

| Test | Priority |
| ---- | -------- |
| New DID without passphrase | High |
| New DID with passphrase | High | （nothing now）
| Import DID without passphrase | High |
| Import DID with passphrase | High |
| Create DID: the process of verifying mnemonic words, click back, and then verify again | Medium |
| Import DID: the process of verifying mnemonic words, click back, and then verify again | Medium| |
| Create DID does not continue after verifying mnemonic words, use the same mnemonic words to import DID | Medium |
| Create or import DID:you can change words when the mnemonic word is wrong | High |
| import and import DID: For the same DID, you can repeat multiple times without reporting errors | High |
| Select DID | High |
| Switch from DID login with passphrase to DID login without passphrase | High |
| Switch from DID login without passphrase to DID login with passphrase | High |
| Switch from DID login with turn On and off biometrics | High |
| DID login with turn On biometrics by entering true information | High |
| DID login with turn On biometrics by entering wrong information | High |
| Select identity page: when the identity exceeds one page, you can see all the identities | Medium |
| Share DID with Contact or Other app to add friend | High |
| Sign out DID | High |

## Launcher

| Test | Priority |
| ---- | -------- |
| Launch a built-in app | High |
| Add/remove favorites from Recent Capsules | Medium |
| Open scanner | Medium |
| Open settings | High |
| Open DPoSVoting and work well | High |
| Show running apps | High |
| Close all apps from running app list | High |
| Close a app from running app list | High |
| Open app from running app list | High |
| Capsule Marketplace is built in Android, but not in ios | High |
| The built-in app is not in Recent Capsules | High |
| Notification with URL can open on click (Add friend request from friend) | High |
| Notification to distinguish between different DID sessions | Medium |
| Show notification with time and app information | High |
| Show notification from Friend | High |
| Show notification from wallet | High |
| When there is a new notification, the notification icon has a small red dot | Medium |
| Open the app from the notifications | Medium |
| After the notification has been read, the little red dot disappears | Medium |
| Notification, running, and home icon: Toggle to close the content of the previous page | Medium |

## Wallet App

| Test | Priority |
| ---- | -------- |
| Create a new wallet with single address | High |
| Create a new wallet with multiple address | High |
| Create a new wallet with passphrase | High |
| Create a new wallet without passphrase | High |
| Import a wallet from mnemonic | High |
| Import a wallet from keystore | High |
| Import a wallet with single address | High |
| Import a wallet with multiple address | High |
| Import a wallet with passphrase | High |
| Import a wallet without passphrase | High |
| Send ELA to another address | High |
| Transfer ELA from mainchain to ID chain | High |
| Send the token of ID chain to another address of ID chain | High |
| Transfer the token of ID chain to ELA mainchain | High |
| Manage multiple wallets：Add, Delete wallet | High |
| Edit wallet name | High |
| Check and copy wallet publickey | High |
| Export wallet | High |
| Export mnemonics | High |
| Change payment password | High |
| ID chain display: on, off | High |
| Add, modify, delete, and check contacts | High |

## DID App

| Test | Priority |
| ---- | -------- |
| Create a new DID | High |
| Import an existing (published) DID from mnemonic | High |
| Import a DID from the wallet app mnemonic | Medium |
| Edit a DID profile (modify + add + delete entry) | High |
| Publish an unpublished DID | High |
| Edit credentials visibility and publish a published DID | High |
| Sign data (from the DID Demo) | High |
| Show the QR code | Medium |
| Copy DID | High |
| Share DID with Contact | High |
| Share DID with other apps | High |
| Fingerprint: activate + save profile + deactivate | Medium |

## DPos Voting App

| Test | Priority |
| ---- | -------- |
| Vote for DPoS supernodes | High |
| No more than 36 supernodes voted | High |
| The number of supernodes voted exceeds 36 | High |
| Normal vote | High |
| Vote to confirm and cancel | High |
| View supernode details | High |
| Search the specified supernode for details | High |
| Check voting history | High |
| Check statistics | High |

## CRC Voting App

| Test | Priority |
| ---- | -------- |
| Vote for CR Council | High |

## QRScanner App

| Test | Priority |
| ---- | -------- |
| Scan a DID qrcode should open the friends app to add a friend | High |
| Scan a elastos scheme | High |
| Scan ELA address (intent from wallet app) | High |

## Settings App

| Test | Priority |
| ---- | -------- |
| Show all apps info | High |
| Launch Dapp | High |
| Manage Dapp permissions: open or close | High |
| Delete Dapp | High |
| Delete Dapp need to confirm | High |
| Delete Dapp：the built-in app can not be delete | High |
| Change Language | High |
| Use dark mode | High |
| Use light mode | High |
| Open Developer Options | High |
| In developer mode, switch nodes | High |
| Close Developer Options | High |
| Password manager can be opened | Medium |
| About Elastos, the link inside can jump | Medium |

## Friends App

| Test | Priority |
| ---- | -------- |
| Add a friend with published DID by using scanner | High |
| Add a friend by entering published DID in the text box | High |
| Add a friend with unpublished DID | Medium |
| Add a friend with default(did:elastos) | Medium |
| Add a friend with default(did:elastos) and delete again| Medium |
| Add a friend, delete it and add it again | Medium |
| Browse a friend's app from apps his profile | Medium |
| Share dapp with friends | Medium |
| Friends are sorted correctly | Medium |
| Modify friend's name and note | Medium |
| Add a contact and bookmark it | Medium |
| Unfavorite contact | Medium |
| Delete favorite contacts | Medium |

## PasswordManager App

| Test | Priority |
| ---- | -------- |
| Create master password | High|
| Set password unlock mode | High|
| View the password inside the capsule | High|
| Create a password category | High|
| Category names cannot be the same | High|
| Password can be added | High|
| Password name cannot be the same | High|
| Password can be modified | High|
| Password can be deleted | High|
| Need to confirm when deleting password | High|
| Password can be copied | High|
| Password hidden display or direct display | High|

## DApps Store App

| Test | Priority |
| ---- | -------- |
| Launch an app from the DApps store | Medium |

# Others

## Inter-app communication

| Test | Priority |
| ---- | -------- |
| Launch a built-in app from an external url | High |
| Launch a third party app from an external url | High |
| Sign in from the DID Demo | High |
| Sign in from nucleusconsole.com using QR code and scanner | High |
| Sign in CR website with use elastos DID (https://www.cyberrepublic.org/login):DID need to be published and then be associated with CR account| High |

## Security

| Test | Priority |
| ---- | -------- |
| Try to install an app from CLI without developer mode enabled | Medium |
| Try to install an app with no verifiable DID signature | Medium |
| Sign in from a website with an altered JWT token | Medium |
| In the process of creating or importing the mnemonic of the identity, the mnemonic, password, and master password cannot be printed in the log | High |
| In the process of creating or importing the mnemonic of wallet, the mnemonic, password cannot be printed in the log |High|
| In the process of creating or modifying the Password, the password info cannot be printed in the log |High|

## Robustness

| Test | Priority |
| ---- | -------- |
| Launch quickly saveral favorite apps from the home screen | Medium |
| Start an app and quickly exit it several times | Medium |

## Compatibility

| Test | Priority |
| ---- | -------- |
| Ensure elastOS can upgrade from the previous public release (with user data) to the new release | High |

# Tools

| Test | Priority |
| ---- | -------- |
| Create a new angular/tabs app from the cli (+build + run android/ios) | High |
| Create an app DID from the CLI (without publishing) | Medium |
| Be able to update built-in or not built-in apps using the CLI, including Launcher and DID session apps | High |
