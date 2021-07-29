(:**************************************************************:)
(: Test: functx-fn-contains-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(contains('query', 'e'), contains('query', 'ery'), contains('query', 'query'), contains('query', 'x'), contains('query', ''), contains('query', ()), contains( (), 'q'))