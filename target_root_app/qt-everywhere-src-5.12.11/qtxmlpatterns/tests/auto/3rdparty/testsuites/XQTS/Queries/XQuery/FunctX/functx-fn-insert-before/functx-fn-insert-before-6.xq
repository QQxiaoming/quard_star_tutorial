(:**************************************************************:)
(: Test: functx-fn-insert-before-6                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(insert-before(
   (), 3, ('a', 'b', 'c') ))
