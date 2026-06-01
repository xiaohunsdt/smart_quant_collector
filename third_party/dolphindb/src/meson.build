inc = include_directories('../include', '../include/lz4')
src = files(
    'AsynWorker.cpp',
    'BatchTableWriter.cpp',
    'Compress.cpp',
    'Concurrent.cpp',
    'ConstantImp.cpp',
    'ConstantMarshall.cpp',
    'Crypto.cpp',
    'DBConnectionPoolImpl.cpp',
    'DFSChunkMeta.cpp',
    'DictionaryImp.cpp',
    'DolphinDB.cpp',
    'DolphinDBImp.cpp',
    'DomainImp.cpp',
    'EventHandler.cpp',
    'Format.cpp',
    'Guid.cpp',
    'int128.cpp',
    'Logger.cpp',
    'Matrix.cpp',
    'MultithreadedTableWriter.cpp',
    'ScalarImp.cpp',
    'SetImp.cpp',
    'SharedMem.cpp',
    'Streaming.cpp',
    'StreamingUtil.cpp',
    'SymbolBase.cpp',
    'SysIO.cpp',
    'TableImp.cpp',
    'TaskStatusMgmt.cpp',
    'Types.cpp',
    'Util.cpp',
    'Vector.cpp',
    'lz4/lz4.c'
)

if host_machine.system() == 'linux'
    src += files('platform/Linux.cpp')
elif host_machine.system() == 'windows'
    src += files('platform/Windows.cpp')
endif
