(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-56                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that an attribute name tests properly interact with namespace declarations. :)
(:*******************************************************:)
declare default element namespace "http://example.com/";
<r xmlns:p="http://example.com/">
    {
        <e p:att=""/>/(@att, attribute::att)
    }
</r>