(:**************************************************************:)
(: Test: functx-fn-resolve-uri-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(resolve-uri(
   'http://example.org','http://datypic.com'))
