#include <Windows.h>
#include <vector>

using std::vector;

void extractChannel( WORD channel, WORD numChannels, WORD blockAlign, vector<char> & in, vector<char> & out) 
{

	out.clear();
	if (in.size()==0) return;

	DWORD numSamples = in.size()/blockAlign;
	WORD newBlockAlign = blockAlign/numChannels;
	WORD remainder = blockAlign-newBlockAlign;
	// calculate starting index
	DWORD i = channel*newBlockAlign;
	for (int s=0;s<numSamples;s++) 
	{
		for (int a=0;a<newBlockAlign;a++) 
		{
			out.push_back(in[i++]);
		}
		i += remainder;
	}
}

