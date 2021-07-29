(:*******************************************************:)
(: Test: K-ExtensionExpression-7                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A fallback expression must be present when no supported pragmas are specified. :)
(:*******************************************************:)
declare namespace prefix = "http://example.com/NotRecognized";
(#prefix:PragmaNotSupported content #) {}