/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         web.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Módulo de servidor HTTP y panel de administración web. Gestiona
 *                el ciclo de vida del servidor, endpoints REST para SQLite,
 *                importación/exportación CSV, autenticación Basic + throttling,
 *                y panel SPA embebido con búsqueda, paginación, ordenamiento
 *                y DETECCIÓN AUTOMÁTICA DE RELACIONES (FOREIGN KEYS).
 */
#include "evaluator.h"
#include "lexer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <sqlite3.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <io.h>
    #define close(fd) closesocket(fd)
#else
    #include <sys/select.h>
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <errno.h>
#endif

#ifdef _WIN32
    #define SHUT_RDWR 2
#endif

// Puntero global al contexto (se inicializa en INICIARSERVER)
static Contexto *ctx_global = NULL;

// Función para establecer el contexto global
void web_set_contexto(Contexto *ctx)
{
    ctx_global = ctx;
}

// Estado del servidor web
volatile int server_running = 0;
static int listen_fd_global = -1;

// AUTENTICACIÓN BÁSICA
#define AUTH_USER "admin"
#define AUTH_PASS "nico2026"

// Decodificador Base64 mínimo
static int b64decode(const char *in, char *out, int max) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i=0, k=0, len=0;
    unsigned char buf[4], tmp[3];
    while(in[i] && in[i]!='\r' && in[i]!='\n' && len<max-1) {
        if(in[i]=='=') break;
        int p=0; while(table[p] && table[p]!=in[i]) p++;
        if(!table[p]) { i++; continue; }
        buf[k++] = p;
        if(k==4) {
            tmp[0]=(buf[0]<<2)|(buf[1]>>4);
            tmp[1]=((buf[1]&0x0F)<<4)|(buf[2]>>2);
            tmp[2]=((buf[2]&0x03)<<6)|buf[3];
            for(int t=0;t<3;t++) if(len<max-1) out[len++]=tmp[t];
            k=0;
        }
        i++;
    }
    if(k==3) {
        out[len++]=(buf[0]<<2)|(buf[1]>>4);
        if(len<max-1) out[len++]=((buf[1]&0x0F)<<4)|(buf[2]>>2);
    } else if(k==2) {
        out[len++]=(buf[0]<<2)|(buf[1]>>4);
    }
    out[len]='\0'; return len;
}

// HTML/JS FRONTEND EMBEBIDO
const char* ADMIN_HTML = 
"<!DOCTYPE html><html><head><meta charset='utf-8'><title>Nico DB Admin</title>"
"<style>body{font-family:system-ui;margin:0;background:#f0f2f5}.nav{display:flex;align-items:center;background:#2d3748;padding:10px 20px;color:#fff;gap:15px}.nav a,.nav button{color:#fff;text-decoration:none;padding:6px 12px;border-radius:4px;cursor:pointer;border:none;font-size:14px;background:transparent}.nav a:hover,.nav button:hover{background:#4a5568}.nav .title{font-weight:bold;margin-right:auto}.sb{float:left;width:200px;height:calc(100vh - 50px);background:#fff;padding:10px;border-right:1px solid #ddd;overflow-y:auto}.mn{margin-left:210px;padding:20px;min-height:calc(100vh - 50px)}table{width:100%;border-collapse:collapse;margin-top:15px;background:#fff}th,td{border:1px solid #e2e8f0;padding:8px;text-align:left}th{background:#edf2f7;cursor:pointer;user-select:none}th:hover{background:#e2e8f0}#metrics{display:flex;gap:15px;margin:10px 0;flex-wrap:wrap}.card{background:#fff;padding:10px 15px;border-radius:6px;box-shadow:0 1px 3px rgba(0,0,0,0.1);font-size:14px}.card b{color:#2d3748}.card span{color:#718096}input{padding:6px;margin:4px;width:100%;box-sizing:border-box}button{padding:6px 10px;cursor:pointer;margin:2px;border:none;border-radius:3px}button:disabled{opacity:0.4;cursor:not-allowed}.ba{background:#38a169;color:#fff}.be{background:#3182ce;color:#fff}.bd{background:#e53e3e;color:#fff}form{background:#f7fafc;padding:15px;border:1px solid #e2e8f0;margin:15px 0;display:none}.actions{margin:10px 0;display:flex;gap:8px;flex-wrap:wrap}.search-box{display:flex;gap:8px;max-width:300px;flex:1}.search-box input{flex:1;padding:6px 10px;border:1px solid #cbd5e0;border-radius:4px}#pagination{margin-top:15px;text-align:center;display:none}#pagination button{background:#718096;color:#fff;padding:6px 12px;margin:0 5px}#pagination span{margin:0 10px;font-weight:bold}input[type=file]{display:none}.file-label{background:#38a169;color:#fff;padding:6px 12px;border-radius:4px;cursor:pointer;display:inline-block}.file-label:hover{background:#2f855a}#err{background:#fed7d7;color:#c53030;padding:10px;margin:10px;border-radius:4px;display:none}.fk-badge{background:#edf2f7;padding:2px 6px;border-radius:3px;font-size:11px;color:#4a5568}.related-col{background:#fffaf0}</style></head><body>"
"<div class='nav'><span class='title'>🗄️ Nico DB Admin</span><a href='/admin'>🏠 Inicio</a><button onclick='location.reload()'>🔄 Recargar</button><button id='csvBtn' onclick='expCSV()' style='display:none'>📥 Exportar CSV</button><label class='file-label' style='background:#718096;margin-left:5px'>📤 Importar CSV<input type='file' id='csvFileNav' accept='.csv'></label><button id='relatedBtn' onclick='showRelatedView()' style='display:none'>🔗 Ver Relacionado</button></div>"
"<div class='sb'><h3 style='margin:10px 0 5px'>📁 Tablas</h3><ul id='tbl' style='list-style:none;padding:0;margin:0'></ul></div>"
"<div class='mn'><h2 id='tit'>Seleccioná una tabla</h2>"
"<div id='err'></div><div id='metrics'></div>"
"<div class='search-box' id='searchBox' style='display:none'><input type='text' id='searchInput' placeholder='🔍 Buscar...'><button onclick='clearSearch()'>✖</button></div>"
"<div id='act' class='actions' style='display:none'><button class='ba' onclick='showForm()'>➕ Nuevo Registro</button></div>"
"<form id='frm'><div id='fld'></div><br><button type='submit' class='ba'>Guardar</button> <button type='button' onclick='frm.style.display=\"none\"' style='background:#cbd5e0'>Cancelar</button></form>"
"<div id='dat'></div>"
"<div id='pagination'><button id='prevBtn' onclick='goPage(currentPage-1)'>⬅ Anterior</button><span id='pageInfo'>Página 1</span><button id='nextBtn' onclick='goPage(currentPage+1)'>Siguiente ➡</button></div>"
"</div>"
"<script>"
"var tbl='',eid=null,searchTimer=null,currentPage=1,pageSize=20,currentTerm='',sortCol='',sortDir='ASC';"
"function logErr(m){console.error(m);document.getElementById('err').style.display='block';document.getElementById('err').textContent='⚠️ '+m;}"
"function init(){fetch('/api/tables').then(r=>{if(!r.ok)throw new Error('HTTP '+r.status);return r.json();}).then(t=>{document.getElementById('tbl').innerHTML=t.length?t.map(n=>'<li style=\"cursor:pointer;padding:8px;border-bottom:1px solid #eee\" onclick=\"load(\\''+n+'\\')\">'+n+'</li>').join(''):'<li style=\"color:#999\">Sin tablas</li>';}).catch(e=>logErr('Error cargando tablas: '+e.message));}"
"function load(n,term,page,sc,sd){if(!page)page=1;if(term===undefined)term=currentTerm;else currentTerm=term;if(sc!==undefined){sortCol=sc;sortDir=sd||'ASC';}tbl=n;currentPage=page;document.getElementById('tit').textContent='📊 '+n;document.getElementById('act').style.display='flex';document.getElementById('csvBtn').style.display='inline-block';document.getElementById('searchBox').style.display='flex';document.getElementById('searchInput').value=term;document.getElementById('dat').innerHTML='<p>Cargando...</p>';document.getElementById('pagination').style.display='none';document.getElementById('metrics').innerHTML='';fetch('/api/schema/'+n).then(r=>{if(!r.ok)throw new Error('Schema err');return r.json();}).then(s=>{window.schema=s;let sql='SELECT * FROM '+n;if(term){let safe=term.replace(/'/g,\"''\");let tc=s.filter(c=>/TEXT|CHAR|VARCHAR/i.test(c.type)).map(c=>c.name+\" LIKE '%\"+safe+\"%'\");if(tc.length)sql+=' WHERE '+tc.join(' OR ');}if(sortCol)sql+=' ORDER BY '+sortCol+' '+sortDir;sql+=' LIMIT '+(pageSize+1)+' OFFSET '+((page-1)*pageSize)+';';return fetch('/api/query',{method:'POST',body:sql}).then(r=>{if(!r.ok)throw new Error('Query err');return r.json();});}).then(rows=>{let hasNext=rows.length>pageSize;if(hasNext)rows.pop();let h='<table><tr>';window.schema.forEach(c=>{let a=(sortCol===c.name)?(sortDir==='ASC'?'▲':'▼'):'';h+='<th onclick=\"toggleSort(\\''+c.name+'\\')\">'+c.name+' <small>('+c.type+')</small> '+a+'</th>';});h+='<th>⚙️</th></tr>';rows.forEach(row=>{h+='<tr>';window.schema.forEach(c=>{let v=row[c.name];h+='<td>'+(v!==undefined?v:'')+'</td>';});let pk=window.schema.find(x=>x.pk==1)?.name||'id';h+='<td><button class=\"be\" onclick=\"edit('+row[pk]+')\">✏️</button> <button class=\"bd\" onclick=\"del('+row[pk]+')\">🗑️</button></td></tr>';});document.getElementById('dat').innerHTML=h+'</table>';if(term)document.getElementById('dat').innerHTML+='<p style=\"color:#718096;margin-top:10px\">🔍 '+rows.length+' resultado(s)</p>';document.getElementById('prevBtn').disabled=(page<=1);document.getElementById('nextBtn').disabled=!hasNext;document.getElementById('pageInfo').textContent='Página '+page;document.getElementById('pagination').style.display='block';fetch('/api/query',{method:'POST',body:'SELECT COUNT(*) as total FROM '+n}).then(r=>r.json()).then(m=>{document.getElementById('metrics').innerHTML='<div class=\"card\"><b>Total registros</b><br><span>'+(m[0]?.total||0)+'</span></div>';}).catch(()=>{});}).catch(e=>logErr('Error cargando datos: '+e.message));updateRelatedButton();}"
"function toggleSort(c){load(tbl,currentTerm,1,c,sortCol===c?(sortDir==='ASC'?'DESC':'ASC'):'ASC');}"
"document.getElementById('searchInput').addEventListener('input',function(e){clearTimeout(searchTimer);searchTimer=setTimeout(()=>load(tbl,e.target.value,1),300);});"
"function clearSearch(){document.getElementById('searchInput').value='';load(tbl,'',1);}"
"function goPage(p){load(tbl,currentTerm,p);}"
"document.getElementById('csvFileNav').addEventListener('change',function(e){if(!e.target.files[0]||!tbl){alert('⚠️ Primero seleccioná una tabla en el sidebar.');e.target.value='';return;}let r=new FileReader();r.onload=function(ev){fetch('/api/import/csv?tabla='+encodeURIComponent(tbl),{method:'POST',body:ev.target.result,headers:{'Content-Type':'text/csv'}}).then(r=>r.json()).then(d=>{alert(d.status==='ok'?'✅ Importados: '+d.imported:'❌ '+d.msg);load(tbl,currentTerm,1);}).catch(e=>logErr('Error importando: '+e.message));};r.readAsText(e.target.files[0],'UTF-8');e.target.value='';});"
"function showForm(){let f=document.getElementById('fld');f.innerHTML='';eid=null;document.querySelector('#frm button[type=\"submit\"]').textContent='Guardar';window.schema.forEach(c=>{if(c.pk==1)return;let isNum=/INT|REAL|NUM|FLOAT|DEC/i.test(c.type);let isDate=/FECHA|DATE|TIME|CREADO|ACTUALIZADO|TIMESTAMP/i.test(c.name);let tipo=isNum?'number':(isDate?'datetime-local':'text');let extra=isDate?' step=\"1\"':'';f.innerHTML+='<label style=\"font-weight:bold\">'+c.name+':</label><input type=\"'+tipo+'\" name=\"'+c.name+'\"'+extra+'>';});document.getElementById('frm').style.display='block';}"
"document.getElementById('frm').onsubmit=e=>{e.preventDefault();let d=new FormData(e.target),c=[],v=[];window.schema.forEach(s=>{if(s.pk==1)return;let x=d.get(s.name);if(!x||x.trim()==='')return;c.push(s.name);let isDate=/FECHA|DATE|TIME|CREADO|ACTUALIZADO|TIMESTAMP/i.test(s.name);let val=(isDate&&x.includes('T'))?x.replace('T',' '):x;v.push(isNaN(val)?\"'\"+val.replace(/'/g,\"''\")+\"'\":val);});if(!c.length){alert('Completá al menos un campo');return;}let pk=window.schema.find(x=>x.pk==1).name;let q=eid?\"UPDATE \"+tbl+\" SET \"+c.map((k,i)=>k+'='+v[i]).join(', ')+\" WHERE \"+pk+'='+eid+\";\":\"INSERT INTO \"+tbl+\" (\"+c.join(', ')+\") VALUES (\"+v.join(', ')+\");\";fetch('/api/exec',{method:'POST',body:q}).then(()=>{frm.style.display='none';load(tbl,currentTerm,currentPage);}).catch(e=>logErr(e.message));};"
"function edit(id){showForm();eid=id;document.querySelector('#frm button[type=\"submit\"]').textContent='Actualizar';let pk=window.schema.find(x=>x.pk==1).name;fetch('/api/query',{method:'POST',body:'SELECT * FROM '+tbl+' WHERE '+pk+'='+id+';'}).then(r=>r.json()).then(rows=>{if(rows[0])window.schema.forEach(s=>{if(s.pk==1)return;let i=document.querySelector('input[name=\"'+s.name+'\"]');if(i){let v=rows[0][s.name]||'';let isDate=/FECHA|DATE|TIME|CREADO|ACTUALIZADO|TIMESTAMP/i.test(s.name);i.value=(isDate&&v.includes(' '))?v.replace(' ','T'):v;}});}).catch(e=>logErr(e.message));}"
"function del(id){if(confirm('¿Eliminar registro?')){fetch('/api/exec',{method:'POST',body:'DELETE FROM '+tbl+' WHERE '+window.schema.find(x=>x.pk==1).name+'='+id+';'}).then(()=>load(tbl,currentTerm,currentPage)).catch(e=>logErr(e.message));}}"
"function expCSV(){if(!tbl)return;window.location.href='/api/export/csv?tabla='+encodeURIComponent(tbl);}"
"async function showRelatedView(){if(!tbl)return;document.getElementById('tit').textContent='🔗 '+tbl+' (Vista Relacionada)';document.getElementById('dat').innerHTML='<p>🔍 Detectando relaciones...</p>';document.getElementById('pagination').style.display='none';document.getElementById('act').style.display='none';try{const [schemaRes, fkRes] = await Promise.all([fetch('/api/schema/'+tbl), fetch('/api/foreign-keys/'+tbl)]);const schema = await schemaRes.json();const foreignKeys = await fkRes.json();if(!foreignKeys.length){document.getElementById('dat').innerHTML='<p style=\"color:#718096\">ℹ️ La tabla \"'+tbl+'\" no tiene relaciones definidas.</p>';return;}let selectCols = schema.map(c => tbl+'.'+c.name).join(', ');let joinClauses = [];let extraCols = [];const refTables = [...new Set(foreignKeys.map(fk => fk.table))];const schemaPromises = refTables.map(t => fetch('/api/schema/'+t).then(r=>r.json()));const refSchemaResults = await Promise.all(schemaPromises);let refSchemas = {};refTables.forEach((t,i)=>refSchemas[t]=refSchemaResults[i]);const descCols = ['nombre','descripcion','titulo','label','name','description','apellido','email'];foreignKeys.forEach(fk => {joinClauses.push('LEFT JOIN '+fk.table+' ON '+tbl+'.'+fk.column+' = '+fk.table+'.'+fk.to_column);const refSchema = refSchemas[fk.table] || [];let foundCol = null;for(let dc of descCols){const match = refSchema.find(c => c.name.toLowerCase().includes(dc));if(match){foundCol=match.name;break;}}if(foundCol) extraCols.push(fk.table+'.'+foundCol+' AS '+fk.table+'_'+foundCol);});const sql = 'SELECT '+selectCols+(extraCols.length?', '+extraCols.join(', '):'')+' FROM '+tbl+' '+joinClauses.join(' ');const res = await fetch('/api/query',{method:'POST',body:sql});if(!res.ok) throw new Error('HTTP '+res.status);const rows = await res.json();if(!rows.length){document.getElementById('dat').innerHTML='<p style=\"color:#718096\">Sin resultados</p>';return;}let html = '<table><thead><tr>';schema.forEach(c => {const fk = foreignKeys.find(f => f.column === c.name);const headerText = fk ? c.name+' → '+fk.table : c.name;html += '<th>'+headerText+' <small>('+c.type+')</small>'+(fk?' <span class=\"fk-badge\">🔗</span>':'')+'</th>';});extraCols.forEach(col => {const parts = col.split(' AS ');const colName = parts[1] || parts[0].split('.').pop();html += '<th class=\"related-col\">🔗 '+colName+'</th>';});html += '</tr></thead><tbody>';rows.forEach(row => {html += '<tr>';schema.forEach(c => {let val = row[c.name] !== undefined ? row[c.name] : '-';const fk = foreignKeys.find(f => f.column === c.name);html += '<td'+(fk?' class=\"related-col\"':'')+'>'+val+'</td>';});extraCols.forEach(col => {const alias = col.split(' AS ')[1] || col.split('.').pop();let val = row[alias] !== undefined ? row[alias] : '-';html += '<td>'+val+'</td>';});html += '</tr>';});html += '</tbody></table>';html += '<div style=\"margin-top:15px;padding:10px;background:#edf2f7;border-radius:6px\"><strong>📊 Resumen:</strong><br>• Tabla: '+tbl+'<br>• Relaciones detectadas: '+foreignKeys.length+'<br>• '+foreignKeys.map(fk => fk.column+' → '+fk.table+'.'+fk.to_column).join('<br>• ')+'<br>• Total registros: '+rows.length+'</div>';document.getElementById('dat').innerHTML = html;} catch(e) { logErr('Error en vista relacionada: '+e.message); }}"
"async function updateRelatedButton(){const btn = document.getElementById('relatedBtn');if(!btn || !tbl) return;try{const res = await fetch('/api/foreign-keys/'+tbl);const fks = await res.json();btn.style.display = fks.length ? 'inline-block' : 'none';} catch(e) { btn.style.display = 'none'; }}"
"window.addEventListener('load', function(){ init(); });"
"</script></body></html>";

// Escapa valores para CSV según RFC 4180
static void csv_escape(char *dst, const char *src, size_t max) {
    if (!src) src = "";
    size_t i = 0, j = 0, quote = 0;
    while (src[i]) { if (src[i]==',' || src[i]=='"' || src[i]=='\n' || src[i]=='\r') { quote=1; break; } i++; }
    if (!quote) { strncpy(dst, src, max-1); dst[max-1]='\0'; return; }
    dst[j++] = '"'; i = 0;
    while (src[i] && j < max-2) {
        if (src[i] == '"') { if (j < max-3) { dst[j++]='"'; dst[j++]='"'; } }
        else dst[j++] = src[i];
        i++;
    }
    if (j < max-1) dst[j++] = '"';
    dst[j] = '\0';
}

// Ruta: /api/export/csv?tabla=nombre
static void handle_csv_export(int fd, const char *path) {
    char tbl[128] = {0};
    const char *q = strchr(path, '?');
    if (q) {
        const char *v = strstr(q, "tabla=");
        if (v) {
            v += 6;
            int i = 0;
            while (v[i] && v[i]!='&' && v[i]!=' ' && i<127) { tbl[i] = v[i]; i++; }
            tbl[i] = '\0';
            for (i = 0; tbl[i]; i++) {
                if (tbl[i]=='%' && tbl[i+1] && tbl[i+2]) {
                    int hi = (tbl[i+1] >= 'a') ? tbl[i+1]-'a'+10 : tbl[i+1]-'0';
                    int lo = (tbl[i+2] >= 'a') ? tbl[i+2]-'a'+10 : tbl[i+2]-'0';
                    tbl[i] = (char)((hi << 4) | lo);
                    memmove(&tbl[i+1], &tbl[i+3], strlen(&tbl[i+3])+1);
                }
            }
        }
    }
    if (!tbl[0] || !ctx_global->sqlite_db)
    {
        const char *err = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 24\r\nConnection: close\r\n\r\nFalta parametro tabla";
        send(fd, err, strlen(err), 0); close(fd); return;
    }
    sqlite3_stmt *stmt;
    char sql[256]; snprintf(sql, sizeof(sql), "SELECT * FROM %s", tbl);
    if (sqlite3_prepare_v2((sqlite3 *)ctx_global->sqlite_db, sql, -1, &stmt, 0) != SQLITE_OK)
    {
        const char *err = "HTTP/1.1 500 Error\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nError SQL";
        send(fd, err, strlen(err), 0); close(fd); return;
    }
    static char buf[131072]; size_t len = 0;
    int cols = sqlite3_column_count(stmt);
    for (int c = 0; c < cols; c++) {
        char e[256]; csv_escape(e, sqlite3_column_name(stmt, c) ? sqlite3_column_name(stmt, c) : "?", sizeof(e));
        len += snprintf(buf+len, sizeof(buf)-len, "%s%s", e, c<cols-1 ? "," : "\r\n");
    }
    while (sqlite3_step(stmt)==SQLITE_ROW && len < sizeof(buf)-2048) {
        for (int c = 0; c < cols; c++) {
            char e[512];
            const char *val = sqlite3_column_text(stmt, c) ? (const char *)sqlite3_column_text(stmt, c) : "";
            csv_escape(e, val, sizeof(e));
            len += snprintf(buf+len, sizeof(buf)-len, "%s%s", e, c<cols-1 ? "," : "\r\n");
        }
    }
    buf[len] = '\0'; sqlite3_finalize(stmt);
    char hdr[512];
    snprintf(hdr, sizeof(hdr), 
        "HTTP/1.1 200 OK\r\nContent-Type: text/csv; charset=utf-8\r\n"
        "Content-Disposition: attachment; filename=\"%s.csv\"\r\n"
        "Content-Length: %zu\r\nConnection: close\r\n\r\n", tbl, len);
    send(fd, hdr, strlen(hdr), 0);
    send(fd, buf, len, 0);
    close(fd);
}

// IMPORTACIÓN CSV
static void handle_csv_import(int fd, const char *path, const char *body, size_t bsize) {
    (void)bsize;
    char tbl[128] = {0};
    const char *q = strchr(path, '?');
    if (q) {
        const char *v = strstr(q, "tabla=");
        if (v) {
            v += 6;
            int i = 0; while (v[i] && v[i] != '&' && v[i] != ' ' && i < 127) { tbl[i] = v[i]; i++; }
            tbl[i] = '\0';
        }
    }
    if (!tbl[0] || !ctx_global->sqlite_db || !body)
    {
        const char *json = "{\"status\":\"error\",\"msg\":\"Falta tabla o datos\"}";
        int blen = strlen(json);
        char hdr[256]; int hlen = snprintf(hdr, sizeof(hdr),
            "HTTP/1.1 400\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", blen);
        send(fd, hdr, hlen, 0); send(fd, json, blen, 0); close(fd); return;
    }
    size_t len = strlen(body);
    char *data = malloc(len + 1);
    if (!data) { close(fd); return; }
    memcpy(data, body, len + 1);
    sqlite3_exec((sqlite3 *)ctx_global->sqlite_db, "BEGIN", 0, 0, 0);
    char cols_list[512] = "";
    char placeholders[256] = "";
    int col_count = 0;
    sqlite3_stmt *info;
    char qinfo[256]; snprintf(qinfo, sizeof(qinfo), "PRAGMA table_info(%s)", tbl);
    if (sqlite3_prepare_v2((sqlite3 *)ctx_global->sqlite_db, qinfo, -1, &info, 0) == SQLITE_OK)
    {
        while (sqlite3_step(info) == SQLITE_ROW && col_count < 32) {
            int pk = sqlite3_column_int(info, 5);
            const char *n = (const char *)sqlite3_column_text(info, 1);
            if (n && pk == 0) {
                strcat(cols_list, col_count > 0 ? "," : "");
                strcat(cols_list, n);
                strcat(placeholders, col_count > 0 ? ",?" : "?");
                col_count++;
            }
        }
        sqlite3_finalize(info);
    }
    if (col_count == 0) {
        free(data);
        sqlite3_exec((sqlite3 *)ctx_global->sqlite_db, "ROLLBACK", 0, 0, 0);
        const char *json = "{\"status\":\"error\",\"msg\":\"No hay columnas importables\"}";
        int blen = strlen(json);
        char hdr[256]; int hlen = snprintf(hdr, sizeof(hdr),
            "HTTP/1.1 400\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", blen);
        send(fd, hdr, hlen, 0); send(fd, json, blen, 0); close(fd); return;
    }
    char insert_sql[1024];
    snprintf(insert_sql, sizeof(insert_sql), "INSERT INTO %s (%s) VALUES (%s)", tbl, cols_list, placeholders);
    int imported = 0;
    char *line = data;
    int is_header = 1;
    const char *error_msg = NULL;
    while (line && *line && !error_msg) {
        char *next = strchr(line, '\n');
        if (next) { *next = '\0'; next++; }
        size_t l = strlen(line); if (l > 0 && line[l-1] == '\r') line[l-1] = '\0';
        if (l <= 1) { line = next; continue; }
        if (is_header) { is_header = 0; line = next; continue; }
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2((sqlite3 *)ctx_global->sqlite_db, insert_sql, -1, &stmt, 0) != SQLITE_OK)
        {
            error_msg = sqlite3_errmsg((sqlite3 *)ctx_global->sqlite_db);
            break;
        }
        int pos = 1, csv_idx = 0;
        char *tok = line;
        while (tok && csv_idx < col_count) {
            char *comma = strchr(tok, ',');
            if (comma) *comma = '\0';
            sqlite3_bind_text(stmt, pos++, tok, -1, SQLITE_TRANSIENT);
            csv_idx++;
            tok = comma ? comma + 1 : NULL;
        }
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) imported++;
        else if (rc != SQLITE_ROW) {
            error_msg = sqlite3_errmsg((sqlite3 *)ctx_global->sqlite_db);
        }
        sqlite3_finalize(stmt);
        line = next;
    }
    if (error_msg || imported == 0)
        sqlite3_exec((sqlite3 *)ctx_global->sqlite_db, "ROLLBACK", 0, 0, 0);
    else
        sqlite3_exec((sqlite3 *)ctx_global->sqlite_db, "COMMIT", 0, 0, 0);
    free(data);
    char json[512];
    if (error_msg) {
        char safe[256] = {0}; int s = 0, m = 0;
        while (error_msg[m] && s < 250) {
            if (error_msg[m] == '"' || error_msg[m] == '\\') safe[s++] = '\\';
            safe[s++] = error_msg[m++];
        }
        safe[s] = '\0';
        snprintf(json, sizeof(json), "{\"status\":\"error\",\"msg\":\"%s\"}", safe);
    } else {
        snprintf(json, sizeof(json), "{\"status\":\"ok\",\"imported\":%d}", imported);
    }
    int body_len = strlen(json);
    char hdr[256];
    int hlen = snprintf(hdr, sizeof(hdr),
        "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
        "Content-Length: %d\r\nConnection: close\r\n\r\n", body_len);
    send(fd, hdr, hlen, 0);
    send(fd, json, body_len, 0);
    close(fd);
}

// THROTTLING DE LOGIN
#define MAX_ATTEMPTS 5
#define LOCK_TIME 60
static int auth_fails = 0;
static time_t auth_lock_until = 0;
static int check_auth_lock() {
    time_t now = time(NULL);
    if (now < auth_lock_until) return 1;
    if (auth_fails >= MAX_ATTEMPTS) { auth_lock_until = now + LOCK_TIME; return 1; }
    return 0;
}
static void record_auth_fail() { auth_fails++; if (auth_fails >= MAX_ATTEMPTS) auth_lock_until = time(NULL) + LOCK_TIME; }
static void record_auth_success() { auth_fails = 0; auth_lock_until = 0; }
static void json_escape(char *dst, const char *src, size_t max) {
    if (!src) src = "";
    size_t i = 0, j = 0;
    while (src[i] && j < max - 2) {
        switch (src[i]) {
            case '"':  dst[j++] = '\\'; dst[j++] = '"'; break;
            case '\\': dst[j++] = '\\'; dst[j++] = '\\'; break;
            case '\n': dst[j++] = '\\'; dst[j++] = 'n'; break;
            case '\r': dst[j++] = '\\'; dst[j++] = 'r'; break;
            case '\t': dst[j++] = '\\'; dst[j++] = 't'; break;
            default:   dst[j++] = src[i]; break;
        }
        i++;
    }
    dst[j] = '\0';
}

// /api/foreign-keys/:tabla (detección universal de FKs)
static void handle_foreign_keys(int fd, const char *path) {
    char tbl[128] = {0};
    sscanf(path + 18, "%127[^/]", tbl);

    if (!tbl[0] || !ctx_global->sqlite_db)
    {
        const char *err = "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\nContent-Length: 20\r\nConnection: close\r\n\r\n{\"error\":\"Falta tabla\"}";
        send(fd, err, strlen(err), 0); close(fd); return;
    }

    char q[256]; snprintf(q, sizeof(q), "PRAGMA foreign_key_list(%s);", tbl);
    sqlite3_stmt *s;

    if (sqlite3_prepare_v2((sqlite3 *)ctx_global->sqlite_db, q, -1, &s, 0) == SQLITE_OK)
    {
        char resp[4096] = "[";
        size_t l = 1;
        int first = 1;
        while (sqlite3_step(s) == SQLITE_ROW) {
            if (!first) { resp[l++] = ','; }
            first = 0;
            const char *from_col = (const char*)sqlite3_column_text(s, 3);
            const char *to_table = (const char*)sqlite3_column_text(s, 2);
            const char *to_col   = (const char*)sqlite3_column_text(s, 4);
            char ef[64], et[64], ec[64];
            json_escape(ef, from_col ? from_col : "", sizeof(ef));
            json_escape(et, to_table ? to_table : "", sizeof(et));
            json_escape(ec, to_col ? to_col : "", sizeof(ec));
            l += snprintf(resp + l, sizeof(resp) - l,
                "{\"column\":\"%s\",\"table\":\"%s\",\"to_column\":\"%s\"}", ef, et, ec);
        }
        resp[l++] = ']'; resp[l] = '\0';
        sqlite3_finalize(s);
        
        char hdr[256];
        int hlen = snprintf(hdr, sizeof(hdr),
            "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n", strlen(resp));
        send(fd, hdr, hlen, 0);
        send(fd, resp, strlen(resp), 0);
        close(fd);
    }
    else
    {
        const char *err = "HTTP/1.1 500 Error\r\nContent-Type: application/json\r\nContent-Length: 18\r\nConnection: close\r\n\r\n{\"error\":\"SQL error\"}";
        send(fd, err, strlen(err), 0); close(fd);
    }
}

static void* handle_client(void *arg) {
    int fd = *(int*)arg; free(arg);
    static char req[65536];
    memset(req, 0, sizeof(req));
    int recv_len = recv(fd, req, sizeof(req) - 1, 0);
    if (recv_len <= 0) { close(fd); return NULL; }
    req[recv_len] = '\0';
    char method[10] = {0}, path[256] = {0};
    sscanf(req, "%9s %255s", method, path);
    
    int content_length = 0;
    char *cl = strstr(req, "Content-Length:");
    if (cl) content_length = atoi(cl + 15);
    char *body = strstr(req, "\r\n\r\n");
    if (body) body += 4;
    else body = req + recv_len;
    size_t body_len = body ? (recv_len - (body - req)) : 0;
    int body_alloc = 0;
    if (content_length > (int)body_len && content_length < 5000000) {
        char *full = malloc(content_length + 1);
        if (full) {
            if (body_len > 0) memcpy(full, body, body_len);
            size_t left = content_length - body_len, got = body_len;
            while (got < (size_t)content_length) {
                int n = recv(fd, full + got, left, 0);
                if (n <= 0) break;
                got += n; left -= n;
            }
            full[got] = '\0';
            body = full; body_len = got; body_alloc = 1;
        }
    }
    
    int auth_ok = 0;
    if (check_auth_lock()) {
        const char *j = "{\"status\":\"error\",\"msg\":\"Bloqueado 60s\"}";
        int bl = strlen(j); char h[256]; int hl = snprintf(h,sizeof(h),"HTTP/1.1 429\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",bl);
        send(fd,h,hl,0); send(fd,j,bl,0); if(body_alloc) { free(body); } close(fd); return NULL;
    }
    char *auth = strstr(req, "Authorization: Basic ");
    if (auth) {
        char enc[256]={0}, dec[256]={0};
        char *p = auth + 21; int i=0;
        while(p[i] && p[i]!='\r' && p[i]!='\n' && i<255) { enc[i] = p[i]; i++; }
        b64decode(enc, dec, sizeof(dec));
        char expected[64]; snprintf(expected,sizeof(expected),"%s:%s",AUTH_USER,AUTH_PASS);
        if(strcmp(dec, expected)==0) auth_ok = 1;
    }
    if (!auth_ok) {
        record_auth_fail();
        const char *r = "HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Basic realm=\"Nico Admin\"\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        send(fd, r, strlen(r), 0); if(body_alloc) { free(body); } close(fd); return NULL;
    }
    record_auth_success();
    
    char resp[65536] = {0}; int status = 404; const char *ctype = "application/json";
    
    if (strcmp(path, "/admin") == 0) {
        status = 200; ctype = "text/html";
        strncpy(resp, ADMIN_HTML, sizeof(resp)-1); resp[sizeof(resp)-1]='\0';
    }
    else if (strncmp(path, "/api/export/csv", 15) == 0) {
        handle_csv_export(fd, path); if(body_alloc) { free(body); } return NULL;
    }
    else if (strncmp(path, "/api/import/csv", 15) == 0 && strcmp(method, "POST") == 0) {
        if (body && body_len > 0) handle_csv_import(fd, path, body, body_len);
        else {
            const char *j = "{\"status\":\"error\",\"msg\":\"Falta cuerpo\"}";
            int bl=strlen(j); char h[256]; int hl=snprintf(h,sizeof(h),"HTTP/1.1 400\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",bl);
            send(fd,h,hl,0); send(fd,j,bl,0);
        }
        if(body_alloc) { free(body); } close(fd); return NULL;
    }
    else if (strcmp(path, "/api/tables") == 0 && (sqlite3 *)ctx_global->sqlite_db != NULL)
    {
        sqlite3_stmt *s;
        if (sqlite3_prepare_v2((sqlite3 *)ctx_global->sqlite_db, "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name;", -1, &s, 0) == SQLITE_OK)
        {
            resp[0]='['; size_t l=1; int f=1;
            while(sqlite3_step(s)==SQLITE_ROW) {
                if(!f) { resp[l++]=','; } f=0;
                const char *n=(const char*)sqlite3_column_text(s,0);
                char en[128]; json_escape(en, n?n:"unknown", sizeof(en));
                l += snprintf(resp+l, sizeof(resp)-l, "\"%s\"", en);
            }
            resp[l++]=']'; resp[l]='\0';
            sqlite3_finalize(s); status=200;
        }
        else
        {
            strcpy(resp, "[]");
            status = 200;
        }
    }
    else if (strncmp(path, "/api/schema/", 12) == 0 && ctx_global->sqlite_db != NULL)
    {
        char t[128]={0}; sscanf(path+12,"%127[^/]",t);
        char q[256]; snprintf(q,sizeof(q),"PRAGMA table_info(%s);",t);
        sqlite3_stmt *s;
        if (sqlite3_prepare_v2((sqlite3 *)ctx_global->sqlite_db, q, -1, &s, 0) == SQLITE_OK)
        {
            resp[0]='['; size_t l=1; int f=1;
            while(sqlite3_step(s)==SQLITE_ROW) {
                if(!f) { resp[l++]=','; } f=0;
                const char *nm=(const char*)sqlite3_column_text(s,1);
                const char *tp=(const char*)sqlite3_column_text(s,2);
                int pk=sqlite3_column_int(s,5);
                char en[128], et[64];
                json_escape(en, nm?nm:"", sizeof(en));
                json_escape(et, tp?tp:"", sizeof(et));
                l += snprintf(resp+l,sizeof(resp)-l,"{\"name\":\"%s\",\"type\":\"%s\",\"pk\":%d}", en, et, pk);
            }
            resp[l++]=']'; resp[l]='\0'; sqlite3_finalize(s); status=200;
        }
        else
        {
            strcpy(resp, "[]");
            status = 200;
        }
    }
    else if (strncmp(path, "/api/foreign-keys/", 18) == 0 && ctx_global->sqlite_db != NULL)
    {
        handle_foreign_keys(fd, path);
        if(body_alloc) { free(body); }
        return NULL;
    }
    else if (strcmp(path, "/api/query") == 0 && strcmp(method, "POST") == 0 && ctx_global->sqlite_db != NULL && body)
    {
        char *sql=body; sql[strcspn(sql,"\r\n")]='\0';
        sqlite3_stmt *s;
        if (sqlite3_prepare_v2((sqlite3 *)ctx_global->sqlite_db, sql, -1, &s, 0) == SQLITE_OK)
        {
            resp[0]='['; size_t l=1; int f=1;
            while(sqlite3_step(s)==SQLITE_ROW) {
                if(!f) { resp[l++]=','; } f=0; resp[l++]='{';
                int cols=sqlite3_column_count(s);
                for(int c=0;c<cols;c++) {
                    const char *n=sqlite3_column_name(s,c);
                    const char *v=(const char*)sqlite3_column_text(s,c);
                    char en[128], ev[512];
                    json_escape(en, n?n:"", sizeof(en));
                    json_escape(ev, v?v:"", sizeof(ev));
                    l += snprintf(resp+l,sizeof(resp)-l,"\"%s\":\"%s\"", en, ev);
                    if(c<cols-1) resp[l++]=',';
                }
                resp[l++]='}';
            }
            resp[l++]=']'; resp[l]='\0'; sqlite3_finalize(s); status=200;
        }
    }
    else if (strcmp(path, "/api/exec") == 0 && strcmp(method, "POST") == 0 && ctx_global->sqlite_db != NULL && body)
    {
        char *sql=body; sql[strcspn(sql,"\r\n")]='\0';
        char *err=NULL;
        if (sqlite3_exec((sqlite3 *)ctx_global->sqlite_db, sql, NULL, NULL, &err) == SQLITE_OK)
        {
            strcpy(resp,"{\"status\":\"ok\"}"); status=200;
        }
        else
        {
            snprintf(resp,sizeof(resp),"{\"status\":\"error\",\"msg\":\"%s\"}", err?err:"unknown");
            sqlite3_free(err); status=500;
        }
    }
    else if (strncmp(path, "/render", 7) == 0) {
        const char *file_param = strstr(path, "archivo=");
        char filename[256] = {0};
        if (file_param) sscanf(file_param + 8, "%255[^& ]", filename);
        else strcpy(filename, "ejemplos/demo.nico");
        FILE *check = fopen(filename, "r");
        if (!check) {
            const char *err = "HTTP/1.1 404 Not Found\r\nContent-Length: 16\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nArchivo no existe";
            send(fd, err, strlen(err), 0); close(fd); return NULL;
        }
        fclose(check);
        char cmd[512];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "nico.exe \"%s\" 2>nul", filename);
#else
    snprintf(cmd, sizeof(cmd), "./nico \"%s\" 2>/dev/null", filename);
#endif     
        char raw[65536] = {0};
        size_t raw_len = 0;
        FILE *pipe = popen(cmd, "r");
        if (pipe) {
            raw_len = fread(raw, 1, sizeof(raw) - 1, pipe);
            pclose(pipe);
        }
        raw[raw_len] = '\0';
        char clean[65536]; size_t j = 0;
        for (size_t i = 0; i < raw_len && j < sizeof(clean)-1; i++) {
            if (raw[i] == '\x1b' && raw[i+1] == '[') {
                i += 2;
                while (raw[i] && raw[i] != 'm' && raw[i] != 'H' && raw[i] != 'J' && raw[i] != 'K' && raw[i] != 'l' && raw[i] != 'h') i++;
                continue;
            }
            clean[j++] = raw[i];
        }
        clean[j] = '\0';
        size_t clean_len = j;
        char hdr[512];
        int hlen = snprintf(hdr, sizeof(hdr),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n\r\n", clean_len);
        send(fd, hdr, hlen, 0);
        send(fd, clean, clean_len, 0);
        close(fd); return NULL;
    }
    
    int blen = strlen(resp);
    char hdr[512]; const char *stxt = (status==200)?"OK":"Error";
    int hlen = snprintf(hdr,sizeof(hdr),"HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",status,stxt,ctype,blen);
    send(fd, hdr, hlen, 0);
    send(fd, resp, blen, 0);
    if(body_alloc) { free(body); }
    close(fd);
    return NULL;
}

void* server_loop(void *arg) {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            fprintf(stderr, "[WEB] Error al inicializar Winsock.\n");
            return NULL;
        }
#endif
    int port = *(int*)arg; free(arg);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return NULL; }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
    fprintf(stderr, "[WEB] bind falló: %d\n", WSAGetLastError());
#else
    fprintf(stderr, "[WEB] bind falló: %s\n", strerror(errno));
#endif
        perror("bind"); close(fd); return NULL;
    }
    if (listen(fd, 10) < 0) {
        perror("listen"); close(fd); return NULL;
    }
    listen_fd_global = fd;
    fprintf(stderr, "🌐 Servidor web iniciado en http://localhost:%d\n", port);
    while (server_running) {
        fd_set readfds;
        struct timeval timeout;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        int activity = select(fd + 1, &readfds, NULL, NULL, &timeout);
        if (activity < 0) {
            if (!server_running) break;
            continue;
        }
        if (activity == 0) continue;
        if (FD_ISSET(fd, &readfds)) {
            struct sockaddr_in cli_addr;
            socklen_t cli_len = sizeof(cli_addr);
            int client_fd = accept(fd, (struct sockaddr*)&cli_addr, &cli_len);
            if (client_fd < 0) {
                if (!server_running) break;
                continue;
            }
            int *arg = malloc(sizeof(int)); *arg = client_fd;
            pthread_t tid;
            pthread_create(&tid, NULL, handle_client, arg);
            pthread_detach(tid);
        }
    }
    close(fd);
    listen_fd_global = -1;
    fprintf(stderr, "🛑 Servidor web detenido.\n");
    return NULL;
}

// DESPUÉS:
void cmd_iniciarserver(Contexto *ctx, int puerto)
{
    if (server_running)
    {
        printf("⚠️ El servidor ya está en ejecución.\n");
        return;
    }

    if (puerto <= 0)
        puerto = 8080;

    // Guardar contexto global para que los handlers lo usen
    web_set_contexto(ctx);

    server_running = 1;
    pthread_t tid;
    int *pp = malloc(sizeof(int));
    *pp = puerto;
    pthread_create(&tid, NULL, server_loop, pp);
    pthread_detach(tid);
}

void cmd_detenerserver(Contexto *ctx)
{
    (void)ctx;
    if (!server_running || listen_fd_global == -1)
    {
        printf("⚠️ No hay servidor web en ejecución.\n");
        return;
    }

    printf("🛑 Deteniendo servidor web...\n");
    server_running = 0;
    shutdown(listen_fd_global, SHUT_RDWR);
    close(listen_fd_global);
}
