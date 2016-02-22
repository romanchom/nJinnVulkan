#ifndef VERTEXJUDGE_H
#define VERTEXJUDGE_H

template<typename ScoreType, int scoreMultiplier = 1>
class VertexJudge{
public:
	VertexJudge(uint32_t cacheSize, uint32_t maxValence);
	ScoreType getScore(uint32_t cachePosition, uint32_t valence);
private:
	ScoreType getPositionScore(uint32_t cachePosition);
	ScoreType getValenceScore(uint32_t valence);
	ScoreType * _cachePositionTable;
	ScoreType * _valenceTable;
	uint32_t _cacheSize;
	uint32_t _maxValence;
};


template<typename ScoreType, int scoreMultiplier>
VertexJudge<ScoreType, scoreMultiplier>::VertexJudge(uint32_t cacheSize, uint32_t maxValence) : 
	_cacheSize(cacheSize + 3),
	_maxValence(maxValence)
{
	_cachePositionTable = new ScoreType[_cacheSize];
	_valenceTable = new ScoreType[maxValence];
	for (uint32_t i = 0; i < _cacheSize; ++i){
		_cachePositionTable[i] = getPositionScore(i);
	}
	for (uint32_t i = 0; i < maxValence; ++i){
		_valenceTable[i] = getValenceScore(i);
	}
}


template<typename ScoreType, int scoreMultiplier>
ScoreType VertexJudge<ScoreType, scoreMultiplier>::getPositionScore(uint32_t cachePosition){
	static const double cacheDecayPower = 1.5;
	static const double lastTriScore = 0.75;
	double score(0);
	if (cachePosition >= _cacheSize){
		score = 0;
	}else if (cachePosition < 3){
		score = lastTriScore;
	}else{
		score = 1.0 - ((double)(cachePosition - 3) / (double)(_cacheSize - 3));
		score = pow(score, cacheDecayPower);
	}
	return (ScoreType) (score * scoreMultiplier);
}


template<typename ScoreType, int scoreMultiplier>
ScoreType VertexJudge<ScoreType, scoreMultiplier>::getValenceScore(uint32_t valence){
	static const double valenceBoostScale = 2.0;
	static const double valenceBoostPower = 0.5;
	double score(0);
	if (valence == 0) {
		score = -1.0;
	}else{
		score = valenceBoostScale * pow(valence, -valenceBoostPower);
	}
	return (ScoreType)(score * scoreMultiplier);
}


template<typename ScoreType, int scoreMultiplier>
ScoreType VertexJudge<ScoreType, scoreMultiplier>::getScore(uint32_t cachePosition, uint32_t valence){
	ScoreType pos, val;
	if (valence == 0) return -1 * scoreMultiplier;
	if (cachePosition < _cacheSize){
		pos = _cachePositionTable[cachePosition];
	}else{
		pos = getPositionScore(cachePosition);
	}

	if (valence < _maxValence){
		val = _valenceTable[valence];
	}else{
		val = getValenceScore(valence);
	}

	return pos + val;
}

#endif // VERTEXJUDGE_H