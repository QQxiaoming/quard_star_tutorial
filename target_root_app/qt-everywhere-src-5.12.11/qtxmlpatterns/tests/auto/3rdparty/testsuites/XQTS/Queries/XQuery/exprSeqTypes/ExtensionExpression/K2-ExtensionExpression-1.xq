(:*******************************************************:)
(: Test: K2-ExtensionExpression-1                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An extension expression cannot be in an undeclared namespace. :)
(:*******************************************************:)

declare namespace xs = "";
(#xs:name content #) {1}