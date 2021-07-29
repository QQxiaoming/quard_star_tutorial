(:**************************************************************:)
(: Test: functx-fn-string-length-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(string-length(normalize-space('  query  ')))
