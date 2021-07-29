(:**************************************************************:)
(: Test: functx-fn-upper-case-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(upper-case('query'), upper-case('QUERY'), upper-case('Query'), upper-case('query-123'))