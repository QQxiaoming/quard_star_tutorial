(:*******************************************************:)
(: Test: K2-Axes-89                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure attribute tests match, and that attributes are properly copied when combined with a default element namespace declaration. :)
(:*******************************************************:)
declare default element namespace "http://typedecl";
<r>
    {
        <e attr="foo"/>/@attr
    }
</r>