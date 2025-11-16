
#ifndef IR_TYPES_COMMON_H
#define IR_TYPES_COMMON_H

#include "ir_types_custom.h"
#include <iostream>
#include <map>
#include <string>

using std::cerr;
using std::map;
using std::string;

#define ALLIRSYMBOLTYPE(V) \
    V(SymbolUnknown)       \
    V(SymbolNonTerm)       \
    V(SymbolLit)           \
    V(SymbolIden)          \
    V(SymbolTerm)          \
    V(SymbolCustomTerm)

#define ALLDATATYPE(V)            \
    V(DataNone)                   \
    V(DataUnknownType)            \
    V(DataChangeFeed)             \
    V(DataDatabaseName)           \
    V(DataSuperRegion)            \
    V(DataRoleName)               \
    V(DataCatalogName)            \
    V(DataSchemaName)             \
    V(DataFunctionName)           \
    V(DataFunctionExpr)           \
    V(DataExtensionName)          \
    V(DataCollationName)          \
    V(DataColumnName)             \
    V(DataConstraintName)         \
    V(DataViewName)               \
    V(DataSequenceName)           \
    V(DataTableName)              \
    V(DataRegionName)             \
    V(DataTemplateName)           \
    V(DataEncodingName)           \
    V(DataCTypeName)              \
    V(DataIndexName)              \
    V(DataTypeName)               \
    V(DataPartitionName)          \
    V(DataRangeName)              \
    V(DataFamilyName)             \
    V(DataStatsName)              \
    V(DataSettingName)            \
    V(DataSavePointName)          \
    V(DataPrivilege)              \
    V(DataWindowName)             \
    V(DataStatementPreparedName)  \
    V(DataCursorName)             \
    V(DataZoneName)               \
    V(DataChannelName)            \
    V(DataTableAliasName)         \
    V(DataColumnAliasName)        \
    V(DataLiteral)                \
    V(DataViewColumnName)         \
    V(DataStatisticsName)         \
    V(DataJobName)                \
    V(DataForeignKeyName)         \
    V(DataStorageName)            \
    V(DataGenericOptionName)      \
    V(DataGenericOptionArgs)      \
    V(DataClassName)              \
    V(DataPragmaName)             \
    V(DataPragmaValue)            \
    V(DataSecretName)             \
    V(DataCompressionName)        \
    V(DataRelOptionName)          \
    V(DataRelOptionArgs)          \
    V(DataAccessMethodName)       \
    V(DataEventName)              \
    V(DataServerName)             \
    V(DataPolicyName)             \
    V(DataRuleName)               \
    V(DataTriggerName)            \
    V(DataSamplingFuncName)       \
    V(DataParamName)              \
    V(DataAttrName)               \
    V(DataCheckPointName)         \
    V(DataLocalFileName)          \
    V(DataDatabaseAliasName)      \
    V(DataTransactionName)        \
    V(DataVirtualTableModuleName) \
    V(DataVariableName)           \
    V(DataTableSpaceName)         \
    V(DataCharsetName)            \
    V(DataProcedureName)          \
    V(DataStorageParams)

#define ALLDATAAFFINITYTYPE(V)     \
    V(AFFIUNKNOWN)                 \
    V(AFFIANY)                     \
    V(AFFIBIT)                     \
    V(AFFIBOOL)                    \
    V(AFFIBYTES)                   \
    V(AFFICOLLATE)                 \
    V(AFFIDATE)                    \
    V(AFFIENUM)                    \
    V(AFFIDECIMAL)                 \
    V(AFFIFLOAT)                   \
    V(AFFIINET)                    \
    V(AFFIINT)                     \
    V(AFFIINTERVAL)                \
    V(AFFIINTERVALTZ)              \
    V(AFFIJSONB)                   \
    V(AFFIOID)                     \
    V(AFFISERIAL)                  \
    V(AFFISTRING)                  \
    V(AFFITIME)                    \
    V(AFFITIMETZ)                  \
    V(AFFITIMESTAMP)               \
    V(AFFITIMESTAMPTZ)             \
    V(AFFIUUID)                    \
    V(AFFIGEOGRAPHY)               \
    V(AFFIGEOMETRY)                \
    V(AFFIBOX2D)                   \
    V(AFFIVOID)                    \
    V(AFFIPOINT)                   \
    V(AFFILINESTRING)              \
    V(AFFIPOLYGON)                 \
    V(AFFIMULTIPOINT)              \
    V(AFFIMULTILINESTRING)         \
    V(AFFIMULTIPOLYGON)            \
    V(AFFIGEOMETRYCOLLECTION)      \
    V(AFFIOIDWRAPPER)              \
    V(AFFIWHOLESTMT)               \
    V(AFFIONOFF)                   \
    V(AFFIONOFFAUTO)               \
    V(AFFIARRAY)                   \
    V(AFFIARRAYANY)                \
    V(AFFIARRAYUNKNOWN)            \
    V(AFFIARRAYBIT)                \
    V(AFFIARRAYBOOL)               \
    V(AFFIARRAYBYTES)              \
    V(AFFIARRAYCOLLATE)            \
    V(AFFIARRAYDATE)               \
    V(AFFIARRAYENUM)               \
    V(AFFIARRAYDECIMAL)            \
    V(AFFIARRAYFLOAT)              \
    V(AFFIARRAYINET)               \
    V(AFFIARRAYINT)                \
    V(AFFIARRAYINTERVAL)           \
    V(AFFIARRAYJSONB)              \
    V(AFFIARRAYOID)                \
    V(AFFIARRAYSERIAL)             \
    V(AFFIARRAYSTRING)             \
    V(AFFIARRAYTIME)               \
    V(AFFIARRAYTIMETZ)             \
    V(AFFIARRAYTIMESTAMP)          \
    V(AFFIARRAYTIMESTAMPTZ)        \
    V(AFFIARRAYUUID)               \
    V(AFFIARRAYGEOGRAPHY)          \
    V(AFFIARRAYGEOMETRY)           \
    V(AFFIARRAYBOX2D)              \
    V(AFFIARRAYVOID)               \
    V(AFFIARRAYPOINT)              \
    V(AFFIARRAYLINESTRING)         \
    V(AFFIARRAYPOLYGON)            \
    V(AFFIARRAYMULTIPOINT)         \
    V(AFFIARRAYMULTILINESTRING)    \
    V(AFFIARRAYMULTIPOLYGON)       \
    V(AFFIARRAYGEOMETRYCOLLECTION) \
    V(AFFIARRAYOIDWRAPPER)         \
    V(AFFIARRAYWHOLESTMT)          \
    V(AFFIARRAYONOFF)              \
    V(AFFIARRAYONOFFAUTO)          \
    V(AFFITABLENAME)               \
    V(AFFICOLUMNNAME)              \
    V(AFFICONSTRAINTNAME)          \
    V(AFFITUPLE)

#define ALLCONTEXTFLAGS(V)    \
    V(ContextUnknown)         \
    V(ContextDefine)          \
    V(ContextUse)             \
    V(ContextUseTop)          \
    V(ContextUndefine)        \
    V(ContextReplaceDefine)   \
    V(ContextReplaceUndefine) \
    V(ContextNoModi)          \
    V(ContextUseFollow)

#define ALLFUNCTIONTYPES(V) \
    V(FUNCAGGR)             \
    V(FUNCWINDOW)           \
    V(FUNCNORMAL)           \
    V(FUNCARRAY)            \
    V(FUNCENUM)             \
    V(FUNCBOOL)             \
    V(FUNCCOMPARE)          \
    V(FUNCCRYPTO)           \
    V(FUNCDATETIME)         \
    V(FUNCDECIMAL)          \
    V(FUNCFLOAT)            \
    V(FUNCUUID)             \
    V(FUNCINET)             \
    V(FUNCINT)              \
    V(FUNCJSONB)            \
    V(FUNCARRAYSTRING)      \
    V(FUNCSEQUENCE)         \
    V(FUNCSTREAM)           \
    V(FUNCSTRING)           \
    V(FUNCSYSTEMINFO)       \
    V(FUNCTIMETZ)           \
    V(FUNCSYSTEMREPAIR)     \
    V(FUNCUNKNOWN)

#define ALLCOLLATIONTYPES(V) \
    V(defaultcollation)      \
    V(und)                   \
    V(aa)                    \
    V(af)                    \
    V(ar)                    \
    V(as)                    \
    V(az)                    \
    V(be)                    \
    V(bg)                    \
    V(bn)                    \
    V(bs)                    \
    V(ca)                    \
    V(cs)                    \
    V(cy)                    \
    V(da)                    \
    V(de)                    \
    V(dz)                    \
    V(ee)                    \
    V(el)                    \
    V(en)                    \
    V(eo)                    \
    V(es)                    \
    V(et)                    \
    V(fa)                    \
    V(fi)                    \
    V(fil)                   \
    V(fo)                    \
    V(fr)                    \
    V(gu)                    \
    V(ha)                    \
    V(haw)                   \
    V(he)                    \
    V(hi)                    \
    V(hr)                    \
    V(hu)                    \
    V(hy)                    \
    V(ig)                    \
    V(is)                    \
    V(ja)                    \
    V(kk)                    \
    V(kl)                    \
    V(km)                    \
    V(kn)                    \
    V(ko)                    \
    V(kok)                   \
    V(ln)                    \
    V(lt)                    \
    V(lv)                    \
    V(mk)                    \
    V(ml)                    \
    V(mr)                    \
    V(mt)                    \
    V(my)                    \
    V(nb)                    \
    V(nn)                    \
    V(nso)                   \
    V(om)                    \
    V(pa)                    \
    V(pl)                    \
    V(ps)                    \
    V(ro)                    \
    V(ru)                    \
    V(se)                    \
    V(si)                    \
    V(sk)                    \
    V(sl)                    \
    V(sq)                    \
    V(sr)                    \
    V(ssy)                   \
    V(sv)                    \
    V(ta)                    \
    V(te)                    \
    V(th)                    \
    V(tn)                    \
    V(to)                    \
    V(tr)                    \
    V(uk)                    \
    V(ur)                    \
    V(vi)                    \
    V(wae)                   \
    V(yo)                    \
    V(zh)

enum SYMBOLTYPE {
#define DECLARE_TYPE(v) v,
    ALLIRSYMBOLTYPE(DECLARE_TYPE)
#undef DECLARE_TYPE
};

enum IRTYPE {
#define DECLARE_TYPE(v) v,
    IRTypeRoot,
    IRTypeAllStmtList,
    ALLIRTYPE(DECLARE_TYPE)
#undef DECLARE_TYPE
};

enum DATATYPE {
#define DECLARE_TYPE(v) v,
    ALLDATATYPE(DECLARE_TYPE)
#undef DECLARE_TYPE
};

// A more detailed version of Data Type. More information to fix the literal
// types.
enum DATAAFFINITYTYPE {
#define DECLARE_TYPE(v) v,
    ALLDATAAFFINITYTYPE(DECLARE_TYPE)
#undef DECLARE_TYPE
};

enum DATAFLAG {
#define DECLARE_TYPE(v) v,
    ALLCONTEXTFLAGS(DECLARE_TYPE)
#undef DECLARE_TYPE
};

enum FUNCTIONTYPE {
#define DECLARE_TYPE(v) v,
    ALLFUNCTIONTYPES(DECLARE_TYPE)
#undef DECLARE_TYPE
};

static inline string get_string_by_symbol_type(SYMBOLTYPE type)
{

#define DECLARE_CASE(classname) \
    if (type == classname)      \
        return #classname;
    ALLIRSYMBOLTYPE(DECLARE_CASE);
#undef DECLARE_CASE

    return "";
}

static inline string get_string_by_ir_type(IRTYPE type)
{

#define DECLARE_CASE(classname) \
    if (type == classname)      \
        return #classname;
    ALLIRTYPE(DECLARE_CASE);
#undef DECLARE_CASE

    return "";
}

static inline string get_string_by_data_type(DATATYPE type)
{

#define DECLARE_CASE(classname) \
    if (type == classname)      \
        return #classname;
    ALLDATATYPE(DECLARE_CASE);
#undef DECLARE_CASE

    return "";
}

static inline string get_string_by_data_flag(DATAFLAG flag_type_)
{
#define DECLARE_CASE(classname)  \
    if (flag_type_ == classname) \
        return #classname;
    ALLCONTEXTFLAGS(DECLARE_CASE);
#undef DECLARE_CASE
    return "";
}

static inline string get_string_by_data_affi(DATAAFFINITYTYPE flag_type_)
{
#define DECLARE_CASE(classname)  \
    if (flag_type_ == classname) \
        return #classname;
    ALLDATAAFFINITYTYPE(DECLARE_CASE);
#undef DECLARE_CASE
    return "";
}

static inline SYMBOLTYPE get_symbol_type_by_string(string s)
{
#define DECLARE_CASE(datatypename) \
    if (s == #datatypename)        \
        return datatypename;
    ALLIRSYMBOLTYPE(DECLARE_CASE);
#undef DECLARE_CASE

    cerr << "\n\n\nError: Cannot find the matching data type by"
            " string: "
            + s + " \n\n\n";
    return SymbolUnknown;
}

static inline IRTYPE get_ir_type_by_string(string s)
{
#define DECLARE_CASE(datatypename) \
    if (s == #datatypename)        \
        return datatypename;
    ALLIRTYPE(DECLARE_CASE);
#undef DECLARE_CASE

    cerr << "\n\n\nError: Cannot find the matching data type by"
            " string: "
            + s + " \n\n\n";
    return IRTypeUnknownType;
}

static inline DATATYPE get_datatype_by_string(string s)
{
#define DECLARE_CASE(datatypename) \
    if (s == #datatypename)        \
        return datatypename;
    ALLDATATYPE(DECLARE_CASE);
#undef DECLARE_CASE

    cerr << "\n\n\nError: Cannot find the matching data type by"
            " string: "
            + s + " \n\n\n";
    return DataUnknownType;
}

static inline DATAFLAG get_dataflag_by_string(string s)
{
#define DECLARE_CASE(datatypename) \
    if (s == #datatypename)        \
        return datatypename;       \
    ALLCONTEXTFLAGS(DECLARE_CASE);
#undef DECLARE_CASE

    cerr << "\n\n\nError: Cannot find the matching data type by"
            " string: "
            + s + " \n\n\n";
    return ContextUnknown;
}

static inline DATAAFFINITYTYPE get_dataaffi_by_string(string s)
{
#define DECLARE_CASE(datatypename) \
    if (s == #datatypename)        \
        return datatypename;
    ALLDATAAFFINITYTYPE(DECLARE_CASE);
#undef DECLARE_CASE

    cerr << "\n\n\nError: Cannot find the matching data type by"
            " string: "
            + s + " \n\n\n";
    return AFFIUNKNOWN;
}

static inline FUNCTIONTYPE get_functype_by_string(string s)
{
#define DECLARE_CASE(functiontypename) \
    if (s == #functiontypename)        \
        return functiontypename;
    ALLFUNCTIONTYPES(DECLARE_CASE);
#undef DECLARE_CASE
    cerr << "\n\n\nError: Cannot find the matching function type by"
            " string: "
            + s + " \n\n\n";
    //    abort();
    return FUNCUNKNOWN;
}

static inline DATAAFFINITYTYPE get_data_affinity_by_idx(int idx)
{
    return static_cast<DATAAFFINITYTYPE>(idx);
}

#endif // IR_TYPES_COMMON_H
