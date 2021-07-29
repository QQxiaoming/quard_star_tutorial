(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-57                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Attempt an attribute node copy that never will happen. :)
(:*******************************************************:)
declare default element namespace "http://example.com/";
declare namespace p = "http://example.com/";
<r>
    {
    <e p:att=""/>/(@att)
    }
</r>