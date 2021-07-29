(:**************************************************************:)
(: Test: functx-fn-matches-11                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";

let $address := '123 Main Street
Traverse City, MI 49684'
return (matches($address, 'Street$'))
