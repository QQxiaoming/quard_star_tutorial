(:**************************************************************:)
(: Test: functx-fn-matches-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";

let $address := '123 Main Street
Traverse City, MI 49684'
return (matches('query', 'q'), matches('query', 'ue'), matches('query', '^qu'), matches('query', 'qu$'), matches('query', '[ux]'), matches('query', 'q.*'), matches('query', '[a-z]{5}'), matches((), 'q' ), matches($address, 'Street.*City'), matches($address, 'Street.*City', 's'), matches($address, 'Street$'), matches($address, 'Street$', 'm'), matches($address, 'street'), matches($address, 'street', 'i'), matches($address, 'Main Street'), matches($address, 'Main Street', 'x'), matches($address, 'Main \s Street', 'x'), matches($address, 'street$', 'im'))