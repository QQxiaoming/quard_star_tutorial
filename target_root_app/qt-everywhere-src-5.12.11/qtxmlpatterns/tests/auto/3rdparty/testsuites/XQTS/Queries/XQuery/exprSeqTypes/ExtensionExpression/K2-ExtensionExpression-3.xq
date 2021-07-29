(:*******************************************************:)
(: Test: K2-ExtensionExpression-3                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Whitespace is allowed but not required if there is no pragma content. :)
(:*******************************************************:)
declare namespace ex = "http://example.com/";
(#ex:myExtensionExpression           #) {true()}