(:*******************************************************:)
(: Test: K2-ExtensionExpression-4                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Content looking like comments are not recognized as so in pragma content. :)
(:*******************************************************:)
declare namespace ex = "http://example.com/";
(#ex:myExtensionExpression (:asdad:)content#) {true()}