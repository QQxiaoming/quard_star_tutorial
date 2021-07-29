(:*******************************************************:)
(: Test: K2-ComputeConAttr-10                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure XQTY0024 is not issued when appearing nested. :)
(:*******************************************************:)
<a>
		<?target content?>
		{<b>{attribute name{"content"}}</b>}
</a>