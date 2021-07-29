(:**************************************************************:)
(: Test: functx-fn-codepoint-equal-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(codepoint-equal('abc', 'abc'), codepoint-equal('abc', 'ab c'), codepoint-equal('abc', ()))