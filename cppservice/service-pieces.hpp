#pragma once
#include <iostream>
using namespace std;


class Pieces{
public:
    uint64_t onlyId;
    uint64_t excelId;
    
    int LoadData(int);
    int PrintData();
};

int Pieces::LoadData(int id){
    printf("loading data: %d\n", id);
}

int Pieces::PrintData(){
    printf("printing data: %d\n");
}
