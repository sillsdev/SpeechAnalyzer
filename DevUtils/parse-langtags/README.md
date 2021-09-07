
# parse-langtags
Utility to convert [langtags.json](https://github.com/silnrsi/langtags) into a language list for Speech Analyzer.
Manually trigger the GitHub action [parse-langtags.yml](../../.github/workflows/parse-langtags.yml) to update the iso639.txt file. Changes will be commited on a new branch `chore/update-iso639` which can then be used to create a PR.

## Usage
Command-line:
```bash
node dist/index.js
    -o [output file] 
```

Typically, use `iso639.txt` for the output file name.

-----

### Get the latest langtags.json file
This will download the latest copy of langtags.json into this current folder

`npm run get`

----

### Convert langtags.json
1. Compile the project
`npm run build`
2. Convert langtags.json into an output file (e.g. iso639.txt)
`node dist/index.js -o iso639.txt`
------------------


## Developer Setup
This utility requires Git, Node.js, and TypeScript (installed locally).

### Install Git
Download and install Git

https://git-scm.com/downloads

### Install Node.js and Dependencies
Download and install the latest current version for Node.js (>=14.15.0)

https://nodejs.org/en/download/current/

After installing Node.js, reboot your PC, open Git Bash to this directory and install this project's dependencies:
```bash
npm install
```

This will install [TypeScript](https://www.typescriptlang.org/) locally and can be accessed with

```bash
npx tsc
```

### Compiling parse-langs
This compiles the TypeScript source files in `src/` into Javascript (`dist/`)

To rebuild the project
```bash
npm run build
```

You can also have TypeScript watch the project and recompile automatically
```bash
npm run watch
```

### Debugging with Visual Studio Code
Open Folder as a VS Code Project

Edit your applicable parameters in [launch.json](./.vscode/launch.json). If using Windows paths, you'll need to escape the slashes (e.g. `-p "C:\\somewhere\\to\\iso639.txt"`)

-------------

## License for parse-langtags
Copyright (c) 2021 SIL International. All rights reserved.
Licensed under the [MIT license](LICENSE).