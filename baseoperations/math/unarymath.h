#ifndef SIN_H
#define SIN_H

namespace Ilwis {
namespace BaseOperations{
class UnaryMath : public OperationImplementation
{
public:
    enum UnaryOperations{uoSIN, uoCOS, uoTAN, uoSQRT, uoASIN, uoACOS, uoATAN, uoLog10, uoLN, uoABS, uoCEIL,
                         uoFLOOR,uoCOSH, uoEXP, uoNEG,uoRND,uoSGN,uoSINH,uoTANH};
    UnaryMath();
    UnaryMath(quint64 metaid, const Ilwis::OperationExpression &expr);

    bool execute(ExecutionContext *ctx=0);
    static Ilwis::OperationImplementation *create(quint64 metaid,const Ilwis::OperationExpression& expr);
    Ilwis::OperationImplementation::State prepare();

    static quint64 createMetadata();
private:
    IGridCoverage _inputGC;
    IGridCoverage _outputGC;
    Box3D<qint32> _box;
    bool _spatialCase;
    double _number;

};
}
}

#endif // SIN_H