(:**************************************************************:)
(: Test: functx-fn-subsequence-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(subsequence(
   ('a', 'b', 'c', 'd', 'e'), 3) , subsequence(
   ('a', 'b', 'c', 'd', 'e'), 3, 2), subsequence(
   ('a', 'b', 'c', 'd', 'e'), 3, 10), subsequence(
   ('a', 'b', 'c', 'd', 'e'), 10), subsequence(
   ('a', 'b', 'c', 'd', 'e'), -2, 5), subsequence( (), 3))