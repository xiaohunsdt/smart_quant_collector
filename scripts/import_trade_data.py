#!/usr/bin/env python3
"""
Batch import trade_data/ CSVs into DolphinDB (TSDB + Array Vector).

NOTE: This script writes directly to DFS tables (not stream tables).
      It is designed for historical data bulk loading.
      For real-time data, the C++ MTW client writes to stream tables.

Usage:
    python import_trade_data.py --host 127.0.0.1 --port 8848

Dependencies:
    pip install dolphindb
"""
import argparse
import csv
from pathlib import Path
from typing import Iterator, Dict, List, Any

import dolphindb as ddb


ROOT = Path(__file__).parent / "trade_data"
DB_PATH = "dfs://trade_db"


def parse_path(csv_path: Path) -> dict:
    """Path format: trade_data/{exchange}/{market_type}/{symbol}/{type}_{date}.csv"""
    rel = csv_path.relative_to(ROOT)
    parts = rel.parts
    return {
        "exchange": parts[0],
        "market_type": parts[1],
        "symbol": parts[2],
        "data_type": parts[3].split("_")[0],
        "trade_date": parts[3].split("_")[1].replace(".csv", ""),
    }


def ddb_date_int(ts_us: int) -> int:
    """Microsecond timestamp -> DolphinDB DATE (days since 1970.01.01)"""
    return ts_us // 86400000000


def parse_rows(rows: List[Dict[str, str]], meta: dict) -> Dict[str, List[Any]]:
    """Convert CSV row list to columnar dict (DolphinDB Python API format)."""
    data_type = meta["data_type"]
    result: Dict[str, List[Any]] = {}

    if data_type == "trades":
        result = {
            "exchange_timestamp": [],
            "local_diff": [],
            "trade_id": [],
            "price": [],
            "quantity": [],
            "direction": [],
            "is_buyer_maker": [],
            "symbol": [],
            "exchange": [],
            "market_type": [],
            "trade_date": [],
        }
        for r in rows:
            ts = int(r["exchange_timestamp"])
            direction = int(r["direction"])
            result["exchange_timestamp"].append(ts)
            result["local_diff"].append(int(r["local_diff"]))
            result["trade_id"].append(0)
            result["price"].append(float(r["price"]))
            result["quantity"].append(float(r["quantity"]))
            result["direction"].append(direction)
            result["is_buyer_maker"].append(direction == -1)
            result["symbol"].append(meta["symbol"])
            result["exchange"].append(meta["exchange"])
            result["market_type"].append(meta["market_type"])
            result["trade_date"].append(ddb_date_int(ts))

    elif data_type == "bookticker":
        result = {
            "exchange_timestamp": [],
            "local_diff": [],
            "best_bid_price": [],
            "best_bid_qty": [],
            "best_ask_price": [],
            "best_ask_qty": [],
            "symbol": [],
            "exchange": [],
            "market_type": [],
            "trade_date": [],
        }
        for r in rows:
            ts = int(r["exchange_timestamp"])
            result["exchange_timestamp"].append(ts)
            result["local_diff"].append(int(r["local_diff"]))
            result["best_bid_price"].append(float(r["best_bid_price"]))
            result["best_bid_qty"].append(float(r["best_bid_qty"]))
            result["best_ask_price"].append(float(r["best_ask_price"]))
            result["best_ask_qty"].append(float(r["best_ask_qty"]))
            result["symbol"].append(meta["symbol"])
            result["exchange"].append(meta["exchange"])
            result["market_type"].append(meta["market_type"])
            result["trade_date"].append(ddb_date_int(ts))

    elif data_type == "orderbook":
        result = {
            "exchange_timestamp": [],
            "local_diff": [],
            "symbol": [],
            "exchange": [],
            "market_type": [],
            "trade_date": [],
            "bid_prices": [],
            "bid_sizes": [],
            "ask_prices": [],
            "ask_sizes": [],
        }
        keys = list(rows[0].keys())
        # Dynamically detect depth level
        depth = max(
            int(k.replace("ask_price", "").replace("ask_size", "")
                 .replace("bid_price", "").replace("bid_size", ""))
            for k in keys
            if k.startswith(("ask_", "bid_"))
        )
        for r in rows:
            ts = int(r["exchange_timestamp"])
            result["exchange_timestamp"].append(ts)
            result["local_diff"].append(int(r["local_diff"]))
            result["symbol"].append(meta["symbol"])
            result["exchange"].append(meta["exchange"])
            result["market_type"].append(meta["market_type"])
            result["trade_date"].append(ddb_date_int(ts))

            bp, bs, ap, as_ = [], [], [], []
            for i in range(1, depth + 1):
                bp.append(float(r.get(f"bid_price{i}", "0.0")))
                bs.append(float(r.get(f"bid_size{i}", "0.0")))
                ap.append(float(r.get(f"ask_price{i}", "0.0")))
                as_.append(float(r.get(f"ask_size{i}", "0.0")))
            result["bid_prices"].append(bp)
            result["bid_sizes"].append(bs)
            result["ask_prices"].append(ap)
            result["ask_sizes"].append(as_)

    return result


def batch_iter(rows: Iterator[Dict[str, str]], size: int = 5000):
    batch = []
    for r in rows:
        batch.append(r)
        if len(batch) >= size:
            yield batch
            batch = []
    if batch:
        yield batch


def upload_batch(session: ddb.session, table_name: str,
                 data: Dict[str, List[Any]]) -> int:
    if not data or not data.get("exchange_timestamp"):
        return 0
    session.run(
        f"tableInsert{{loadTable('{DB_PATH}', '{table_name}')}}", data
    )
    return len(data["exchange_timestamp"])


def main():
    parser = argparse.ArgumentParser(
        description="Import trade_data CSVs into DolphinDB"
    )
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8848)
    parser.add_argument("--user", default="admin")
    parser.add_argument("--password", default="123456")
    parser.add_argument("--batch", type=int, default=5000,
                        help="rows per batch")
    parser.add_argument("--data-dir", type=Path, default=ROOT,
                        help="path to trade_data dir")
    args = parser.parse_args()

    if not args.data_dir.exists():
        print(f"ERROR: data dir not found: {args.data_dir}")
        return 1

    s = ddb.session()
    try:
        s.connect(args.host, args.port, args.user, args.password)
    except Exception as e:
        print(f"ERROR: failed to connect to DolphinDB: {e}")
        return 1

    print(f"Connected to DolphinDB {args.host}:{args.port}")

    csv_files = sorted(args.data_dir.rglob("*.csv"))
    print(f"Found {len(csv_files)} CSV files under {args.data_dir}")

    total_rows = 0
    for csv_path in csv_files:
        meta = parse_path(csv_path)
        table_name = meta["data_type"]
        label = (
            f"[{meta['exchange']}/{meta['market_type']}/{meta['symbol']}] "
            f"{table_name} ({meta['trade_date']})"
        )
        print(f"Importing {label} ...", end=" ", flush=True)

        rows_inserted = 0
        with open(csv_path, newline="", encoding="utf-8") as f:
            reader = csv.DictReader(f)
            for batch in batch_iter(reader, args.batch):
                data = parse_rows(batch, meta)
                n = upload_batch(s, table_name, data)
                rows_inserted += n

        print(f"{rows_inserted} rows")
        total_rows += rows_inserted

    print(f"\nAll done. Total rows inserted: {total_rows}")
    s.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
