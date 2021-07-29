(:**************************************************************:)
(: Test: functx-fn-starts-with-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(starts-with('query', 'que'), starts-with('query', 'query'), starts-with('query', 'u'), starts-with('query', ''), starts-with('', 'query'), starts-with('', ''), starts-with('query', ()), starts-with(' query', 'q'))