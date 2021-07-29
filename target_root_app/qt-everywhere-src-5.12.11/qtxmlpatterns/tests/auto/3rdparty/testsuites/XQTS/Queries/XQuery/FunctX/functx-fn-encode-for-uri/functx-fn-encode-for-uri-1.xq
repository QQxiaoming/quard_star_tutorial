(:**************************************************************:)
(: Test: functx-fn-encode-for-uri-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(encode-for-uri(
   'Sales % Numbers.pdf'))
