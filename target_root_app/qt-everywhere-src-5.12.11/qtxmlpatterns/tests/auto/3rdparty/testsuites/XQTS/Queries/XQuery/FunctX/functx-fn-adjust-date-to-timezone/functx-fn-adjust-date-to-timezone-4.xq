(:**************************************************************:)
(: Test: functx-fn-adjust-date-to-timezone-4                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(adjust-date-to-timezone(
   xs:date('2006-02-15-03:00'), ()))
