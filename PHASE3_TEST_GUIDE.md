# MoonOS Phase 3 - Test Guide 🧪

**Date**: Phase 3 Complete  
**Status**: Ready for Testing  
**Commands Under Test**: 7 new file system commands

---

## 🚀 Quick Start

### Build & Run
```bash
$ cd /workspaces/MoonOS
$ make clean
$ make
$ make run
```

**Expected Boot Message:**
```
[KERNEL] MoonOS 64-bit kernel starting...
[MEMORY] PMM initialized - 112 MB available
[MEMORY] VMM initialized - paging enabled
[GDT] GDT loaded and activated
[IDT] IDT loaded with exception handlers
[KEYBOARD] PS/2 Keyboard driver initialized
[FS] MoonFS Mounted.

╔════════════════════════════════════════╗
║     Welcome to MoonOS CLI v0.1.0       ║
║     No GUI - Pure command line OS      ║
║     File System Ready!                 ║
╚════════════════════════════════════════╝

Type 'help' for available commands.

moonos:/>
```

✅ If you see this, boot was successful!

---

## 📋 Test Plan

### Test Set 1: Basic File Commands

#### Test 1.1: Create File
```bash
moonos:/> touch myfile.txt
[FS] File created: myfile.txt

moonos:/> ls
--- File List ---
myfile.txt (0 bytes)
```
✅ **Expected**: File appears in listing

#### Test 1.2: Delete File
```bash
moonos:/> rm myfile.txt
[FS] Deleted: myfile.txt

moonos:/> ls
--- File List ---
(empty)
```
✅ **Expected**: File removed from listing

#### Test 1.3: Create Multiple Files
```bash
moonos:/> touch file1.txt
[FS] File created: file1.txt

moonos:/> touch file2.txt
[FS] File created: file2.txt

moonos:/> touch file3.txt
[FS] File created: file3.txt

moonos:/> ls
--- File List ---
file1.txt (0 bytes)
file2.txt (0 bytes)
file3.txt (0 bytes)
```
✅ **Expected**: All files listed

---

### Test Set 2: Directory Commands

#### Test 2.1: Create Directory
```bash
moonos:/> mkdir Documents
[FS] Directory created: Documents

moonos:/> ls
--- File List ---
Documents/ (0 bytes)
```
✅ **Expected**: Directory marked with "/" suffix

#### Test 2.2: Change Directory
```bash
moonos:/> cd Documents
[FS] Changed directory to: Documents

moonos:/Documents> pwd
Current directory: Documents
```
✅ **Expected**: 
- Prompt changes to show directory
- pwd confirms location

#### Test 2.3: Navigate Back to Root
```bash
moonos:/Documents> cd /
[FS] Changed directory to: /

moonos:/> pwd
Current directory: /
```
✅ **Expected**: Can return to root with "cd /"

#### Test 2.4: List Files in Subdirectory
```bash
moonos:/> mkdir Projects

moonos:/> cd Projects
[FS] Changed directory to: Projects

moonos:/Projects> touch code.cpp
[FS] File created: code.cpp

moonos:/Projects> ls
--- File List ---
code.cpp (0 bytes)

moonos:/Projects> cd /
[FS] Changed directory to: /

moonos:/> ls
--- File List ---
Projects/ (0 bytes)
(code.cpp should NOT appear here)
```
✅ **Expected**: Files stay in their directory | Directory shows only children

---

### Test Set 3: Complex Scenarios

#### Test 3.1: Nested Directories
```bash
moonos:/> mkdir home
[FS] Directory created: home

moonos:/> cd home
[FS] Changed directory to: home

moonos:/home> mkdir user
[FS] Directory created: user

moonos:/home> ls
--- File List ---
user/ (0 bytes)

moonos:/home> cd user
[FS] Changed directory to: user

moonos:/home/user> pwd
Current directory: user
```

⚠️ **Note**: pwd shows "user" only (not full path) - limitation noted for Phase 4

---

#### Test 3.2: Mixed Files and Directories
```bash
moonos:/> mkdir docs
[FS] Directory created: docs

moonos:/> touch README.txt
[FS] File created: README.txt

moonos:/> touch data.bin
[FS] File created: data.bin

moonos:/> ls
--- File List ---
docs/ (0 bytes)
README.txt (0 bytes)
data.bin (0 bytes)
```
✅ **Expected**: Dirs and files coexist | Dirs marked with "/" suffix

---

#### Test 3.3: File Operations in Subdirectory
```bash
moonos:/> mkdir work
[FS] Directory created: work

moonos:/> cd work
[FS] Changed directory to: work

moonos:/work> touch project.txt
[FS] File created: project.txt

moonos:/work> touch notes.txt
[FS] File created: notes.txt

moonos:/work> ls
--- File List ---
project.txt (0 bytes)
notes.txt (0 bytes)

moonos:/work> rm notes.txt
[FS] Deleted: notes.txt

moonos:/work> ls
--- File List ---
project.txt (0 bytes)

moonos:/work> cd /
[FS] Changed directory to: /

moonos:/> ls
--- File List ---
work/ (0 bytes)
```
✅ **Expected**: File operations isolated per directory

---

### Test Set 4: Error Cases

#### Test 4.1: Create Duplicate
```bash
moonos:/> touch test.txt
[FS] File created: test.txt

moonos:/> touch test.txt
[FS] File already exists: test.txt / Type: FILE or [FS] Error creating file
```
✅ **Expected**: Error message or ignored

#### Test 4.2: Delete Non-Existent File
```bash
moonos:/> rm nonexistent.txt
[FS] File not found: nonexistent.txt or similar error
```
✅ **Expected**: Error message

#### Test 4.3: Change to Non-Existent Directory
```bash
moonos:/> cd nosuchdir
[FS] Directory not found: nosuchdir or similar error
```
✅ **Expected**: Error message, prompt unchanged

#### Test 4.4: Delete Non-Empty Directory
```bash
moonos:/> mkdir folder
[FS] Directory created: folder

moonos:/> cd folder
[FS] Changed directory to: folder

moonos:/folder> cd /
[FS] Changed directory to: /

moonos:/> rm folder
(Should fail or show warning about directory not empty)
```
✅ **Expected**: Can't delete non-empty directory

---

### Test Set 5: Command Integration

#### Test 5.1: Using Help to Find Commands
```bash
moonos:/> help
Available commands:
...
ls              List directory contents
mkdir           Create a new directory
cd              Change directory to
pwd             Print working directory
touch           Create a new file
cat             Show file contents
rm              Delete file or directory
...
```
✅ **Expected**: New commands in help listing

#### Test 5.2: Echo Still Works
```bash
moonos:/> echo Hello World!
Hello World!
```
✅ **Expected**: Echo unaffected

#### Test 5.3: Clear Still Works
```bash
moonos:/> clear
(screen clears)
moonos:/>
```
✅ **Expected**: Clear unaffected

---

### Test Set 6: History & Editing

#### Test 6.1: History Works with File Commands
```bash
moonos:/> mkdir test1
[FS] Directory created: test1

moonos:/> mkdir test2
[FS] Directory created: test2

(Press UP arrow)
moonos:/> mkdir test2         (should show last command)

moonos:/> mkdir test3
[FS] Directory created: test3

(Press UP three times to cycle back)
(Should eventually show "mkdir test1")
```
✅ **Expected**: History saves file commands

#### Test 6.2: Inline Editing with File Commands
```bash
moonos:/> mkdir doccuments
(Use LEFT arrow to position cursor, delete 'c', type 'u')
moonos:/> mkdir documents
[FS] Directory created: documents
```
✅ **Expected**: Can edit file command line

---

## 📊 Test Results Checklist

### Functionality Tests

| Test | Expected | Result |
|------|----------|--------|
| touch creates file | File added to listing | ✅/❌ |
| rm deletes file | File removed from listing | ✅/❌ |
| mkdir creates dir | Directory visible with "/" | ✅/❌ |
| cd enters dir | Prompt changes to dirname | ✅/❌ |
| cd / returns to root | Prompt shows "/" | ✅/❌ |
| pwd shows correct dir | Shows current directory | ✅/❌ |
| ls lists children only | Subdirs don't show parent files | ✅/❌ |
| Mixed files/dirs | Both types appear in ls | ✅/❌ |

### Error Handling

| Test | Should Fail Gracefully | Result |
|------|------------------------|--------|
| touch duplicate | Error, file not created | ✅/❌ |
| cd nonexistent | Error, stay in current dir | ✅/❌ |
| rm nonexistent | Error message | ✅/❌ |
| rm non-empty dir | Reject, show error | ✅/❌ |

### Integration Tests

| Test | Should Work | Result |
|------|------------|--------|
| Help lists all 13 commands | Yes | ✅/❌ |
| History saves file commands | Yes | ✅/❌ |
| Edit command still works | Yes | ✅/❌ |
| Echo still works | Yes | ✅/❌ |

---

## 🐛 Known Issues to Watch For

### Issue 1: Prompt Not Updating
**Symptom**: `cd Documents` doesn't change prompt
**Expected**: `moonos:/Documents>`
**Status**: Should be fixed in this build

### Issue 2: Directory Creation Fails Silently
**Symptom**: `mkdir folder` shows no output
**Expected**: `[FS] Directory created: folder`
**Status**: Check fs.cpp for error handling

### Issue 3: ls Shows Files from All Dirs
**Symptom**: Files from parent dir appear in subdirectory ls
**Expected**: Only current dir children shown
**Status**: Check parent_inode filtering in list_files()

### Issue 4: Cat Shows Nothing
**Symptom**: `cat file.txt` no output
**Expected**: "(empty file)" message
**Status**: File content storage not implemented (Phase 4)

---

## 🔍 Debugging Steps

### If Commands Don't Work

1. **Check compilation:**
   ```bash
   make clean
   make 2>&1 | tail -50
   ```
   Should show: `xorriso: success`

2. **Check ISO created:**
   ```bash
   ls -lh build/MoonOS.iso
   ```
   Should show ~5.0 MB file

3. **Check boot message:**
   - Look for `[FS] MoonFS Mounted.`
   - If missing, check kernel/core/kernel.cpp initialization

4. **Serial output:**
   - All FS operations print to serial COM1
   - If QEMU with `-serial stdio`, should see all messages
   - If not, file system silently failing

---

## 📝 Test Session Template

```
Session Date: ___________
Tester: ___________

Build Status: ✅/❌
Boot Status: ✅/❌

Test Set 1 (Basic Files): ___ / 3 PASS
Test Set 2 (Directories): ___ / 4 PASS
Test Set 3 (Complex): ___ / 3 PASS
Test Set 4 (Errors): ___ / 4 PASS
Test Set 5 (Integration): ___ / 3 PASS
Test Set 6 (History): ___ / 2 PASS

Total: ___ / 23 PASS

Issues Found:
- 
- 
- 

Notes:
```

---

## 🎯 Success Criteria

✅ **MINIMAL**: File/dir creation, listing, deletion, navigation work  
✅ **STANDARD**: + Error handling + Directory isolation  
✅ **COMPLETE**: + All edge cases + Help integration + History  

**Phase 3 Complete When**: All Test Sets 1-3 pass consistently

---

## 🚀 Next Phase

After Phase 3 testing passes:

**Phase 4 Tasks:**
1. Implement disk persistence (save FS to ATA disk)
2. Add file content storage system
3. Integrate editor file save to FS
4. Support absolute paths in cd command

---

## 📞 Support

If tests fail:
1. Check compiler warnings: `make 2>&1 | grep -i warning`
2. Verify FS initialization in kernel.cpp
3. Check parent_inode assignment in fs.cpp
4. Review prompt generation in shell.cpp
5. Test with `echo` command to verify shell still works

---

**Ready to Test?** 🧪

```
$ make && make run
→ Boot MoonOS
→ Type: mkdir test
→ Type: ls
→ Type: cd test
→ Type: pwd
→ 🎉 Success!
```

Let's go! 🚀
