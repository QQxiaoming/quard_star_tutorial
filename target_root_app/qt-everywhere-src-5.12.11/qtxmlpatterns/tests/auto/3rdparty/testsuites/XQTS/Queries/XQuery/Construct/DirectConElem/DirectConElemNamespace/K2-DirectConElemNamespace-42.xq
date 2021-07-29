(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-42                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A direct element constructor with a redundant, prefixed namespace declaration. :)
(:*******************************************************:)
<e xmlns:p="http://www.example.com/">
    <e xmlns:p="http://www.example.com/"/>
</e>