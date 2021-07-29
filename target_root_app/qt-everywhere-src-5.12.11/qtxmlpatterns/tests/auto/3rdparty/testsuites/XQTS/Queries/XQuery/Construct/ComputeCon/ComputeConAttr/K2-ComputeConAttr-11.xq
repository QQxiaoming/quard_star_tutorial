(:*******************************************************:)
(: Test: K2-ComputeConAttr-11                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure XQTY0024 is not issued when a predicate is used to filter the children. :)
(:*******************************************************:)
<a>{(<?target content?>, attribute name{"content"})[2]}
</a>