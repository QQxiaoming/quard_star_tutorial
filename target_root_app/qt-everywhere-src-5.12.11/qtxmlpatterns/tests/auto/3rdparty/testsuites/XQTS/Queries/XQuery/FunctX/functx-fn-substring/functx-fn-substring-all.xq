(:**************************************************************:)
(: Test: functx-fn-substring-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(substring('query', 1), substring('query', 3), substring('query', 1, 1), substring('query', 2, 3), substring('query', 2, 850), substring('query', 6, 2), substring('query', -2), substring('query', -2, 5), substring('query', 1, 0), substring('', 1), substring((), 1))