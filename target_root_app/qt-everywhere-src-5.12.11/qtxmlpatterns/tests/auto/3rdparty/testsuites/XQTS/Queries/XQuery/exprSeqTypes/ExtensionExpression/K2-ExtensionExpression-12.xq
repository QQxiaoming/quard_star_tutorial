(:*******************************************************:)
(: Test: K2-ExtensionExpression-12                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: No whitespace is required between pragma content and name if the content is empty. :)
(:*******************************************************:)
declare namespace ex = "http://example.com/";
(#ex:myExtensionExpression#) {true()}