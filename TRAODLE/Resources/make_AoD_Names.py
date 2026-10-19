# Rigenera Resources/AoD_Names.txt dai log e dagli XML dell'Export manager di WorldEdit (Release/*.log, Release/*.xml) e dal
# file dei template degli effetti (TRAOD/Reference/Tools/worldedit/Sys/effects.fsc). Uso: python make_AoD_Names.py
import re, glob, collections, os
def H(s):
    b = s.encode(); n = len(b); h = n; j = (n >> 6) + 1; i = 0
    def i32(x): x &= 0xffffffff; return x - (1 << 32) if x & 0x80000000 else x
    while n > 0:
        c = b[i] - 256 if b[i] > 127 else b[i]
        h = i32(h ^ i32((h >> 2) + i32(32 * h) + c)); i += 1; n -= j
    return h & 0xffffffff
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, 'AoD_Names.txt')

nodes = collections.Counter(); meshes = collections.Counter(); events = collections.Counter()
templates = {}; scripts = {}; bad = collections.Counter()
ok = lambda s: s and all(0x20 < ord(c) < 0x7F or c == ' ' for c in s) and s.strip() == s
RELEASE = os.path.join(HERE, '..', '..', 'Release')
LOGS = [f for f in sorted(glob.glob(os.path.join(RELEASE, '*.log'))) if not os.path.basename(f).startswith('TRAODLE')]	# Esclusi i log di TRAODLE
for xml in sorted(glob.glob(os.path.join(RELEASE, '*.xml'))):     # XML dell'Export manager: nodi (nome, hash) e script
    text = open(xml, 'rb').read().decode('latin-1')
    for name, h in re.findall(r'<Name>"(.*?)"</Name>\s*<Hash>"([0-9a-f]+)"</Hash>', text):
        if ok(name) and H(name) == int(h, 16): nodes[name] += 1
        else: bad['xml node hash'] += 1
    for block in re.findall(r'<Scripts>(.*?)</Scripts>', text, re.S):
        for name, h in re.findall(r'<S(.*?)>\s*<Hash>"([0-9a-f]+)"</Hash>', block):
            if ok(name) and H(name) == int(h, 16): nodes[name] += 1
for log in LOGS:
    level = re.search(r'([^/\\]+)\.log$', log).group(1).upper()
    text = open(log, encoding='latin-1').read().replace('\r', '')
    for name in re.findall(r'^Room - (.+)$', text, re.M):
        if ok(name): nodes[name] += 1
    for name, h in re.findall(r'^(\S.*?) \(([0-9a-f]+)\)$', text, re.M):
        if ok(name) and H(name) == int(h, 16): nodes[name] += 1
        else: bad['node hash'] += 1
    for name in re.findall(r'^Mesh : (.*?) ?(?:\[Collision\])?$', text, re.M):
        name = name.strip()
        if ok(name): meshes[name] += 1
    for name in re.findall(r', Event \[\d+ : (.*)\]$', text, re.M):
        if ok(name): events[name] += 1
    templates[level] = re.findall(r'^Exporting Object Template : (.*)$', text, re.M)
    scripts[level] = re.findall(r'^(.+?) : \d+ b total', text, re.M)

for name in ('global_audiomanager', 'global_cameramanager', 'global_effectsmanager', 'global_zonemanager'):
    nodes[name] += 1                                               # Nodi globali di WorldEdit (destinatari di messaggi, non nei log)

def unique(counter, upper):
    byhash = collections.defaultdict(collections.Counter)
    for name, n in counter.items():
        byhash[H(name.upper() if upper else name)][name] += n
    out = []
    for h, names in byhash.items():
        keys = {k.upper() if upper else k for k in names}
        if len(keys) > 1: bad['ambiguous'] += 1; continue          # nomi diversi con lo stesso hash: scartati
        out.append(names.most_common(1)[0][0])                     # grafia piu' frequente
    return sorted(out, key=str.lower)

lines = ['# Nomi originali di Tomb Raider: The Angel of Darkness ricavati dai log dell\'"Export manager" di WorldEdit (2003).',
         '# Generato da Resources\\make_AoD_Names.py a partire da Release\\*.log e *.xml. Formato (una voce per riga, campi separati da TAB):',
         '#   N <nome>                          nodo RMX o stanza: hash = GetHashValue(nome)',
         '#   M <nome>                          mesh degli oggetti della ZONE: hash = GetHashValue(nome in maiuscolo)',
         '#   E <nome>                          gruppo di eventi EVX: hash = GetHashValue(nome in maiuscolo)',
         '#   T <livello> <indice> <nome>       Object Template di indice dato (RMX_OBJECT.ObjectIndex) del livello',
         '#   S <livello> <indice> <nome>       script di indice dato (ordine dei file SCX) del livello',
         '#   F <nome> <forma> <tipo>           template degli effetti (Tools\\worldedit\\Sys\\effects.fsc): hash = GetHashValue(nome in',
         '#                                     maiuscolo) = RMX_EMITTER.EffectHash; forma ST_BOX / ST_LINE / ST_POINT, EF_CONST / EF_ONESHOT',
         '# Gli hash ambigui (nomi diversi con lo stesso hash) sono esclusi.']
lines += ['N\t' + n for n in unique(nodes, False)]
lines += ['M\t' + n for n in unique(meshes, True)]
lines += ['E\t' + n for n in unique(events, True)]
for level in sorted(templates):
    lines += ['T\t%s\t%d\t%s' % (level, i, n) for i, n in enumerate(templates[level]) if ok(n)]
for level in sorted(scripts):
    lines += ['S\t%s\t%d\t%s' % (level, i, n) for i, n in enumerate(scripts[level]) if ok(n)]
FSC = os.path.join(HERE, '..', 'TRAOD', 'Reference', 'Tools', 'worldedit', 'Sys', 'effects.fsc')
if os.path.exists(FSC):                                            # Template degli effetti di WorldEdit (emitter)
    for name, shape, render, kind in re.findall(r'^\[([^\]]+)\]\s+\[(\w+)\]\s+\[(\w+)\]\s+\[(\w+)\]', open(FSC).read(), re.M):
        lines.append('F\t%s\t%s\t%s' % (name, shape, kind))
open(OUT, 'w', newline='\r\n', encoding='ascii').write('\n'.join(lines) + '\n')
print({k: sum(1 for l in lines if l.startswith(k + '\t')) for k in 'NMETSF'}, dict(bad))
