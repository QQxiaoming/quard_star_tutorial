(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-28                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that when one namespace declaration goes out of scope, that the one above persists. :)
(:*******************************************************:)
<e xmlns:p="http://www.w3.org/2005/xpath-functions">
<b xmlns:p="http://www.w3.org/2001/XMLSchema"/>
{fn:count(0)}
</e>