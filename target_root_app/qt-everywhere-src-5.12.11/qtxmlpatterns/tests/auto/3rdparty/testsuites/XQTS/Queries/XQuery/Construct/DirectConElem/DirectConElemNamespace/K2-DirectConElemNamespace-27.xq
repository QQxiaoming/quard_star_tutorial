(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-27                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that when one namespace declaration goes out of scope, that the one above doesn't dissapear. :)
(:*******************************************************:)
<e xmlns:p="http://www.w3.org/2001/XMLSchema">
<b xmlns:p="http://www.w3.org/2001/XMLSchema"/>
{p:integer(1)}
</e>