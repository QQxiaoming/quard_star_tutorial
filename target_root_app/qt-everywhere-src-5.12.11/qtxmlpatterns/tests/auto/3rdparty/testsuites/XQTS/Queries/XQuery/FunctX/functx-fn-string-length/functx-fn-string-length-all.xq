(:**************************************************************:)
(: Test: functx-fn-string-length-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(string-length('query'), string-length('  query  '), string-length(normalize-space('  query  ')), string-length('xml query'), string-length(''), string-length(()))