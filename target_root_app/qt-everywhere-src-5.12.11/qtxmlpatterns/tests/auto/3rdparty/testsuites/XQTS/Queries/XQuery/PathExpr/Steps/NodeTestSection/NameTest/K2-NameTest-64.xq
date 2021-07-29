(:*******************************************************:)
(: Test: K2-NameTest-64                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the axis is correct when using an attribute(*, type) test in the abbreviated axis. :)
(:*******************************************************:)
<e>{<e foo="asd"/>/attribute(*, xs:untypedAtomic)}</e>