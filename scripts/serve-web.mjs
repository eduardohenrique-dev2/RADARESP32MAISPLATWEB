import http from 'node:http';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'../dist');
const port=Number(process.env.PORT||4173);
const mime={'.html':'text/html; charset=utf-8','.css':'text/css; charset=utf-8','.js':'text/javascript; charset=utf-8','.json':'application/json; charset=utf-8','.svg':'image/svg+xml','.png':'image/png','.ico':'image/x-icon'};

const server=http.createServer((req,res)=>{
  const url=new URL(req.url||'/',`http://${req.headers.host||'localhost'}`);
  let pathname=decodeURIComponent(url.pathname);
  if(pathname==='/'||pathname==='')pathname='/index.html';
  const filePath=path.resolve(root,'.'+pathname);
  if(!filePath.startsWith(root)){
    res.writeHead(403);res.end('Forbidden');return;
  }
  fs.stat(filePath,(err,stat)=>{
    if(err||!stat.isFile()){
      res.writeHead(404,{'Content-Type':'text/plain; charset=utf-8'});res.end('Not found');return;
    }
    res.writeHead(200,{'Content-Type':mime[path.extname(filePath).toLowerCase()]||'application/octet-stream','Cache-Control':'no-store'});
    fs.createReadStream(filePath).pipe(res);
  });
});

server.listen(port,'0.0.0.0',()=>{
  console.log(`\nSmart Radar DEMO: http://localhost:${port}/?demo=1`);
  console.log(`Celular na mesma rede: http://IP-DO-PC:${port}/?demo=1\n`);
});
