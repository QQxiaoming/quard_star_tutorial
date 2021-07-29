(:**************************************************************:)
(: Test: functx-fn-insert-before-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(insert-before(
   ('a', 'b', 'c'), 1, ('x', 'y')), insert-before(
   ('a', 'b', 'c'), 2, ('x', 'y')), insert-before(
   ('a', 'b', 'c'), 10, ('x', 'y')), insert-before(
   ('a', 'b', 'c'), 0, ('x', 'y')), insert-before(
   ('a', 'b', 'c'), 2, ()), insert-before(
   (), 3, ('a', 'b', 'c') ))