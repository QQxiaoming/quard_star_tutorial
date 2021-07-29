(:*******************************************************:)
(: Test: K2-ComputeConAttr-23                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure XQTY0024 is not issued when a predicate avoid the condition. :)
(:*******************************************************:)
declare variable $myVar := (<elem/>, attribute name {"content"});
<b>
    {$myVar[2]}
</b>