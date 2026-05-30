#!/usr/bin/env python3
import os, sys, csv, glob
from collections import defaultdict
from datetime import datetime, timezone
from typing import Any

TRADE_ROOT = "trade_data"
DATE_STR = "2026-05-30"
DAY_START_US = 1_780_070_400_000_000
DAY_END_US = 1_780_156_800_000_000

PRICE_BOUNDS = {
    "BTCUSDT": (50_000.0, 200_000.0), "BTC_USDT": (50_000.0, 200_000.0),
    "ETHUSDT": (1_000.0, 10_000.0), "ETH_USDT": (1_000.0, 10_000.0),
    "SOLUSDT": (50.0, 500.0), "SOL_USDT": (50.0, 500.0),
}
MAX_LOCAL_TS = 300_000
EXPECTED_TRADE_COLS = 5

def fmt(us):
    return datetime.fromtimestamp(us / 1_000_000, tz=timezone.utc).strftime("%Y-%m-%d %H:%M:%S.%f")

def sym(path):
    for p in path.replace("\\", "/").split("/"):
        if any(p.startswith(x) for x in ("BTC", "ETH", "SOL")): return p
    return path.replace("\\", "/").split("/")[-2] if "/" in path else "unknown"

def validate_orderbook(filepath):
    issues, stats = [], {}
    symbol = sym(filepath)
    bounds = PRICE_BOUNDS.get(symbol, (0.0, float("inf")))
    rows = []
    with open(filepath, "r", newline="") as f:
        reader = csv.reader(f)
        header = next(reader, None)
        if not header: issues.append("EMPTY_FILE"); return issues, stats
        data_cols = len(header) - 2
        if data_cols <= 0 or data_cols % 4 != 0:
            issues.append(f"HEADER_MALFORMED: {len(header)} cols"); return issues, stats
        dl = data_cols // 4
        stats["depth_level"] = dl
        expected = 2 + dl * 4
        for i, row in enumerate(reader, start=2):
            if not any(f.strip() for f in row): continue
            if len(row) != expected:
                issues.append(f"ROW_{i}: col mismatch ({len(row)} vs {expected})"); continue
            if any(v.strip() == "" for v in row):
                issues.append(f"ROW_{i}: empty fields"); continue
            try: ex_ts, local_ts = int(row[0]), int(row[1])
            except ValueError: issues.append(f"ROW_{i}: non-int ts"); continue
            rows.append((i, ex_ts, local_ts, row))
    if not rows: issues.append("NO_DATA_ROWS"); return issues, stats
    data_count = len(rows)
    ex_ts_list = [r[1] for r in rows]
    local_ts_list = [r[2] for r in rows]
    stats["row_count"] = data_count
    stats["min_ex_ts"], stats["max_ex_ts"] = min(ex_ts_list), max(ex_ts_list)
    stats["min_dt"], stats["max_dt"] = fmt(stats["min_ex_ts"]), fmt(stats["max_ex_ts"])
    stats["dur_s"] = (stats["max_ex_ts"] - stats["min_ex_ts"]) / 1_000_000
    na = sum(1 for i in range(1, len(ex_ts_list)) if ex_ts_list[i] < ex_ts_list[i-1])
    if na: issues.append(f"TIMESTAMP_NOT_ASC: {na}")
    oor = sum(1 for t in ex_ts_list if t < DAY_START_US or t > DAY_END_US)
    if oor: issues.append(f"TIMESTAMP_OOR: {oor}")
    bl = sum(1 for lt in local_ts_list if lt < 0 or lt > MAX_LOCAL_TS)
    if bl: issues.append(f"LOCAL_TS_BAD: {bl}")
    dup = len(ex_ts_list) - len(set(ex_ts_list))
    stats["dup_ts"] = dup
    diffs = [ex_ts_list[i] - ex_ts_list[i-1] for i in range(1, len(ex_ts_list))]
    if diffs:
        stats["med_int_us"] = sorted(diffs)[len(diffs)//2]
        stats["min_int_us"], stats["max_int_us"] = min(diffs), max(diffs)
        zi = sum(1 for d in diffs if d == 0)
        hi = sum(1 for d in diffs if d > 10_000_000)
        if zi: issues.append(f"ZERO_INTERVAL: {zi}")
        if hi: issues.append(f"HUGE_INTERVAL: {hi} >10s gaps")
    ask_na, bid_nd = 0, 0
    neg_sz, neg_pr, zero_pr = 0, 0, 0
    extreme = 0
    z_by_lv = defaultdict(int)
    rows_w_zero = []
    for row_idx, ex_ts, local_ts, row in rows:
        ap_list, bp_list = [], []
        ok = True; has_z = False
        for lv in range(dl):
            base = 2 + lv * 4
            try:
                ap, a_sz, bp, b_sz = float(row[base]), float(row[base+1]), float(row[base+2]), float(row[base+3])
            except ValueError: issues.append(f"ROW_{row_idx}: non-num L{lv+1}"); ok = False; break
            ap_list.append(ap); bp_list.append(bp)
            if ap == 0.0 or bp == 0.0:
                zero_pr += 1; has_z = True
                if ap == 0.0: z_by_lv[lv+1] += 1
                if bp == 0.0: z_by_lv[lv+1] += 1
            if ap < 0 or bp < 0: neg_pr += 1
            if a_sz < 0 or b_sz < 0: neg_sz += 1
            if ap < bounds[0] or ap > bounds[1] or bp < bounds[0] or bp > bounds[1]: extreme += 1
        if has_z: rows_w_zero.append(row_idx)
        if not ok: continue
        for j in range(1, len(ap_list)):
            if ap_list[j-1] <= 0 or ap_list[j] <= 0: continue
            if ap_list[j] <= ap_list[j-1]: ask_na += 1; break
        for j in range(1, len(bp_list)):
            if bp_list[j-1] <= 0 or bp_list[j] <= 0: continue
            if bp_list[j] >= bp_list[j-1]: bid_nd += 1; break
    if ask_na: issues.append(f"ASK_NOT_ASC: {ask_na} ({ask_na/data_count*100:.2f}%)")
    if bid_nd: issues.append(f"BID_NOT_DESC: {bid_nd} ({bid_nd/data_count*100:.2f}%)")
    if neg_sz: issues.append(f"NEG_SIZE: {neg_sz}")
    if neg_pr: issues.append(f"NEG_PRICE: {neg_pr}")
    if extreme: issues.append(f"PRICE_OOB: {extreme}")
    if zero_pr:
        pct = zero_pr / (data_count * dl * 2) * 100
        lv_det = ", ".join(f"L{l}={c}" for l, c in sorted(z_by_lv.items()))
        issues.append(f"ZERO_PRICE: {zero_pr} ({pct:.2f}%), {len(rows_w_zero)} rows. By level: {lv_det}")
        stats["zero_cnt"], stats["zero_pct"] = zero_pr, round(pct, 4)
        stats["zero_rows"] = len(rows_w_zero)
    return issues, stats

def validate_trades(filepath):
    issues, stats = [], {}
    symbol = sym(filepath)
    rows = []
    with open(filepath, "r", newline="") as f:
        reader = csv.reader(f)
        header = next(reader, None)
        if not header: issues.append("EMPTY_FILE"); return issues, stats
        if len(header) != EXPECTED_TRADE_COLS: issues.append("HEADER_MALFORMED"); return issues, stats
        for i, row in enumerate(reader, start=2):
            if not any(f.strip() for f in row): continue
            if len(row) != EXPECTED_TRADE_COLS: issues.append(f"ROW_{i}: col mismatch"); continue
            if any(v.strip() == "" for v in row): issues.append(f"ROW_{i}: empty"); continue
            try:
                ex_ts, local_ts = int(row[0]), int(row[1])
                price, qty = float(row[2]), float(row[3])
                direction = int(row[4])
            except ValueError: issues.append(f"ROW_{i}: non-num"); continue
            rows.append((i, ex_ts, local_ts, price, qty, direction))
    if not rows: issues.append("NO_DATA_ROWS"); return issues, stats
    data_count = len(rows)
    ex_ts_list = [r[1] for r in rows]
    local_ts_list = [r[2] for r in rows]
    prices = [r[3] for r in rows]
    qtys = [r[4] for r in rows]
    dirs = [r[5] for r in rows]
    MAX_SANE = 9_000_000_000_000_000
    valid_idx = [i for i, t in enumerate(ex_ts_list) if 0 <= t < MAX_SANE]
    if not valid_idx: issues.append("ALL_BOGUS_TS"); return issues, stats
    bogus = data_count - len(valid_idx)
    ex_ts_list = [ex_ts_list[i] for i in valid_idx]
    local_ts_list = [local_ts_list[i] for i in valid_idx]
    prices = [prices[i] for i in valid_idx]
    qtys = [qtys[i] for i in valid_idx]
    dirs = [dirs[i] for i in valid_idx]
    stats["row_count"] = data_count
    stats["bogus_ts"] = bogus
    stats["min_ex_ts"], stats["max_ex_ts"] = min(ex_ts_list), max(ex_ts_list)
    stats["min_dt"], stats["max_dt"] = fmt(stats["min_ex_ts"]), fmt(stats["max_ex_ts"])
    stats["dur_s"] = (stats["max_ex_ts"] - stats["min_ex_ts"]) / 1_000_000
    stats["min_price"], stats["max_price"] = min(prices), max(prices)
    stats["total_vol"] = sum(qtys)
    na = sum(1 for i in range(1, len(ex_ts_list)) if ex_ts_list[i] < ex_ts_list[i-1])
    if na: issues.append(f"TS_NOT_ASC: {na}")
    oor = sum(1 for t in ex_ts_list if t < DAY_START_US or t > DAY_END_US)
    if oor: issues.append(f"TS_OOR: {oor}")
    bl = sum(1 for lt in local_ts_list if lt < 0 or lt > MAX_LOCAL_TS)
    if bl: issues.append(f"LOCAL_TS_BAD: {bl}")
    diffs = [ex_ts_list[i] - ex_ts_list[i-1] for i in range(1, len(ex_ts_list))]
    if diffs:
        stats["med_int_us"] = sorted(diffs)[len(diffs)//2]
        stats["min_int_us"], stats["max_int_us"] = min(diffs), max(diffs)
        zi = sum(1 for d in diffs if d == 0)
        hi = sum(1 for d in diffs if d > 60_000_000)
        if zi: issues.append(f"ZERO_INTERVAL: {zi}")
        if hi: issues.append(f"HUGE_INTERVAL: {hi}")
    npp = sum(1 for p in prices if p <= 0)
    npq = sum(1 for q in qtys if q <= 0)
    bd = sum(1 for d in dirs if d not in (1, -1))
    if npp: issues.append(f"NON_POS_PRICE: {npp}")
    if npq: issues.append(f"NON_POS_QTY: {npq}")
    if bd: issues.append(f"BAD_DIR: {bd}")
    bounds = PRICE_BOUNDS.get(symbol)
    if bounds:
        ex = sum(1 for p in prices if p < bounds[0] or p > bounds[1])
        if ex: issues.append(f"PRICE_OOB: {ex}")
    return issues, stats

SEVERITY = [
    "EMPTY_FILE", "HEADER_MALFORMED", "NO_DATA_ROWS", "NEG_PRICE", "NEG_SIZE",
    "NON_POS_PRICE", "NON_POS_QTY", "BAD_DIR",
    "ZERO_PRICE", "ASK_NOT_ASC", "BID_NOT_DESC", "TIMESTAMP_NOT_ASC", "TIMESTAMP_OOR",
    "LOCAL_TS_BAD", "ZERO_INTERVAL", "HUGE_INTERVAL", "PRICE_OOB",
]

def sev(issue):
    for i, p in enumerate(SEVERITY):
        if issue.startswith(p): return i
    return len(SEVERITY)

def main():
    files = sorted(glob.glob(os.path.join(TRADE_ROOT, "**", "*.csv"), recursive=True))
    if not files: print("No CSV files found"); return 1
    print("=" * 90)
    print(f"  Trade Data Validation — {DATE_STR} — {len(files)} files")
    print("=" * 90)
    all_stats, total, crit = {}, 0, 0
    for fp in files:
        is_ob = "orderbook" in fp
        label = "OB" if is_ob else "TR"
        print(f"\n{'─'*70}\n  {label}: {fp}\n{'─'*70}")
        sz = os.path.getsize(fp) / (1024*1024)
        print(f"  Size: {sz:.2f} MB")
        issues, stats = validate_orderbook(fp) if is_ob else validate_trades(fp)
        all_stats[fp] = stats
        if stats.get("row_count"):
            print(f"  Rows: {stats['row_count']:,}  |  {stats['min_dt']} → {stats['max_dt']}  |  Dur: {stats['dur_s']:.1f}s")
            if "depth_level" in stats: print(f"  Depth: {stats['depth_level']}")
            if "med_int_us" in stats:
                med = stats["med_int_us"]
                print(f"  Interval(us): min={stats['min_int_us']} med={med} max={stats['max_int_us']}")
                if med > 0: print(f"  Freq: ~{1_000_000/med:.1f} Hz")
            if "total_vol" in stats: print(f"  Vol: {stats['total_vol']:.4f}  |  Price: {stats['min_price']:.2f}→{stats['max_price']:.2f}")
            if "zero_cnt" in stats: print(f"  Zero prices: {stats['zero_cnt']} ({stats['zero_pct']:.2f}%) in {stats['zero_rows']} rows")
        if issues:
            si = sorted(issues, key=sev)
            print(f"  ISSUES ({len(issues)}):")
            for iss in si:
                tag = "CRIT" if sev(iss) <= 4 else "HIGH" if sev(iss) <= 11 else "MED" if sev(iss) <= 14 else "INFO"
                print(f"    [{tag}] {iss}")
            total += len(issues)
            crit += sum(1 for i in issues if sev(i) <= 4)
        else:
            print("  ✓ PASS")
    print(f"\n{'═'*70}\n  SUMMARY\n{'═'*70}")
    print(f"  Files: {len(files)}  |  Total rows: {sum(s.get('row_count',0) for s in all_stats.values()):,}")
    print(f"  Issues: {total}  |  Critical: {crit}")
    if crit == 0 and total == 0: print("\n  ✓ ALL PASSED"); return 0
    elif crit == 0: print(f"\n  ⚠ PASS WITH WARNINGS — {total} non-critical issue(s)"); return 0
    else: print(f"\n  ✗ FAIL — {crit} critical issue(s)"); return 1

if __name__ == "__main__":
    sys.exit(main())
