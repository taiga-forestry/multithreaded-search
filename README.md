# Multi-Threaded Search

A concurrent, thread-safe search engine that implements [Google's PageRank algorithm](https://en.wikipedia.org/wiki/PageRank#:~:text=PageRank%20(PR)%20is%20an%20algorithm,the%20importance%20of%20website%20pages.) and [tf-idf scoring](https://en.wikipedia.org/wiki/Tf%E2%80%93idf) to return relevant results for a given query!

This search engine consists of an Indexer and a Querier, where the indexer preprocesses a corpus of wiki pages to store information about which documents are relevant to which words. Then, the querier is used to respond to user search queries in a time-efficient manner.

However, the indexing process can be quite slow, especially on a corpus of thousands of wiki pages with tens of thousands of words in total. By utilizing threads along with synchronization primitives in C++, most notably mutexes, we can parallelize expensive computations (such as computing term-relevance scores for all word-title pairs) to evenly distribute the workload across multiple threads and make the indexing process more efficient. In particular, this multi-threaded search engine offers indexing speeds 2-3 times faster compared to its equivalent single-threaded version.
