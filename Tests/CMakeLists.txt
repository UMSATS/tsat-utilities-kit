cmake_minimum_required(VERSION 3.23)

add_executable(TxCacheTests)
target_sources(TxCacheTests 
	PRIVATE 
    tx_cache.c
)

target_link_libraries(TxCacheTests PRIVATE TxCache)
