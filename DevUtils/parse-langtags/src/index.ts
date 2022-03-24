#!/usr/bin/env node
// Copyright 2021-2022 SIL International
import {Command } from 'commander';
import * as fs from 'fs';
import * as path from 'path';
const {version} = require('../package.json');
const program = new Command();

////////////////////////////////////////////////////////////////////
// Get parameters
////////////////////////////////////////////////////////////////////
program
  .version(version, '-v, --version', 'output the current version')
  .description("Utilities to convert langtags.json to legacy languages list")
  .option("-o, --output <outputFile>", "output file to generate")
  .parse(process.argv);

// Debugging parameters
const options = program.opts();
const debugParameters = true;
if (debugParameters) {
  console.log('Parameters:');
  if (options.output) {
    console.log(`Output file: "${options.output}"`);
  }
  console.log('\n');
}

export class iso639Type {
  abbreviation: string; // 3 letter
  iso639_3: string;
  tag: string; // Previously country code

  name: string; // name + script + regionname

  // Replace French name with localname
  localname: string

  constructor(l: any) {
    this.abbreviation = (l.iso639_3) ? l.iso639_3 : '';
    this.iso639_3 = (l.iso639_3) ? l.iso639_3 : '';
    this.tag =  (l.tag) ? l.tag : '';
    this.name = (l.name) ? l.name : '';

    // Special code to include script, region name, and variant in the "name"
    let subtags = this.tag.split('-');
    if (subtags.length > 1) {
      this.name += ' (';
      for (let i=1; i<= subtags.length; i++) {
        if (subtags[i] == l.script) {
          this.name += l.script + ' script';
          if (subtags.length > 2) {
            this.name += ', ';
          }
        } else if (subtags[i] == l.region) {
          if (l.regionname) {
            this.name += l.regionname;
          } else if (l.region == 'XK') {
            // Industry practice of XK for Kosovo
            // Reference: https://unicode.org/reports/tr35/#unicode_region_subtag_validity
            this.name += 'Kosovo';
          }
        } else if (subtags[i] == 'x') {
          // private-use variant
          this.name += 'x-' + subtags[i+1];
          break;
        } else if (subtags[i] == 'valencia') {
          // Valencian dialect of Catalan is registered in the Language Subtag Registry
          this.name += 'Valencia';
        }
      }
      this.name += ')';
    }

    // Since the output file is separated by '|' U+007C (VERTICAL LINE), we have to remove that
    // character from any language names. Currently only affects "Hai|ǁom", which has iana name "Haiǁom".
    // Note, some language names use 'ǀ' U+01C0 LATIN LETTER DENTAL CLICK which is OK
    this.name = this.name.replace('|', '');

    this.localname = (l.localname) ? l.localname : '';
  }
}

////////////////////////////////////////////////////////////////////
// End of parameters
////////////////////////////////////////////////////////////////////

function loadLangtagsJson() : any {
  const langTagsFile = "langtags.json";

  // Check if langtags.json file exists
  if (!fs.existsSync(langTagsFile)) {
    console.error("Can't open langtags.json file. Run 'npm run get' first.");
    process.exit(1);
  }

  const rawdata = fs.readFileSync(langTagsFile).toString();
  const langtagsJson = JSON.parse(rawdata);
  return langtagsJson;
}

let langtagsJson = loadLangtagsJson();

let langtags : iso639Type[] = [];
langtagsJson.forEach(l => {
  if (l.full && l.full != 'x-bad-mru-Cyrl-RU') {
    let langtag: iso639Type = new iso639Type(l);
    langtags.push(langtag);
  }
});

// Write langtags to file
if (langtags.length == 0) {
  console.error("Generates langtags is empty.");
  process.exit(1);
}

let logger = fs.createWriteStream(path.join(process.cwd(), options.output), 'utf8');
langtags.forEach(l => {
  logger.write(l.abbreviation + '|' + l.iso639_3 + '|' + l.tag + '|' +
    l.name + '|' + l.localname + '\r\n');

  // TODO: SA only uses these two fields. Could update SA_View.cpp to use this...
  // logger.write(l.tag + '|' + l.name + '\r\n');
});
logger.end();

console.log('done');
