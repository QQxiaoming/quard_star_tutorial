(:*******************************************************:)
(: Test: K2-ExtensionExpression-6                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A single whitespace must separate pragma name and content. :)
(:*******************************************************:)
declare namespace ex = "http://example.com/";
(#ex:myExtensionExpression(content)#) {true()}