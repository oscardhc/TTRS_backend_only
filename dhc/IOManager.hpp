//
// Created by Haichen Dong on 2019-05-10.
//

#ifndef DEMO1_IOMANAGER_HPP
#define DEMO1_IOMANAGER_HPP

#include <stdio.h>
#include <string.h>
#include "map.hpp"
#include "utility.hpp"
#include <map>
#include "listMap.hpp"
//#include <filesystem>
#include "../whj/splay_new.hpp"
#include <unistd.h>

// 8K per page (There should be NO elements with sizes exceeding 8K)
const int pageSize = 8192;
// Bufferpool size: 8K * 128 = 1M
const int maxBufferSize = 1450;

const int FILE_CNT = 6;
enum FOR_FILE{BPT = 0, USER = 1, TRAIN = 2, STATION = 3, TRID = 4, RECORD = 5};

namespace sjtu {

    class IOManager {
    private:
        char tmp[pageSize];
        char *pool[maxBufferSize];
        int poolAge;
        int totalQuery, hitQuery;
        int _bufferSize;

        int fileCount;
        FILE *file[FILE_CNT];
        int fileSize[FILE_CNT];

        struct Node {
            int index; // Page id (1,2,3...) in hard drive file
            int value; // Pool of this page
            int age;
            short isEdited;
        };

//        sjtu::map<int, Node*> ageMap, idxMap;
//        void moveToFront(Node* node) {
//            if ((--ageMap.end())->first != node->age) {
//                ageMap.erase(ageMap.find(node->age));
//                node->age = ++poolAge;
//                ageMap.insert({node->age, node});
//            }
//        }

        SplayTree<int, Node*> ageMap, idxMap;
        void moveToFront(Node* node) {
            if (ageMap.getmax() != node->age) {
                ageMap.remove(node->age);
                node->age = ++poolAge;
                ageMap.insert(node->age, node);
            }
        }

    public:
        IOManager(int _cnt) {
            _bufferSize = 150;
            fileCount = _cnt;
            char filename[10];
            for (int i = 0; i < fileCount; i++) {
                sprintf(filename, "data_%d", i);
                FILE *tmp = fopen(filename, "a");
                fclose(tmp);
                file[i] = fopen(filename, "r+");
                int ret = fread((char*)(&fileSize[i]), sizeof(int), 1, file[i]);
                if (ret == 0) {
                    fileSize[i] = sizeof(int);
                    fwrite((char*)(&fileSize[i]), sizeof(int), 1, file[i]);
                }
            }
            for (int i = 0; i < _bufferSize; i++) {
                pool[i] = new char[pageSize];
            }
            totalQuery = hitQuery = 0;
//            memset(pool, 0, sizeof(pool));
            for (int i = 0; i < _bufferSize; i++) {
                Node *node = new Node();
                node->index = - i - 1;
                node->value = i;
                node->age = ++poolAge;
                node->isEdited = -1;
//                ageMap.insert({node->age, node});
//                idxMap.insert({node->index, node});
                ageMap.insert(node->age, node);
                idxMap.insert(node->index, node);
            }
        }
        ~IOManager() {
//            for (auto it = idxMap.begin(); it != idxMap.end(); it++) {
//                if (it->second->isEdited == 1) {
//                    flushToDisk(it->second);
//                }
//                delete it->second;
//            }
            while (idxMap.size) {
                int key = idxMap.getRootKey();
                Node *node = idxMap.search(key);
                if (node->isEdited == 1) {
                    flushToDisk(node);
                }
                idxMap.remove(key);
                delete node;
            }
            char filename[10];
            for (int i = 0; i < fileCount; i++) {
                sprintf(filename, "data_%d", i);
                fseek(file[i], 0, SEEK_SET);
                fwrite((char*)(&fileSize[i]), sizeof(int), 1, file[i]);
                fclose(file[i]);
//                std::filesystem::resize_file(filename, fileSize[i]);
                truncate(filename, fileSize[i]);
            }
            for (int i = 0; i < _bufferSize; i++) {
                delete[] pool[i];
            }
            fprintf(stderr, "TOTAL %u HIT %u RATIO %lf\n", totalQuery, hitQuery, 1.0 * hitQuery / totalQuery);
        }
        void flushToDisk(Node* node) {
            int nfile = node->index % 10, idx = node->index / 10;
//            fprintf(stderr, "FLUSH %d %d %d -> %d\n", nfile, idx, *((int*)&pool[node->value][948]), node->value);
            fseek(file[nfile], idx * pageSize, SEEK_SET);
            fwrite(pool[node->value], pageSize, 1, file[nfile]);
        }
        void readFromDisk(Node* node) {
//            fprintf(stderr, "READ %d %d -> %d\n", node->index % 10, node->index / 10, node->value);
            fseek(file[node->index % 10], (node->index / 10) * pageSize, SEEK_SET);
            fread(pool[node->value], pageSize, 1, file[node->index % 10]);
        }
        Node* getLastPage() {
//            auto it = ageMap.begin();
            Node* node = ageMap.search(ageMap.getmin());
            if (node->isEdited == 1) {
                flushToDisk(node);
            }
            return node;
        }
        Node* getAvailableMemory(int forIndex) {
            Node* node = getLastPage();
            if (node->index > 0) {
                if (_bufferSize < maxBufferSize) {
                    for (int i = 0; i < 100; i++) {
                        pool[i + _bufferSize] = new char[pageSize];
                        Node *node = new Node();
                        node->index = - i - _bufferSize - 1;
                        node->value = i + _bufferSize;
                        node->age = -i;
                        node->isEdited = -1;
                        ageMap.insert(node->age, node);
                        idxMap.insert(node->index, node);
                    }
                    _bufferSize += 100;
                }
            }
//            ageMap.erase(ageMap.find(node->age));
//            idxMap.erase(idxMap.find(node->index));
            ageMap.remove(node->age);
            idxMap.remove(node->index);
            node->age = ++poolAge;
            node->index = forIndex;
//            ageMap.insert({node->age, node});
//            idxMap.insert({node->index, node});
            ageMap.insert(node->age, node);
            idxMap.insert(node->index, node);
            readFromDisk(node);
            return node;
        }
        Node* getBufferId(int index, int flag) {
            totalQuery++;
//            auto it = idxMap.find(index);
            Node* node = idxMap.search(index);
//            if (it != idxMap.end()) {
            if (node != nullptr) {
                hitQuery++;
//                Node* node = it->second;
                if (flag == 0) moveToFront(node);
                return node;
            } else {
                Node* node = getAvailableMemory(index);
                return node;
            }
        }
        void getElement(char *t, int offset, int elementSize, FOR_FILE nFile = BPT) {
//            if (nFile == TRAIN) fprintf(stderr, "get %d %d\n", offset, elementSize);
            int flag = nFile == USER || nFile == TRAIN;
            int beginIndex = offset / pageSize;
            int pagePosition = offset % pageSize;
            int pageLeft = pageSize - pagePosition;
            if (pageLeft >= elementSize) {
                Node *poolid = getBufferId(beginIndex * 10 + nFile, flag);
                memcpy(t, pool[poolid->value] + pagePosition, elementSize);
            } else if (elementSize <= 8192 + pageLeft) {
                Node *poolid1 = getBufferId(beginIndex * 10 + nFile, flag);
                Node *poolid2 = getBufferId((beginIndex + 1) * 10 + nFile, flag);
                memcpy(t, pool[poolid1->value] + pagePosition, pageLeft);
                memcpy(t + pageLeft, pool[poolid2->value], elementSize - pageLeft);
            } else if (elementSize <= 8192 * 2 + pageLeft) {
                Node *poolid1 = getBufferId(beginIndex * 10 + nFile, flag);
                Node *poolid2 = getBufferId((beginIndex + 1) * 10 + nFile, flag);
                Node *poolid3 = getBufferId((beginIndex + 2) * 10 + nFile, flag);
                memcpy(t, pool[poolid1->value] + pagePosition, pageLeft);
                memcpy(t + pageLeft, pool[poolid2->value], 8192);
                memcpy(t + pageLeft + 8192, pool[poolid3->value], elementSize - pageLeft - 8192);
            } else {
                Node *poolid1 = getBufferId(beginIndex * 10 + nFile, flag);
                Node *poolid2 = getBufferId((beginIndex + 1) * 10 + nFile, flag);
                Node *poolid3 = getBufferId((beginIndex + 2) * 10 + nFile, flag);
                Node *poolid4 = getBufferId((beginIndex + 3) * 10 + nFile, flag);
                memcpy(t, pool[poolid1->value] + pagePosition, pageLeft);
                memcpy(t + pageLeft, pool[poolid2->value], 8192);
                memcpy(t + pageLeft + 8192, pool[poolid3->value], 8192);
                memcpy(t + pageLeft + 8192 + 8192, pool[poolid4->value], elementSize - pageLeft - 8192 - 8192);
            }
        }
        int createElement(int elementSize, FOR_FILE nFile = BPT) {
            fileSize[nFile] += elementSize;
            return fileSize[nFile] - elementSize;
        }
        void setElement(char *t, int offset, int elementSize, FOR_FILE nFile = BPT) {
//            if (nFile == TRAIN) fprintf(stderr, "set %d %d\n", offset, elementSize);
            int flag = nFile == USER;
            int beginIndex = offset / pageSize;
            int pagePosition = offset % pageSize;
            int pageLeft = pageSize - pagePosition;
            if (pageLeft >= elementSize) {
                Node* poolid = getBufferId(beginIndex * 10 + nFile, flag);
                memcpy(pool[poolid->value] + pagePosition, t, elementSize);
                poolid->isEdited = true;
            } else if (elementSize <= 8192 + pageLeft){
                Node *poolid1 = getBufferId(beginIndex * 10 + nFile, flag);
                Node *poolid2 = getBufferId((beginIndex + 1) * 10 + nFile, flag);
                memcpy(pool[poolid1->value] + pagePosition, t, pageLeft);
                memcpy(pool[poolid2->value], t + pageLeft, elementSize - pageLeft);
                poolid1->isEdited = true;
                poolid2->isEdited = true;
            } else  if (elementSize <= 8192 + 8192 + pageLeft) {
                Node *poolid1 = getBufferId(beginIndex * 10 + nFile, flag);
                Node *poolid2 = getBufferId((beginIndex + 1) * 10 + nFile, flag);
                Node *poolid3 = getBufferId((beginIndex + 2) * 10 + nFile, flag);
                memcpy(pool[poolid1->value] + pagePosition, t, pageLeft);
                memcpy(pool[poolid2->value], t + pageLeft, 8192);
                memcpy(pool[poolid3->value], t + pageLeft + 8192, elementSize - pageLeft - 8192);
                poolid1->isEdited = true;
                poolid2->isEdited = true;
                poolid3->isEdited = true;
            } else {
                Node *poolid1 = getBufferId(beginIndex * 10 + nFile, flag);
                Node *poolid2 = getBufferId((beginIndex + 1) * 10 + nFile, flag);
                Node *poolid3 = getBufferId((beginIndex + 2) * 10 + nFile, flag);
                Node *poolid4 = getBufferId((beginIndex + 3) * 10 + nFile, flag);
                memcpy(pool[poolid1->value] + pagePosition, t, pageLeft);
                memcpy(pool[poolid2->value], t + pageLeft, 8192);
                memcpy(pool[poolid3->value], t + pageLeft + 8192, 8192);
                memcpy(pool[poolid4->value], t + pageLeft + 8192 + 8192, elementSize - pageLeft - 8192 - 8192);
                poolid1->isEdited = true;
                poolid2->isEdited = true;
                poolid3->isEdited = true;
                poolid4->isEdited = true;
            }
        }

        int createElementVirt(int elementSize, FOR_FILE nFile = BPT) {
            return createElement(elementSize - 8, nFile);
        }
        void getElementVirt(char *t, int offset, int elementSize, FOR_FILE nFile = BPT) {
            getElement(t + 8, offset, elementSize - 8, nFile);
        }
        void setElementVirt(char *t, int offset, int elementSize, FOR_FILE nFile = BPT) {
            setElement(t + 8, offset, elementSize - 8, nFile);
        }
    };

}

#endif //DEMO1_IOMANAGER_HPP
