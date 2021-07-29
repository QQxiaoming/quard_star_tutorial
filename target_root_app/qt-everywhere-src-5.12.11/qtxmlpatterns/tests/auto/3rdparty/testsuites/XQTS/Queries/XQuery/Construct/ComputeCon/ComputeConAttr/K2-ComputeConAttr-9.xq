(:*******************************************************:)
(: Test: K2-ComputeConAttr-9                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure XQTY0024 is issued when content appears before computed attributes. :)
(:*******************************************************:)
<elem>
		<!-- comment -->
		{
		"a string",
		999,
		("another string", attribute name {"content"}, 383),
		xs:hexBinary("FF")
		}
</elem>