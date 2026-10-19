# Generates tests/modes/*.txt: from the title screen into each mode, then a fight with
# both players walking, jumping, attacking and doing special moves. Run:
#   python3 tools/mode_scripts.py tests/modes
import sys, os
MODES = ['team_play', 'team_vs', 'single_play', 'single_vs', 'survivor', 'practice']
def moves(p, fwd, back):
    pre = '' if p == 1 else 'p2'
    d = lambda b: pre + b
    F, B, U, D = d(fwd), d(back), d('up'), d('down')
    A, Bt, C, Dk = d('a'), d('b'), d('c'), d('d')
    # (duration, [(offset_start, offset_end, buttons)])
    return [
        (40, [(0, 30, F)]),                                        # walk in
        (30, [(0, 4, U + '+' + F)]),                               # jump in
        (20, [(0, 6, A)]), (20, [(0, 6, Bt)]), (20, [(0, 6, C)]), (20, [(0, 6, Dk)]),
        (20, [(0, 6, D + '+' + Bt)]),                              # crouching kick
        (30, [(0, 3, D), (4, 7, D + '+' + F), (8, 13, F + '+' + A)]),            # quarter circle forward + A
        (30, [(0, 3, D), (4, 7, D + '+' + B), (8, 13, B + '+' + C)]),            # quarter circle back + C
        (30, [(0, 3, F), (4, 7, D), (8, 13, D + '+' + F + '+' + A)]),            # dragon punch
        (45, [(0, 3, D), (4, 7, D + '+' + F), (8, 11, F), (12, 15, D), (16, 19, D + '+' + F), (20, 25, F + '+' + A)]),  # super
        (25, [(0, 6, A + '+' + Bt)]),                              # roll
        (25, [(0, 6, C + '+' + Dk)]),                              # blowback
        (30, [(0, 6, A + '+' + Bt + '+' + C)]),                    # MAX mode
        (40, [(0, 30, B)]),                                        # walk back
    ]
def script(mode):
    idx = MODES.index(mode)
    L = [f'# {mode}: reach the mode from the title screen, then fight with both players.',
         '# Generated; frame numbers are 59.19 Hz video frames from power-on.', '2300 start']
    for i in range(idx): L.append(f'{2400 + 12 * i} down')
    L.append('2500 a')
    for f in range(2590, 3900, 90): L += [f'{f} a', f'{f + 45} p2a']
    t, end = 3900, 14000
    p1, p2 = moves(1, 'right', 'left'), moves(2, 'left', 'right')
    i = 0
    while t < end:
        for pat in (p1[i % len(p1)], p2[(i + 5) % len(p2)]):
            for s, e, b in pat[1]: L.append(f'{t + s}-{t + e} {b}')
        t += max(p1[i % len(p1)][0], p2[(i + 5) % len(p2)][0])
        i += 1
        if i % len(p1) == 0:      # menus between rounds and matches: confirm, continue
            L += [f'{t} a', f'{t + 45} p2a', f'{t + 90} start', f'{t + 135} start2']
            t += 180
    return '\n'.join(L) + '\n'
out = sys.argv[1]
os.makedirs(out, exist_ok=True)
for m in MODES:
    open(os.path.join(out, m + '.txt'), 'w').write(script(m))
