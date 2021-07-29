(:*******************************************************:)
(: Test: K2-ComputeConAttr-30                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Add an attribute from a variable to an element. :)
(:*******************************************************:)
declare variable $local:myVar := attribute n2 {"content"};

<b>
    {$local:myVar}
    {attribute name {"content"}}
</b>