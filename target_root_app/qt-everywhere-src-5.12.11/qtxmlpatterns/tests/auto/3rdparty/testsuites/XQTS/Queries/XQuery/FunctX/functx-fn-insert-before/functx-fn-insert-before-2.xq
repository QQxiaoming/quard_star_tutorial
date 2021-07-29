(:**************************************************************:)
(: Test: functx-fn-insert-before-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(insert-before(
   ('a', 'b', 'c'), 2, ('x', 'y')))
