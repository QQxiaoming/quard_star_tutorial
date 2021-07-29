(:*******************************************************:)
(: Test: K-NumericAdd-53                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Implementations supporting the static typing feature may raise XPTy0004. :)
(:*******************************************************:)
(remove((1, "two"), 2) + xs:untypedAtomic("1")) eq 2