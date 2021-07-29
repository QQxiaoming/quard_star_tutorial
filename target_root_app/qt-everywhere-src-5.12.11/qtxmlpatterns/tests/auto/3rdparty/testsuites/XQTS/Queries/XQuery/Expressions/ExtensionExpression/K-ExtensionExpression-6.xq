(:*******************************************************:)
(: Test: K-ExtensionExpression-6                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A pragma expression containing complex content. :)
(:*******************************************************:)
declare namespace prefix = "http://example.com/NotRecognized";
1 eq (#prefix:notRecognized ##cont## # # ( "# ) # )# )#ent #) {1}