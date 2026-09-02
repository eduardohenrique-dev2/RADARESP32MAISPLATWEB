import { cp, rm, mkdir } from "node:fs/promises";
import { resolve } from "node:path";
const source = resolve("data"); const output = resolve("dist");
await rm(output,{recursive:true,force:true}); await mkdir(output,{recursive:true}); await cp(source,output,{recursive:true});
console.log("Dashboard copiado de data/ para dist/.");
