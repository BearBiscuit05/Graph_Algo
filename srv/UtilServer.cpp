//
// Created by Thoh Testarossa on 2019-04-05.
//

#include "../srv/UtilServer.h"
#include "../util/TIsExtended.hpp"
#include <string>
#include <iostream>

template <typename T>
UtilServer<T>::UtilServer(int vCount, int eCount, int numOfInitV, int nodeNo)
{
    //Test
    std::cout << "Server init" << std::endl;
    //Test end

    this->nodeNo = nodeNo;

    this->vCount = vCount;
    this->eCount = eCount;
    this->numOfInitV = numOfInitV;

    this->isLegal = TIsExtended<T, GraphUtil>::Result &&
                    vCount > 0 &&
                    eCount > 0 &&
                    numOfInitV > 0 &&
                    nodeNo >= 0;

    this->vValues = nullptr;
    this->vSet = nullptr;
    this->eSet = nullptr;
    this->initVSet = nullptr;
    this->filteredV = nullptr;
    this->filteredVCount = nullptr;

    if(this->isLegal)
    {
        int chk = 0;

        this->executor = T();
        this->executor.Deploy(vCount, numOfInitV);

        this->vValues_shm = UNIX_shm();
        this->vSet_shm = UNIX_shm();
        this->eSet_shm = UNIX_shm();
        this->initVSet_shm = UNIX_shm();
        this->filteredV_shm = UNIX_shm();
        this->filteredVCount_shm = UNIX_shm();

        this->server_msq = UNIX_msg();
        this->client_msq = UNIX_msg();

        if(chk != -1)
            chk = this->vValues_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (VVALUES_SHM << SHM_OFFSET)),
                this->vCount * this->numOfInitV * sizeof(double),
                0666);
        if(chk != -1)
            chk = this->vSet_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (VSET_SHM << SHM_OFFSET)),
                this->vCount * sizeof(Vertex),
                0666);
        if(chk != -1)
            chk = this->eSet_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (ESET_SHM << SHM_OFFSET)),
                this->eCount * sizeof(Edge),
                0666);
        if(chk != -1)
            chk = this->initVSet_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (INITVSET_SHM << SHM_OFFSET)),
                this->numOfInitV * sizeof(int),
                0666);
        if(chk != -1)
            chk = this->filteredV_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (FILTEREDV_SHM << SHM_OFFSET)),
                this->vCount * sizeof(bool),
                0666);
        if(chk != -1)
            chk = this->filteredVCount_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (FILTEREDVCOUNT_SHM << SHM_OFFSET)),
                sizeof(int),
                0666);

        if(chk != -1)
            chk = this->server_msq.create(((this->nodeNo << NODE_NUM_OFFSET) | (SRV_MSG_TYPE << MSG_TYPE_OFFSET)),
                0666);
        if(chk != -1)
            chk = this->client_msq.create(((this->nodeNo << NODE_NUM_OFFSET) | (CLI_MSG_TYPE << MSG_TYPE_OFFSET)),
                0666);

        if(chk != -1)
        {
            this->vValues_shm.attach(0666);
            this->vSet_shm.attach(0666);
            this->eSet_shm.attach(0666);
            this->initVSet_shm.attach(0666);
            this->filteredV_shm.attach(0666);
            this->filteredVCount_shm.attach(0666);

            this->vValues = (double *) this->vValues_shm.shmaddr;
            this->vSet = (Vertex *) this->vSet_shm.shmaddr;
            this->eSet = (Edge *) this->eSet_shm.shmaddr;
            this->initVSet = (int *) this->initVSet_shm.shmaddr;
            this->filteredV = (bool *) this->filteredV_shm.shmaddr;
            this->filteredVCount = (int *) this->filteredVCount_shm.shmaddr;

            //Test
            std::cout << "Init succeeded." << std::endl;
            //Test end
        }
        else
        {
            this->isLegal = false;

            //Test
            std::cout << "Init failed with errno " << errno << std::endl;
            //Test end
        }
    }
}

template <typename T>
UtilServer<T>::~UtilServer()
{
    this->executor.Free();

    this->vValues = nullptr;
    this->vSet = nullptr;
    this->eSet = nullptr;
    this->initVSet = nullptr;
    this->filteredV = nullptr;
    this->filteredVCount = nullptr;

    this->vValues_shm.control(IPC_RMID);
    this->vSet_shm.control(IPC_RMID);
    this->eSet_shm.control(IPC_RMID);
    this->initVSet_shm.control(IPC_RMID);
    this->filteredV_shm.control(IPC_RMID);
    this->filteredVCount_shm.control(IPC_RMID);

    this->server_msq.control(IPC_RMID);
    this->client_msq.control(IPC_RMID);
}

template <typename T>
void UtilServer<T>::run()
{
    if(!this->isLegal) return;

    double *mValues = new double [this->vCount * this->numOfInitV];
    char msgp[256];
    std::string cmd = std::string("");

    int iterCount = 0;

    while(this->client_msq.recv(msgp, (CLI_MSG_TYPE << MSG_TYPE_OFFSET), 256) != -1)
    {
        //Test
        std::cout << "Processing at iter " << ++iterCount << std::endl;
        //Test end

        cmd = msgp;
        if(std::string("execute") == cmd)
        {
            for (int i = 0; i < this->vCount * this->numOfInitV; i++) mValues[i] = INVALID_MASSAGE;

            this->executor.MSGGenMerge_array(this->vCount, this->eCount, this->vSet, this->eSet, this->numOfInitV, this->initVSet, this->vValues, mValues);

            this->executor.MSGApply_array(this->vCount, this->vSet, this->numOfInitV, this->initVSet, this->vValues, mValues);

            this->server_msq.send("finished", (SRV_MSG_TYPE << MSG_TYPE_OFFSET), 256);
        }

        else if(std::string("exit") == cmd)
            break;
        else break;
    }

    //Test
    std::cout << "Shutdown properly" << std::endl;
    //Test end
}

