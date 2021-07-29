(:*******************************************************:)
(: Test: K-ExtensionExpression-8                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A pragma expression containing many comments. :)
(:*******************************************************:)
declare namespace prefix = "http://example.com/NotRecognized";
(::)1(::)eq(::)(#prefix:name ##cont## # # ( "# ) #
		)# )#ent #)(::){(::)1(::)}(::)