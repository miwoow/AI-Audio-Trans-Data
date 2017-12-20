#!/bin/env python
#encoding:utf-8


import jieba

def do():
    fh = open('vocabulary.txt', 'r')
    fh2 = open('vocabulary.txt.2', 'w')
    for line in fh.readlines():
        line = line.strip()
        seg_list = jieba.cut(line, cut_all=False)
        n_line = ' '.join(seg_list)
        n_line = n_line.encode('utf-8')
        fh2.write(n_line + '\n')
    fh.close()
    fh2.close()

if __name__ == '__main__':
    do()
