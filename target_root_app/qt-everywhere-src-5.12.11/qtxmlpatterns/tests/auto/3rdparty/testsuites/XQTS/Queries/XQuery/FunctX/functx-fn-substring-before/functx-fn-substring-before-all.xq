(:**************************************************************:)
(: Test: functx-fn-substring-before-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(substring-before('query', 'r'), substring-before('query', 'ery'), substring-before('queryquery', 'ery'), substring-before('query', 'query'), substring-before('query', 'x'), substring-before('query', ''), substring-before('query', ()))