(:*******************************************************:)
(: Test: K2-ExtensionExpression-10                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Whitespace between pragma-start and name cannot contain comments. :)
(:*******************************************************:)
declare namespace ex = "http://example.com/";
(#(::)ex:myExtensionExpression:)#) {true()}