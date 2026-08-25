#!/usr/bin/env python3
"""PStore 打包辅助：解包 AMR → 注入 props → 重打包。
用法: python3 inject-props.py <appid> [amr路径]
"""
import json
import os
import shutil
import sys
import zipfile

def main():
    appid = sys.argv[1] if len(sys.argv) > 1 else '800186lbstsmanage'
    amr_dir = sys.argv[2] if len(sys.argv) > 2 else 'ui'
    # 找到 AMR
    amr = None
    for f in os.listdir(amr_dir):
        if f.endswith('.amr') and appid in f:
            amr = os.path.join(amr_dir, f)
            break
    if not amr:
        for f in os.listdir('.'):
            if f.endswith('.amr'):
                amr = f
                break
    if not amr:
        print('ERROR: no amr found')
        sys.exit(1)
    print(f'AMR: {amr}')

    work = '/tmp/amr_repack'
    if os.path.exists(work):
        shutil.rmtree(work)
    os.makedirs(work)

    # 解包
    with zipfile.ZipFile(amr) as z:
        z.extractall(work)
    print('=== 原始 manifest ===')
    with open(os.path.join(work, 'manifest.json')) as f:
        print(f.read())

    # 注入 props
    mp = os.path.join(work, 'manifest.json')
    with open(mp) as f:
        m = json.load(f)
    m['props'] = {
        'addDesktop': {
            'x3s_platform': True,
            'y02_1_platform': True,
        },
        'supportUnInstall': True,
    }
    with open(mp, 'w') as f:
        json.dump(m, f, indent=2, ensure_ascii=False)
    print('=== 注入后 manifest ===')
    with open(mp) as f:
        print(f.read())

    # 重打包
    out_amr = amr  # 覆盖原文件
    with zipfile.ZipFile(out_amr, 'w', zipfile.ZIP_DEFLATED) as z:
        for root, dirs, files in os.walk(work):
            for fn in files:
                p = os.path.join(root, fn)
                z.write(p, os.path.relpath(p, work))
    print(f'=== repacked: {out_amr} ===')
    with zipfile.ZipFile(out_amr) as z:
        for i in z.infolist():
            print(f'  {i.filename} ({i.file_size}B)')

if __name__ == '__main__':
    main()
