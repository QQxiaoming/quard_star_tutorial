(:*******************************************************:)
(: Test: K2-NamespaceProlog-4                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: When a pre-declared namespace prefix has been undeclared, it is not available. :)
(:*******************************************************:)
declare namespace xs = "";
xs:integer(1)