#ifndef IPROCESS_H
#define IPROCESS_H

__interface IProcess {

    long Process(void * pCaller, Model * pModel, int nProgress = 0, int nLevel = 1);
    // return processed data pointer to object staring at dwOffset
    void * GetProcessedObject(LPCTSTR szName, int selectedChannel, int numChannels, int sampleSize, DWORD dwIndex, size_t sObjectSize, BOOL bReverse=FALSE);
    // return the size of the data in bytes for a single channel
    DWORD GetProcessedModelWaveDataSize();
    //  return the number of samples for a single channel
    DWORD GetNumSamples(Model * pModel) const;
};

#endif
