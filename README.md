# Lure

Easy to use client hooks for old Roblox clients

## Classes

Lure is included with three sample classes, but you may add your own easily.

- **Http**: Detours `Http::trustCheck` and `Http::httpGetPostWinInet`. Replaces the trustcheck with a custom check wherein you can include your own domains. Also redirects the `/asset`, `/Thumbs/Asset.ashx`, and `/Thumbs/Avatar.ashx` endpoints to their modern Roblox counterparts.
- **Crypt**: Adds SHA256 support for signature verification and patches a buffer overflow.
- **Context**: Adds verbose (or, at the very least, easier-to-understand) error messages for permission checks. **This is only enabled when Lure is compiled as Debug.**

Adding your own hooks is as easy as using [IDA Pro](https://hex-rays.com/ida-pro/) to find the subroutine address and comparing it against the function signature that you are looking for. Reading the existing hooks is recommended for creating your own.

## Internals

Lure's method of patching Roblox clients is quite simple. It works by detouring a subroutine of the application at the address you specify to a function defined within Lure itself. However, certain Roblox clients use an address obfuscation method via VMProtect that makes detours during runtime a bit more difficult. Luckily, Lure can still detour even if the client you're patching obfuscates addresses. **If the client you're applying Lure to obfuscates addresses during runtime, define the `RESOLVE_OFFSETS` preprocessor macro in the configuration header file.**

To go into further detail, VMProtect offsets the entire memory of the Roblox client by a random address during startup. The offset range is within `0x00000000` through `0x00FF0000`. An example scenario for this would be finding the address for the `Http::trustCheck` subroutine. If the entrypoint of the program is `0x00BF1000`, then the offset is `0x00BF000`. So, the offsetted subroutine address would look something like `0x00DF20A0`. To get the true address, you would do simple arithmetic; `0x00DF20A0` minus `0x00BF000` is equal to `0x002020A0`. `0x002020A0` would be the actual address of `Http::trustCheck` and that would be the address you plug into Lure.

## Usage

You'll need Visual Studio to build Lure. [vcpkg](https://github.com/microsoft/vcpkg) is used for packages. For patching the import table, [StudPE](http://www.cgsoftlabs.ro/studpe.html) is recommended.

1. Configure and build Lure
2. Copy the output DLL file to where you are keeping the Roblox executable (usually named `RobloxApp.exe`)
3. Open the Roblox executable with StudPE (File -> Open PE File)
4. Go to the **Functions** tab
5. Right click in the left panel and click **Add New Import**.
6. Click **Dll Select** and browse to `Lure.dll` in your Roblox folder
7. Click **Select func.** and select `import` from the list that appears
8. Click `Add to list`, then click `ADD` at the bottom
9. Close the program by clicking `OK`. This will also save the patches to the executable file

## Configuration

For finding hook addresses, [x64dbg](https://x64dbg.com/) or [IDA Pro](https://hex-rays.com/ida-pro/) is recommended.

| Name                       | Expected value                                                                                                                                                                                                                    |
| -------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ALLOWED_WILDCARD_DOMAINS` | Domains that are allowed no matter the subdomain; i.e. if `google.com` was whitelisted, `google.com`, `a.google.com`, and `a.b.google.com` would all be valid.                                                                    |
| `ALLOWED_DOMAINS`          | Domains that are allowed only if the domain *exactly* matches; i.e. if `roblox.com` was whitelisted, `roblox.com` would be valid whereas `test.roblox.com` would not be.                                                          |
| `ALLOWED_SCHEMES`          | URL schemes that are allowed; `http`, `https`, etc. This is only for proper URLs.                                                                                                                                                 |
| `ALLOWED_EMBEDDED_SCHEMES` | **Don't modify this unless you know what you're doing!** If a passed URI begins with this scheme, then the entire URL is valid; the data thereafter is irrelevant. This is only used for things like JavaScript and IE resources. |
| `PUBLIC_KEY`               | The Base64 encoded public key for signature verification. Default is the Roblox public key.                                                                                                                                       |

Other preprocessor configuration are just addresses to subroutines. You may define `RESOLVE_OFFSETS` in case the client obfuscates address locations on startup; Lure will find the addresses for you.

Notes:
- The signature public key is compiled with the DLL; you needn't hex-edit the executable since all script signature verification is delegated to Lure.
- You must find addresses *specifically for the version of Roblox you're patching for*; Lure is included with sample addresses for an August 2010 client.

## License

Lure is licensed under the MIT license. A copy of the license [has been included](https://github.com/orcfoss/Lure/blob/trunk/LICENSE) with Lure.

Lure borrows parts of (ndoesstuff/JoinScriptUrlImpl)[https://github.com/ndoesstuff/JoinScriptUrlImpl], a project also licensed under the MIT license..