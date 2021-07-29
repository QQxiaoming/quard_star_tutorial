(:**************************************************************:)
(: Test: functx-fn-compare-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(compare('a', 'b'), compare('a', 'a'), compare('b', 'a'), compare('ab', 'abc'), compare('a', 'B'), compare(upper-case('a'), upper-case('B')), compare('a', ()))