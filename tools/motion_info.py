"""motion_info.py - lists the motions in one of the game's motion archives (.mtar): flags, frame count and the root
movement of each (distance per frame), e.g. to see how fast a walk or crawl moves Snake.

usage: python3 motion_info.py <archive.mtar> [motion numbers...]
"""
import struct,sys,math
import numpy as np
def half(u): return float(np.frombuffer(struct.pack('<H',u),dtype=np.float16)[0])
def load(path): return open(path,'rb').read()
def motions(d): return struct.unpack_from('<I',d,8)[0]
def move(d,i):
    off,size=struct.unpack_from('<QQ',d,0x200+i*16)
    flags,step,frames,doff=struct.unpack_from('<IIII',d,off+4)
    r=dict(i=i,flags=flags,step=step,frames=frames,size=size,name=struct.unpack_from('<I',d,off)[0])
    if not flags&0x20 or size==0: return r
    mo=struct.unpack_from('<i',d,off+0x38)[0]
    base=off+doff+2*mo
    w=lambda k: struct.unpack_from('<H',d,base+2*k)[0]
    p=[half(w(0)),half(w(1)),half(w(2))]
    keys=[(0,)+tuple(p)]
    bit=48; t=0
    def bits(n):
        nonlocal bit
        k=bit>>4; s=bit&15
        v=(w(k)|(w(k+1)<<16)|(w(k+2)<<32))>>s
        bit+=n
        return v&((1<<n)-1)
    while t<frames and len(keys)<400:
        du=bits(8)
        x=half(bits(16)); y=half(bits(16)); z=half(bits(16))
        if du==0: break
        t+=du; keys.append((t,x,y,z))
    r['keys']=keys
    return r
if __name__=='__main__':
    d=load(sys.argv[1])
    sel=[int(a) for a in sys.argv[2:]] or range(motions(d))
    for i in sel:
        m=move(d,i)
        k=m.get('keys')
        if not k: print(i,'fl=%x'%m['flags'],'frames',m['frames'],'no move track'); continue
        x0,y0,z0=k[0][1:]; x1,y1,z1=k[-1][1:]
        T=k[-1][0]
        ys=[q[2] for q in k]
        print(f"slot {i:3d} name={m['name']:08x} fl={m['flags']:04x} fr={m['frames']:4d} keys={len(k):3d} T={T:4d} dx={x1-x0:8.1f} dz={z1-z0:8.1f} per-frame={math.hypot(x1-x0,z1-z0)/max(1,T):6.2f} y={min(ys):.0f}..{max(ys):.0f} first={k[0][1]:.0f},{k[0][3]:.0f}")
