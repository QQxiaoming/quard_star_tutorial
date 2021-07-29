(:**************************************************************:)
(: Test: functx-fn-substring-after-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(substring-after('query', 'u'), substring-after('queryquery', 'ue'), substring-after('query', 'y'), substring-after('query', 'x'), substring-after('query', ''), substring-after('', 'x'))