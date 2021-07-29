(:*******************************************************:)
(: Test: K-ExtensionExpression-3                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple pragma expression.                  :)
(:*******************************************************:)
declare namespace prefix = "http://example.com/NotRecognized";
(#prefix:pr content #) {1 eq 1}