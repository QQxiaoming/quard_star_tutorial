(:**************************************************************:)
(: Test: functx-fn-round-half-to-even-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(round-half-to-even(5.5), round-half-to-even(6.5), round-half-to-even(9372.253, 2), round-half-to-even(9372.253, 0), round-half-to-even(9372.253, -3))