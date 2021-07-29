(:**************************************************************:)
(: Test: functx-fn-namespace-uri-from-QName-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(namespace-uri-from-QName( () ))
