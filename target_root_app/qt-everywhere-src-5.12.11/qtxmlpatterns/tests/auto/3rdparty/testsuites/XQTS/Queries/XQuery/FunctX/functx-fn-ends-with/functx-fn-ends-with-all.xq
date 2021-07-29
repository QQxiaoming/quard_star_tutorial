(:**************************************************************:)
(: Test: functx-fn-ends-with-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(ends-with('query', 'y'), ends-with('query', 'query'), ends-with('query', ''), ends-with('query   ', 'y'), ends-with('', 'y'))